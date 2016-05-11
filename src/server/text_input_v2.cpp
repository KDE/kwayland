/****************************************************************************
Copyright 2016  Martin Gräßlin <mgraesslin@kde.org>

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
****************************************************************************/
#include "text_interface.h"
#include "text_interface_p.h"
#include "display.h"
#include "resource_p.h"
#include "seat_interface_p.h"
#include "surface_interface.h"

#include <wayland-text-input-unstable-v2-server-protocol.h>

namespace KWayland
{
namespace Server
{

class TextInputUnstableV2Interface::Private : public TextInputInterface::Private
{
public:
    Private(TextInputInterface *q, TextInputManagerUnstableV2Interface *c, wl_resource *parentResource);
    ~Private();

    void sendEnter(SurfaceInterface *surface, quint32 serial) override;
    void sendLeave(quint32 serial) override;
    void requestActivate(SeatInterface *seat, SurfaceInterface *surface) override;
    void requestDeactivate(SeatInterface *seat) override;
    void preEdit(const QByteArray &text, const QByteArray &commit) override;
    void commit(const QByteArray &text) override;
    void deleteSurroundingText(qint32 index, quint32 length) override;
    void textDirection(Qt::LayoutDirection direction) override;
    void preEditCursor(qint32 index) override;
    void cursorPosition(qint32 index, qint32 anchor) override;
    void keysymPressed(quint32 keysym, Qt::KeyboardModifiers modifiers) override;
    void keysymReleased(quint32 keysym, Qt::KeyboardModifiers modifiers) override;
    TextInputInterfaceVersion interfaceVersion() const override {
        return TextInputInterfaceVersion::UnstableV2;
    }
    void sendInputPanelState() override;

private:
    static void updateStateCallback(wl_client *client, wl_resource *resource, uint32_t serial, uint32_t reason);
    static const struct zwp_text_input_v2_interface s_interface;

    TextInputUnstableV2Interface *q_func() {
        return reinterpret_cast<TextInputUnstableV2Interface *>(q);
    }
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zwp_text_input_v2_interface TextInputUnstableV2Interface::Private::s_interface = {
    destroyCallback,
    enableCallback,
    disableCallback,
    showInputPanelCallback,
    hideInputPanelCallback,
    setSurroundingText2Callback,
    setContentTypeCallback,
    setCursorRectangleCallback,
    setPreferredLanguageCallback,
    updateStateCallback
};
#endif

void TextInputUnstableV2Interface::Private::requestActivate(SeatInterface *seat, SurfaceInterface *s)
{
    Q_UNUSED(seat)
    surface = QPointer<SurfaceInterface>(s);
    emit q_func()->requestActivate();
}

void TextInputUnstableV2Interface::Private::requestDeactivate(SeatInterface *seat)
{
    Q_UNUSED(seat)
    surface.clear();
    emit q_func()->requestDeactivate();
}

void TextInputUnstableV2Interface::Private::sendEnter(SurfaceInterface *surface, quint32 serial)
{
    if (!resource) {
        return;
    }
    zwp_text_input_v2_send_enter(resource, serial, surface->resource());
}

void TextInputUnstableV2Interface::Private::sendLeave(quint32 serial)
{
    if (!resource) {
        return;
    }
    zwp_text_input_v2_send_leave(resource, serial, surface->resource());
}

void TextInputUnstableV2Interface::Private::preEdit(const QByteArray &text, const QByteArray &commit)
{
    if (!resource) {
        return;
    }
    zwp_text_input_v2_send_preedit_string(resource, text.constData(), commit.constData());
}

void TextInputUnstableV2Interface::Private::commit(const QByteArray &text)
{
    if (!resource) {
        return;
    }
    zwp_text_input_v2_send_commit_string(resource, text.constData());
}

void TextInputUnstableV2Interface::Private::keysymPressed(quint32 keysym, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers)
    if (!resource) {
        return;
    }
    zwp_text_input_v2_send_keysym(resource, seat ? seat->timestamp() : 0, keysym, WL_KEYBOARD_KEY_STATE_PRESSED, 0);
}

void TextInputUnstableV2Interface::Private::keysymReleased(quint32 keysym, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers)
    if (!resource) {
        return;
    }
    zwp_text_input_v2_send_keysym(resource, seat ? seat->timestamp() : 0, keysym, WL_KEYBOARD_KEY_STATE_RELEASED, 0);
}

void TextInputUnstableV2Interface::Private::deleteSurroundingText(qint32 index, quint32 length)
{
    if (!resource) {
        return;
    }
    // TODO: index is now beforeLenght, is that still correct?
    zwp_text_input_v2_send_delete_surrounding_text(resource, index, length);
}

void TextInputUnstableV2Interface::Private::cursorPosition(qint32 index, qint32 anchor)
{
    if (!resource) {
        return;
    }
    zwp_text_input_v2_send_cursor_position(resource, index, anchor);
}

void TextInputUnstableV2Interface::Private::textDirection(Qt::LayoutDirection direction)
{
    if (!resource) {
        return;
    }
    zwp_text_input_v2_text_direction wlDirection;
    switch (direction) {
    case Qt::LeftToRight:
        wlDirection = ZWP_TEXT_INPUT_V2_TEXT_DIRECTION_LTR;
        break;
    case Qt::RightToLeft:
        wlDirection = ZWP_TEXT_INPUT_V2_TEXT_DIRECTION_RTL;
        break;
    case Qt::LayoutDirectionAuto:
        wlDirection = ZWP_TEXT_INPUT_V2_TEXT_DIRECTION_AUTO;
        break;
    default:
        Q_UNREACHABLE();
        break;
    }
    zwp_text_input_v2_send_text_direction(resource, wlDirection);
}

void TextInputUnstableV2Interface::Private::preEditCursor(qint32 index)
{
    if (!resource) {
        return;
    }
    zwp_text_input_v2_send_preedit_cursor(resource, index);
}

void TextInputUnstableV2Interface::Private::sendInputPanelState()
{
    if (!resource) {
        return;
    }
    zwp_text_input_v2_send_input_panel_state(resource,
                                             inputPanelVisible ? ZWP_TEXT_INPUT_V2_INPUT_PANEL_VISIBILITY_VISIBLE : ZWP_TEXT_INPUT_V2_INPUT_PANEL_VISIBILITY_HIDDEN,
                                             overlappedSurfaceArea.x(), overlappedSurfaceArea.y(), overlappedSurfaceArea.width(), overlappedSurfaceArea.height());
}

TextInputUnstableV2Interface::Private::Private(TextInputInterface *q, TextInputManagerUnstableV2Interface *c, wl_resource *parentResource)
    : TextInputInterface::Private(q, c, parentResource, &zwp_text_input_v2_interface, &s_interface)
{
}

TextInputUnstableV2Interface::Private::~Private() = default;

void TextInputUnstableV2Interface::Private::updateStateCallback(wl_client *client, wl_resource *resource, uint32_t serial, uint32_t reason)
{
    auto p = cast<Private>(resource);
    Q_ASSERT(*p->client == client);
    Q_UNUSED(serial)
    // TODO: use other reason values reason
    if (reason == ZWP_TEXT_INPUT_V2_UPDATE_STATE_RESET) {
        emit p->q_func()->requestReset();
    }
}

TextInputUnstableV2Interface::TextInputUnstableV2Interface(TextInputManagerUnstableV2Interface *parent, wl_resource *parentResource)
    : TextInputInterface(new Private(this, parent, parentResource), parent)
{
}

TextInputUnstableV2Interface::~TextInputUnstableV2Interface() = default;

class TextInputManagerUnstableV2Interface::Private : public TextInputManagerInterface::Private
{
public:
    Private(TextInputManagerUnstableV2Interface *q, Display *d);

private:
    void bind(wl_client *client, uint32_t version, uint32_t id) override;

