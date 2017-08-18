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
#include "subcompositor.h"
#include "event_queue.h"
#include "subsurface.h"
#include "surface.h"
#include "wayland_pointer_p.h"
// Wayland
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{

class Q_DECL_HIDDEN SubCompositor::Private
{
public:
    WaylandPointer<wl_subcompositor, wl_subcompositor_destroy> subCompositor;
    EventQueue *queue = nullptr;
};

SubCompositor::SubCompositor(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

SubCompositor::~SubCompositor()
{
    release();
}

void SubCompositor::release()
{
    d->subCompositor.release();
}

void SubCompositor::destroy()
{
    d->subCompositor.destroy();
}

void SubCompositor::setup(wl_subcompositor *subcompositor)
{
    Q_ASSERT(subcompositor);
    Q_ASSERT(!d->subCompositor.isValid());
    d->subCompositor.setup(subcompositor);
}

SubSurface *SubCompositor::createSubSurface(QPointer<Surface> surface, QPointer<Surface> parentSurface, QObject *parent)
{
    Q_ASSERT(isValid());
    SubSurface *s = new SubSurface(surface, parentSurface, parent);
    auto w = wl_subcompositor_get_subsurface(d->subCompositor, *surface, *parentSurface);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

bool SubCompositor::isValid() const
{
    return d->subCompositor.isValid();
}

SubCompositor::operator wl_subcompositor*()
{
    return d->subCompositor;
}

SubCompositor::operator wl_subcompositor*() const
{
    return d->subCompositor;
}

EventQueue *SubCompositor::eventQueue()
{
    return d->queue;
}

void SubCompositor::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

}
}
