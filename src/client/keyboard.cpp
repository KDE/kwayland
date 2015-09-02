/********************************************************************
Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>

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
#include "keyboard.h"
#include "surface.h"
#include "wayland_pointer_p.h"
// wayland
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{

class Keyboard::Private
{
public:
    Private(Keyboard *q);
    void setup(wl_keyboard *k);

    WaylandPointer<wl_keyboard, wl_keyboard_destroy> keyboard;
    Surface *enteredSurface = nullptr;

    struct {
        qint32 charactersPerSecond = 0;
        qint32 delay = 0;
    } repeatInfo;
private:
    void enter(uint32_t serial, wl_surface *surface, wl_array *keys);
    void leave(uint32_t serial);
    static void keymapCallback(void *data, wl_keyboard *keyboard, uint32_t format, int fd, uint32_t size);
    static void enterCallback(void *data, wl_keyboard *keyboard, uint32_t serial, wl_surface *surface, wl_array *keys);
    static void leaveCallback(void *data, wl_keyboard *keyboard, uint32_t serial, wl_surface *surface);
    static void keyCallback(void *data, wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
    static void modifiersCallback(void *data, wl_keyboard *keyboard, uint32_t serial, uint32_t modsDepressed,
                                  uint32_t modsLatched, uint32_t modsLocked, uint32_t group);
    static void repeatInfoCallback(void *data, wl_keyboard *keyboard, int32_t charactersPerSecond, int32_t delay);
    Keyboard *q;
    static const wl_keyboard_listener s_listener;
};

Keyboard::Private::Private(Keyboard *q)
    : q(q)
{
}

void Keyboard::Private::setup(wl_keyboard *k)
{
    Q_ASSERT(k);
    Q_ASSERT(!keyboard);
    keyboard.setup(k);
    wl_keyboard_add_listener(keyboard, &s_listener, this);
}

const wl_keyboard_listener Keyboard::Private::s_listener = {
    keymapCallback,
    enterCallback,
    leaveCallback,
    keyCallback,
    modifiersCallback,
    repeatInfoCallback
};

Keyboard::Keyboard(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

Keyboard::~Keyboard()
{
    release();
}

void Keyboard::release()
{
    d->keyboard.release();
}

void Keyboard::destroy()
{
    d->keyboard.destroy();
}

void Keyboard::setup(wl_keyboard *keyboard)
{
    d->setup(keyboard);
}

void Keyboard::Private::enterCallback(void *data, wl_keyboard *keyboard, uint32_t serial, wl_surface *surface, wl_array *keys)
{
    auto k = reinterpret_cast<Private*>(data);
    Q_ASSERT(k->keyboard == keyboard);
    k->enter(serial, surface, keys);
}

void Keyboard::Private::enter(uint32_t serial, wl_surface *surface, wl_array *keys)
{
    Q_UNUSED(keys)
    enteredSurface = Surface::get(surface);
    emit q->entered(serial);
}

void Keyboard::Private::leaveCallback(void *data, wl_keyboard *keyboard, uint32_t serial, wl_surface *surface)
{
    auto k = reinterpret_cast<Private*>(data);
    Q_ASSERT(k->keyboard == keyboard);
    Q_ASSERT(*(k->enteredSurface) == surface);
    k->leave(serial);
}

void Keyboard::Private::leave(uint32_t serial)
{
    enteredSurface = nullptr;
    emit q->left(serial);
}

void Keyboard::Private::keyCallback(void *data, wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    Q_UNUSED(serial)
    auto k = reinterpret_cast<Keyboard::Private*>(data);
    Q_ASSERT(k->keyboard == keyboard);
    auto toState = [state] {
        if (state == WL_KEYBOARD_KEY_STATE_RELEASED) {
            return KeyState::Released;
        } else {
            return KeyState::Pressed;
        }
    };
    emit k->q->keyChanged(key, toState(), time);
}

void Keyboard::Private::keymapCallback(void *data, wl_keyboard *keyboard, uint32_t format, int fd, uint32_t size)
{
    auto k = reinterpret_cast<Keyboard::Private*>(data);
    Q_ASSERT(k->keyboard == keyboard);
    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
        return;
    }
    emit k->q->keymapChanged(fd, size);
}

void Keyboard::Private::modifiersCallback(void *data, wl_keyboard *keyboard, uint32_t serial, uint32_t modsDepressed,
                                 uint32_t modsLatched, uint32_t modsLocked, uint32_t group)
{
    Q_UNUSED(serial)
    auto k = reinterpret_cast<Keyboard::Private*>(data);
    Q_ASSERT(k->keyboard == keyboard);
    emit k->q->modifiersChanged(modsDepressed, modsLatched, modsLocked, group);
}

void Keyboard::Private::repeatInfoCallback(void *data, wl_keyboard *keyboard, int32_t charactersPerSecond, int32_t delay)
{
    auto k = reinterpret_cast<Keyboard::Private*>(data);
    Q_ASSERT(k->keyboard == keyboard);
    k->repeatInfo.charactersPerSecond = qMax(charactersPerSecond, 0);
    k->repeatInfo.delay = qMax(delay, 0);
    emit k->q->keyRepeatChanged();
}

Surface *Keyboard::enteredSurface()
{
    return d->enteredSurface;
}

Surface *Keyboard::enteredSurface() const
{
    return d->enteredSurface;
}

bool Keyboard::isValid() const
{
    return d->keyboard.isValid();
}

bool Keyboard::isKeyRepeatEnabled() const
{
    return d->repeatInfo.charactersPerSecond > 0;
}

qint32 Keyboard::keyRepeatDelay() const
{
    return d->repeatInfo.delay;
}

qint32 Keyboard::keyRepeatRate() const
{
    return d->repeatInfo.charactersPerSecond;
}

Keyboard::operator wl_keyboard*()
{
    return d->keyboard;
}

Keyboard::operator wl_keyboard*() const
{
    return d->keyboard;
}

}
}