    static void unbind(wl_resource *resource);
    static Private *cast(wl_resource *r) {
        return reinterpret_cast<Private*>(wl_resource_get_user_data(r));
    }

    static void destroyCallback(wl_client *client, wl_resource *resource);
    static void getTextInputCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource * seat);

    TextInputManagerUnstableV2Interface *q;
    static const struct zwp_text_input_manager_v2_interface s_interface;
    static const quint32 s_version;
};
const quint32 TextInputManagerUnstableV2Interface::Private::s_version = 1;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zwp_text_input_manager_v2_interface TextInputManagerUnstableV2Interface::Private::s_interface = {
    destroyCallback,
    getTextInputCallback
};
#endif

void TextInputManagerUnstableV2Interface::Private::destroyCallback(wl_client *client, wl_resource *resource)
{
    // TODO: implement
}

void TextInputManagerUnstableV2Interface::Private::getTextInputCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource * seat)
{
    SeatInterface *s = SeatInterface::get(seat);
    if (!s) {
        // TODO: send error
        return;
    }
    auto m = cast(resource);
    auto *t = new TextInputUnstableV2Interface(m->q, resource);
    t->d_func()->seat = s;
    s->d_func()->registerTextInput(t);
    m->inputs << t;
    QObject::connect(t, &QObject::destroyed, m->q,
        [t, m] {
            m->inputs.removeAll(t);
        }
    );
    QObject::connect(t, &TextInputUnstableV2Interface::requestActivate, m->q,
        [t] {
            t->d_func()->seat->d_func()->updateActiveTextInput();
        }
    );
    QObject::connect(t, &TextInputUnstableV2Interface::requestDeactivate, m->q,
        [t] {
            t->d_func()->seat->d_func()->updateActiveTextInput();
        }
    );
    t->d->create(m->display->getConnection(client), version, id);
}

TextInputManagerUnstableV2Interface::Private::Private(TextInputManagerUnstableV2Interface *q, Display *d)
    : TextInputManagerInterface::Private(TextInputInterfaceVersion::UnstableV2, q, d, &zwp_text_input_manager_v2_interface, s_version)
    , q(q)
{
}

void TextInputManagerUnstableV2Interface::Private::bind(wl_client *client, uint32_t version, uint32_t id)
{
    auto c = display->getConnection(client);
    wl_resource *resource = c->createResource(&zwp_text_input_manager_v2_interface, qMin(version, s_version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &s_interface, this, unbind);
    // TODO: should we track?
}

void TextInputManagerUnstableV2Interface::Private::unbind(wl_resource *resource)
{
    Q_UNUSED(resource)
    // TODO: implement?
}

TextInputManagerUnstableV2Interface::TextInputManagerUnstableV2Interface(Display *display, QObject *parent)
    : TextInputManagerInterface(new Private(this, display), parent)
{
}

TextInputManagerUnstableV2Interface::~TextInputManagerUnstableV2Interface() = default;

}
}
