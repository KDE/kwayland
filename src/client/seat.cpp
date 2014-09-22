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
#include "seat.h"
#include "keyboard.h"
#include "pointer.h"
// Wayland
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{

class Seat::Private
{
public:
    Private(Seat *q);
    void resetSeat();
    void setup(wl_seat *seat);

    wl_seat *seat = nullptr;
    bool capabilityKeyboard = false;
    bool capabilityPointer = false;
    bool capabilityTouch = false;
    QString name;

private:
    void setHasKeyboard(bool has);
    void setHasPointer(bool has);
    void setHasTouch(bool has);
    void capabilitiesChanged(uint32_t capabilities);
    void setName(const QString &name);
    static void capabilitiesCallback(void *data, wl_seat *seat, uint32_t capabilities);
    static void nameCallback(void *data, wl_seat *wl_seat, const char *name);

    Seat *q;
    static const wl_seat_listener s_listener;
};

Seat::Private::Private(Seat *q)
    : q(q)
{
}

void Seat::Private::setup(wl_seat *s)
{
    Q_ASSERT(s);
    Q_ASSERT(!seat);
    seat = s;
    wl_seat_add_listener(seat, &s_listener, this);
}

const wl_seat_listener Seat::Private::s_listener = {
    capabilitiesCallback,
    nameCallback
};

Seat::Seat(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

Seat::~Seat()
{
    release();
}

void Seat::release()
{
    if (!d->seat) {
        return;
    }
    wl_seat_destroy(d->seat);
    d->seat = nullptr;
    d->resetSeat();
}

void Seat::destroy()
{
    if (!d->seat) {
        return;
    }
    free(d->seat);
    d->seat = nullptr;
    d->resetSeat();
}

void Seat::Private::resetSeat()
{
    setHasKeyboard(false);
    setHasPointer(false);
    setHasTouch(false);
    setName(QString());
}

void Seat::Private::setHasKeyboard(bool has)
{
    if (capabilityKeyboard == has) {
        return;
    }
    capabilityKeyboard = has;
    emit q->hasKeyboardChanged(capabilityKeyboard);
}

void Seat::Private::setHasPointer(bool has)
{
    if (capabilityPointer == has) {
        return;
    }
    capabilityPointer = has;
    emit q->hasPointerChanged(capabilityPointer);
}

void Seat::Private::setHasTouch(bool has)
{
    if (capabilityTouch == has) {
        return;
    }
    capabilityTouch = has;
    emit q->hasTouchChanged(capabilityTouch);
}

void Seat::setup(wl_seat *seat)
{
    d->setup(seat);
}

void Seat::Private::capabilitiesCallback(void *data, wl_seat *seat, uint32_t capabilities)
{
    auto s = reinterpret_cast<Seat::Private*>(data);
    Q_ASSERT(s->seat == seat);
    s->capabilitiesChanged(capabilities);
}

void Seat::Private::nameCallback(void *data, wl_seat *seat, const char *name)
{
    auto s = reinterpret_cast<Seat::Private*>(data);
    Q_ASSERT(s->seat == seat);
    s->setName(QString::fromUtf8(name));
}

void Seat::Private::capabilitiesChanged(uint32_t capabilities)
{
    setHasKeyboard(capabilities & WL_SEAT_CAPABILITY_KEYBOARD);
    setHasPointer(capabilities & WL_SEAT_CAPABILITY_POINTER);
    setHasTouch(capabilities & WL_SEAT_CAPABILITY_TOUCH);
}

Keyboard *Seat::createKeyboard(QObject *parent)
{
    Q_ASSERT(isValid());
    Q_ASSERT(d->capabilityKeyboard);
    Keyboard *k = new Keyboard(parent);
    k->setup(wl_seat_get_keyboard(d->seat));
    return k;
}

Pointer *Seat::createPointer(QObject *parent)
{
    Q_ASSERT(isValid());
    Q_ASSERT(d->capabilityPointer);
    Pointer *p = new Pointer(parent);
    p->setup(wl_seat_get_pointer(d->seat));
    return p;
}

#if 0
wl_touch *Seat::createTouch()
{
    Q_ASSERT(isValid());
    Q_ASSERT(d->capabilityTouch);
    return wl_seat_get_touch(d->seat);
}
#endif

void Seat::Private::setName(const QString &n)
{
    if (name == n) {
        return;
    }
    name = n;
    emit q->nameChanged(name);
}

bool Seat::isValid() const
{
    return d->seat != nullptr;
}

bool Seat::hasKeyboard() const
{
    return d->capabilityKeyboard;
}

bool Seat::hasPointer() const
{
    return d->capabilityPointer;
}

bool Seat::hasTouch() const
{
    return d->capabilityTouch;
}

QString Seat::name() const
{
    return d->name;
}

Seat::operator wl_seat*()
{
    return d->seat;
}

Seat::operator wl_seat*() const
{
    return d->seat;
}

}
}
