/****************************************************************************
Copyright 2016  Sebastian Kügler <sebas@kde.org>

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
#include "windowmetadatamap.h"
#include "event_queue.h"
#include "wayland_pointer_p.h"
#include "wayland-windowmetadatamap-client-protocol.h"

#include <QDebug>

namespace KWayland
{
namespace Client
{

class WindowMetadataMap::Private
{
public:
    Private() = default;

    WaylandPointer<org_kde_kwin_windowmetadatamap, org_kde_kwin_windowmetadatamap_destroy> windowmetadatamap;
    EventQueue *queue = nullptr;
};

WindowMetadataMap::WindowMetadataMap(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

WindowMetadataMap::~WindowMetadataMap()
{
    release();
}

void WindowMetadataMap::setup(org_kde_kwin_windowmetadatamap *windowmetadatamap)
{
    Q_ASSERT(windowmetadatamap);
    Q_ASSERT(!d->windowmetadatamap);
    d->windowmetadatamap.setup(windowmetadatamap);
}

void WindowMetadataMap::release()
{
    d->windowmetadatamap.release();
}

void WindowMetadataMap::destroy()
{
    d->windowmetadatamap.destroy();
}

void WindowMetadataMap::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *WindowMetadataMap::eventQueue()
{
    return d->queue;
}

WindowMetadataMap::operator org_kde_kwin_windowmetadatamap*() {
    return d->windowmetadatamap;
}

WindowMetadataMap::operator org_kde_kwin_windowmetadatamap*() const {
    return d->windowmetadatamap;
}

bool WindowMetadataMap::isValid() const
{
    return d->windowmetadatamap.isValid();
}

void WindowMetadataMap::registerClient(const QString & serviceName, Surface *surface)
{
    qDebug() << "Registering client" << serviceName;
    wl_surface* ss; // FIXME
    org_kde_kwin_windowmetadatamap_register_client(d->windowmetadatamap, serviceName.toLocal8Bit(), ss);
}


}
}

