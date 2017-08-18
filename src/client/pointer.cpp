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
#include "pointer.h"
#include "surface.h"
#include "wayland_pointer_p.h"
// Qt
#include <QPointF>
#include <QPointer>
// wayland
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{

class Q_DECL_HIDDEN Pointer::Private
{
public:
    Private(Pointer *q);
    void setup(wl_pointer *p);

    WaylandPointer<wl_pointer, wl_pointer_release> pointer;
    QPointer<Surface> enteredSurface;
    quint32 enteredSerial = 0;
private:
    void enter(uint32_t serial, wl_surface *surface, const QPointF &relativeToSurface);
    void leave(uint32_t serial);
    static void enterCallback(void *data, wl_pointer *pointer, uint32_t serial, wl_surface *surface,
                              wl_fixed_t sx, wl_fixed_t sy);
    static void leaveCallback(void *data, wl_pointer *pointer, uint32_t serial, wl_surface *surface);
    static void motionCallback(void *data, wl_pointer *pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy);
    static void buttonCallback(void *data, wl_pointer *pointer, uint32_t serial, uint32_t time,
                               uint32_t button, uint32_t state);
    static void axisCallback(void *data, wl_pointer *pointer, uint32_t time, uint32_t axis, wl_fixed_t value);

    Pointer *q;
    static const wl_pointer_listener s_listener;
};

Pointer::Private::Private(Pointer *q)
    : q(q)
{
}

void Pointer::Private::setup(wl_pointer *p)
{
    Q_ASSERT(p);
    Q_ASSERT(!pointer);
    pointer.setup(p);
    wl_pointer_add_listener(pointer, &s_listener, this);
}

const wl_pointer_listener Pointer::Private::s_listener = {
    enterCallback,
    leaveCallback,
    motionCallback,
    buttonCallback,
    axisCallback
};

Pointer::Pointer(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

Pointer::~Pointer()
{
    release();
}

void Pointer::release()
{
    d->pointer.release();
}

void Pointer::destroy()
{
    d->pointer.destroy();
}

void Pointer::setup(wl_pointer *pointer)
{
    d->setup(pointer);
}

void Pointer::Private::enterCallback(void *data, wl_pointer *pointer, uint32_t serial, wl_surface *surface,
                            wl_fixed_t sx, wl_fixed_t sy)
{
    auto p = reinterpret_cast<Pointer::Private*>(data);
    Q_ASSERT(p->pointer == pointer);
    p->enter(serial, surface, QPointF(wl_fixed_to_double(sx), wl_fixed_to_double(sy)));
}

void Pointer::Private::enter(uint32_t serial, wl_surface *surface, const QPointF &relativeToSurface)
{
    enteredSurface = QPointer<Surface>(Surface::get(surface));
    enteredSerial = serial;
    emit q->entered(serial, relativeToSurface);
}

void Pointer::Private::leaveCallback(void *data, wl_pointer *pointer, uint32_t serial, wl_surface *surface)
{
    auto p = reinterpret_cast<Pointer::Private*>(data);
    Q_ASSERT(p->pointer == pointer);
    Q_UNUSED(surface)
    p->leave(serial);
}

void Pointer::Private::leave(uint32_t serial)
{
    enteredSurface.clear();
    emit q->left(serial);
}

void Pointer::Private::motionCallback(void *data, wl_pointer *pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy)
{
    auto p = reinterpret_cast<Pointer::Private*>(data);
    Q_ASSERT(p->pointer == pointer);
    emit p->q->motion(QPointF(wl_fixed_to_double(sx), wl_fixed_to_double(sy)), time);
}

void Pointer::Private::buttonCallback(void *data, wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
    auto p = reinterpret_cast<Pointer::Private*>(data);
    Q_ASSERT(p->pointer == pointer);
    auto toState = [state] {
        if (state == WL_POINTER_BUTTON_STATE_RELEASED) {
            return ButtonState::Released;
        } else {
            return ButtonState::Pressed;
        }
    };
    emit p->q->buttonStateChanged(serial, time, button, toState());
}

void Pointer::Private::axisCallback(void *data, wl_pointer *pointer, uint32_t time, uint32_t axis, wl_fixed_t value)
{
    auto p = reinterpret_cast<Pointer::Private*>(data);
    Q_ASSERT(p->pointer == pointer);
    auto toAxis = [axis] {
        if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL) {
            return Axis::Horizontal;
        } else {
            return Axis::Vertical;
        }
    };
    emit p->q->axisChanged(time, toAxis(), wl_fixed_to_double(value));
}

void Pointer::setCursor(Surface *surface, const QPoint &hotspot)
{
    Q_ASSERT(isValid());
    wl_surface *s = nullptr;
    if (surface) {
        s = *surface;
    }
    wl_pointer_set_cursor(d->pointer, d->enteredSerial, s, hotspot.x(), hotspot.y());
}

void Pointer::hideCursor()
{
    setCursor(nullptr);
}

Surface *Pointer::enteredSurface()
{
    return d->enteredSurface.data();
}

Surface *Pointer::enteredSurface() const
{
    return d->enteredSurface.data();
}

bool Pointer::isValid() const
{
    return d->pointer.isValid();
}

Pointer::operator wl_pointer*() const
{
    return d->pointer;
}

Pointer::operator wl_pointer*()
{
    return d->pointer;
}

}
}
