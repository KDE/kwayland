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

    wl_keyboard *keyboard = nullptr;

private:
    static void keymapCallback(void *data, wl_keyboard *keyboard, uint32_t format, int fd, uint32_t size);
    static void enterCallback(void *data, wl_keyboard *keyboard, uint32_t serial, wl_surface *surface, wl_array *keys);
    static void leaveCallback(void *data, wl_keyboard *keyboard, uint32_t serial, wl_surface *surface);
    static void keyCallback(void *data, wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
    static void modifiersCallback(void *data, wl_keyboard *keyboard, uint32_t serial, uint32_t modsDepressed,
                                  uint32_t modsLatched, uint32_t modsLocked, uint32_t group);
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
    keyboard = k;
    wl_keyboard_add_listener(keyboard, &s_listener, this);
}

const wl_keyboard_listener Keyboard::Private::s_listener = {
    keymapCallback,
    enterCallback,
    leaveCallback,
    keyCallback,
    modifiersCallback
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
    if (!d->keyboard) {
        return;
    }
    wl_keyboard_destroy(d->keyboard);
    d->keyboard = nullptr;
}

void Keyboard::setup(wl_keyboard *keyboard)
{
    d->setup(keyboard);
}

void Keyboard::Private::enterCallback(void *data, wl_keyboard *keyboard, uint32_t serial, wl_surface *surface, wl_array *keys)
{
    // ignore
    Q_UNUSED(data)
    Q_UNUSED(keyboard)
    Q_UNUSED(serial)
    Q_UNUSED(surface)
    Q_UNUSED(keys)
}

void Keyboard::Private::leaveCallback(void *data, wl_keyboard *keyboard, uint32_t serial, wl_surface *surface)
{
    // ignore
    Q_UNUSED(data)
    Q_UNUSED(keyboard)
    Q_UNUSED(serial)
    Q_UNUSED(surface)
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

bool Keyboard::isValid() const
{
    return d->keyboard != nullptr;
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
