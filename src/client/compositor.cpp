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
#include "compositor.h"
#include "event_queue.h"
#include "region.h"
#include "surface.h"
#include "wayland_pointer_p.h"
// Qt
#include <QGuiApplication>
#include <QRegion>
#include <qpa/qplatformnativeinterface.h>

#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{

class Q_DECL_HIDDEN Compositor::Private
{
public:
    Private() = default;

    WaylandPointer<wl_compositor, wl_compositor_destroy> compositor;
    EventQueue *queue = nullptr;
};

Compositor::Compositor(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

Compositor::~Compositor()
{
    release();
}

Compositor *Compositor::fromApplication(QObject *parent)
{
    if (!QGuiApplication::platformName().contains(QStringLiteral("wayland"), Qt::CaseInsensitive)) {
        return nullptr;
    }
    QPlatformNativeInterface *native = qApp->platformNativeInterface();
    if (!native) {
        return nullptr;
    }
    wl_compositor *compositor = reinterpret_cast<wl_compositor*>(native->nativeResourceForIntegration(QByteArrayLiteral("compositor")));
    if (!compositor) {
        return nullptr;
    }
    Compositor *c = new Compositor(parent);
    c->d->compositor.setup(compositor, true);
    return c;
}

void Compositor::setup(wl_compositor *compositor)
{
    Q_ASSERT(compositor);
    Q_ASSERT(!d->compositor);
    d->compositor.setup(compositor);
}

void Compositor::release()
{
    d->compositor.release();
}

void Compositor::destroy()
{
    d->compositor.destroy();
}

void Compositor::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *Compositor::eventQueue()
{
    return d->queue;
}

Surface *Compositor::createSurface(QObject *parent)
{
    Q_ASSERT(isValid());
    Surface *s = new Surface(parent);
    auto w = wl_compositor_create_surface(d->compositor);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

Region *Compositor::createRegion(QObject *parent)
{
    return createRegion(QRegion(), parent);
}

Region *Compositor::createRegion(const QRegion &region, QObject *parent)
{
    Q_ASSERT(isValid());
    Region *r = new Region(region, parent);
    auto w = wl_compositor_create_region(d->compositor);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    r->setup(w);
    return r;
}

std::unique_ptr< Region > Compositor::createRegion(const QRegion &region)
{
    return std::unique_ptr<Region>(createRegion(region, nullptr));
}

Compositor::operator wl_compositor*() {
    return d->compositor;
}

Compositor::operator wl_compositor*() const {
    return d->compositor;
}

bool Compositor::isValid() const
{
    return d->compositor.isValid();
}

}
}
