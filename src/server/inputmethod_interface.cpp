/********************************************************************
Copyright 2020 Aleix Pol Gonzalez <aleixpol@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#include "inputmethod_interface.h"
#include "resource_p.h"
#include "seat_interface.h"
#include "display.h"
#include "surface_interface.h"
#include "output_interface.h"

#include <QHash>
#include "qwayland-server-input-method-unstable-v1.h"
#include "wayland-text-server-protocol.h"

using namespace KWayland;
using namespace Server;

static int s_version = 1;

using namespace KWayland::Server;

class InputMethodContextInterface::Private : public QtWaylandServer::zwp_input_method_context_v1
{
public:
    Private(InputMethodInterface* method, InputMethodContextInterface* q) : zwp_input_method_context_v1(), q(q), method(method) {}

    void zwp_input_method_context_v1_commit_string(Resource *, uint32_t serial, const QString & text) override {
        Q_EMIT q->commitString(serial, text);
    }
    void zwp_input_method_context_v1_preedit_string(Resource *, uint32_t serial, const QString &text, const QString &commit) override {
        Q_EMIT q->preeditString(serial, text, commit);
    }

    void zwp_input_method_context_v1_preedit_styling(Resource *, uint32_t index, uint32_t length, uint32_t style) override {
        Q_EMIT q->preeditStyling(index, length, style);
    }
    void zwp_input_method_context_v1_preedit_cursor(Resource *, int32_t index) override {
        Q_EMIT q->preeditCursor(index);
    }
    void zwp_input_method_context_v1_delete_surrounding_text(Resource *, int32_t index, uint32_t length) override {
        Q_EMIT q->deleteSurroundingText(index, length);
    }
    void zwp_input_method_context_v1_cursor_position(Resource *, int32_t index, int32_t anchor) override {
        Q_EMIT q->cursorPosition(index, anchor);
    }
    void zwp_input_method_context_v1_modifiers_map(Resource *, wl_array *map) override {
        const QList<QByteArray> modifiersMap = QByteArray::fromRawData(static_cast<const char*>(map->data), map->size).split('\0');

        mods.clear();
        for (const QByteArray &modifier : modifiersMap) {
            if (modifier == "Shift")
                mods << Qt::ShiftModifier;
            else if (modifier == "Alt")
                mods << Qt::AltModifier;
            else if (modifier == "Control")
                mods << Qt::ControlModifier;
            else if (modifier == "Mod1")
                mods << Qt::AltModifier;
            else if (modifier == "Mod4")
                mods << Qt::MetaModifier;
            else
                mods << Qt::NoModifier;
        }
    }
    void zwp_input_method_context_v1_keysym(Resource *, uint32_t serial, uint32_t time, uint32_t sym, uint32_t state, uint32_t modifiers) override {
        Q_EMIT q->keysym(serial, time, sym, state == WL_KEYBOARD_KEY_STATE_PRESSED, toQtModifiers(modifiers));
    }
    void zwp_input_method_context_v1_grab_keyboard(Resource *, uint32_t keyboard) override {
        Q_EMIT q->grabKeyboard(keyboard);
    }
    void zwp_input_method_context_v1_key(Resource *, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) override {
        Q_EMIT q->key(serial, time, key, state == WL_KEYBOARD_KEY_STATE_PRESSED);
    }
    void zwp_input_method_context_v1_modifiers(Resource *, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) override {
        Q_EMIT q->modifiers(serial, toQtModifiers(mods_depressed), toQtModifiers(mods_latched), toQtModifiers(mods_locked), group);
    }
    void zwp_input_method_context_v1_language(Resource *, uint32_t serial, const QString &language) override {
        Q_EMIT q->language(serial, language);
    }
    void zwp_input_method_context_v1_text_direction(Resource *, uint32_t serial, uint32_t direction) override {
        Qt::LayoutDirection qtDirection;
        switch (direction) {
            case WL_TEXT_INPUT_TEXT_DIRECTION_LTR:
                qtDirection = Qt::LeftToRight;
                break;
            case WL_TEXT_INPUT_TEXT_DIRECTION_RTL:
                qtDirection = Qt::RightToLeft;
                break;
            case WL_TEXT_INPUT_TEXT_DIRECTION_AUTO:
                qtDirection = Qt::LayoutDirectionAuto;
                break;
        }
        Q_EMIT q->textDirection(serial, qtDirection);
    }

    Qt::KeyboardModifiers toQtModifiers(uint32_t modifiers)
    {
        Qt::KeyboardModifiers ret = Qt::NoModifier;
        for (int i = 0; modifiers >>= 1; ++i) {
            ret |= mods[i];
        }
        return ret;
    }
    void zwp_input_method_context_v1_destroy(QtWaylandServer::zwp_input_method_context_v1::Resource * resource) override {
        method->sendDeactivate();
    }

private:
    InputMethodContextInterface *const q;
    InputMethodInterface *const method;
    QVector<Qt::KeyboardModifiers> mods;
};

InputMethodContextInterface::InputMethodContextInterface(InputMethodInterface *parent)
    : QObject(parent)
    , d(new InputMethodContextInterface::Private(parent, this))
{
}

InputMethodContextInterface::~InputMethodContextInterface() = default;

void InputMethodContextInterface::sendCommitState(uint32_t serial)
{
    for (auto r : d->resourceMap())
        d->send_commit_state(r->handle, serial);
}

void InputMethodContextInterface::sendContentType(uint32_t hint, uint32_t purpose)
{
    for (auto r : d->resourceMap())
        d->send_content_type(r->handle, hint, purpose);
}

void InputMethodContextInterface::sendInvokeAction(uint32_t button, uint32_t index)
{
    for (auto r : d->resourceMap())
        d->send_invoke_action(r->handle, button, index);
}

void InputMethodContextInterface::sendPreferredLanguage(const QString& language)
{
    for (auto r : d->resourceMap())
        d->send_preferred_language(r->handle, language);
}

void InputMethodContextInterface::sendReset()
{
    for (auto r : d->resourceMap())
        d->send_reset(r->handle);
}

void InputMethodContextInterface::sendSurroundingText(const QString& text, uint32_t cursor, uint32_t anchor)
{
    for (auto r : d->resourceMap())
        d->send_surrounding_text(r->handle, text, cursor, anchor);
}

class InputPanelSurfaceInterface::Private : public QtWaylandServer::zwp_input_panel_surface_v1
{
public:
    Private(InputPanelSurfaceInterface* q)
        : zwp_input_panel_surface_v1()
        , q(q)
    {
    }

    void zwp_input_panel_surface_v1_set_overlay_panel(Resource *) override {
        Q_EMIT q->overlayPanel();
    }
    void zwp_input_panel_surface_v1_set_toplevel(Resource *, struct ::wl_resource * output, uint32_t position) override {
        OutputInterface* outputIface = OutputInterface::get(output);
        Q_EMIT q->topLevel(outputIface, Position(position));
    }

    InputPanelSurfaceInterface* const q;
    QPointer<SurfaceInterface> m_surface;
    bool m_overlay = false;
};

InputPanelSurfaceInterface::InputPanelSurfaceInterface(QObject* parent)
    : QObject(parent)
    , d(new InputPanelSurfaceInterface::Private(this))
{
}

InputPanelSurfaceInterface::~InputPanelSurfaceInterface() = default;

class InputPanelInterface::Private : public QObject, public QtWaylandServer::zwp_input_panel_v1
{
public:
    Private(InputPanelInterface *q, Display *d)
        : zwp_input_panel_v1(*d, s_version)
        , q(q)
    {
    }

    void zwp_input_panel_v1_get_input_panel_surface(Resource * resource, uint32_t id, struct ::wl_resource * surface) override {
        auto surfaceIface = SurfaceInterface::get(surface);

        auto ipsi = new InputPanelSurfaceInterface(nullptr);
        ipsi->d->init(resource->client(), id, resource->version());
        ipsi->d->m_surface = surfaceIface;
        m_surfaces[id] = ipsi;

        Q_EMIT q->inputPanelSurfaceAdded(id, ipsi);
    }

    QHash<uint32_t, InputPanelSurfaceInterface*> m_surfaces;
    InputPanelInterface * const q;
};

InputPanelInterface::InputPanelInterface(Display* d, QObject *parent)
    : QObject(parent)
    , d(new InputPanelInterface::Private(this, d))
{
}

InputPanelInterface::~InputPanelInterface() = default;

QHash<uint32_t, InputPanelSurfaceInterface*> InputPanelInterface::surfaces() const
{
    return d->m_surfaces;
}

KWayland::Server::SurfaceInterface * KWayland::Server::InputPanelSurfaceInterface::surface() const
{
    return d->m_surface;
}

class InputMethodInterface::Private : public QtWaylandServer::zwp_input_method_v1
{
public:
    Private(Display *d, InputMethodInterface* q)
        : zwp_input_method_v1(*d, s_version)
        , q(q)
        , m_display(d)
    {}

    void zwp_input_method_v1_bind_resource(Resource * resource) override {
        auto x = m_context->d->add(resource->client(), resource->version());

        if (m_enabled) {
            send_activate(resource->handle, x->handle);
        }
    }

    InputMethodContextInterface *m_context = nullptr;
    InputMethodInterface * const q;
    Display * const m_display;

    bool m_enabled = false;
};

InputMethodInterface::InputMethodInterface(Display *d, QObject* parent)
    : QObject(parent)
    , d(new InputMethodInterface::Private(d, this))
{
}

InputMethodInterface::~InputMethodInterface() = default;

InputMethodContextInterface* InputMethodInterface::sendActivate()
{
    d->m_context = new InputMethodContextInterface(this);

    d->m_enabled = true;
    for (auto resource : d->resourceMap()) {
        auto connection = d->m_context->d->add(resource->client(), resource->version());
        d->send_activate(resource->handle, connection->handle);
    }
    return d->m_context;
}

void InputMethodInterface::sendDeactivate()
{
    if (!d->m_enabled)
        return;

    d->m_enabled = false;
    for (auto resource : d->resourceMap()) {
        auto connection = d->m_context->d->resourceMap().value(resource->client());
        d->send_deactivate(connection->handle, resource->handle);
    }

    delete d->m_context;
    d->m_context = nullptr;
}
