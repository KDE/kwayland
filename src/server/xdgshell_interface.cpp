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
#include "xdgshell_interface_p.h"

namespace KWayland
{
namespace Server
{

XdgShellInterface::Private::Private(XdgShellInterfaceVersion interfaceVersion, XdgShellInterface *q, Display *d, const wl_interface *interface, quint32 version)
    : Global::Private(d, interface, version)
    , interfaceVersion(interfaceVersion)
    , pingTimer(new QTimer)
    , q(q)
{
    pingTimer->setSingleShot(true);
    pingTimer->setInterval(1000);
}

XdgShellInterface::XdgShellInterface(Private *d, QObject *parent)
    : Global(d, parent)
{
}

XdgShellInterface::~XdgShellInterface() = default;

XdgShellSurfaceInterface *XdgShellInterface::getSurface(wl_resource *native)
{
    Q_UNUSED(native)
    return nullptr;
}

XdgShellInterfaceVersion XdgShellInterface::interfaceVersion() const
{
    Q_D();
    return d->interfaceVersion;
}

void XdgShellInterface::ping()
{
    d_func()->ping();
}

XdgShellInterface::Private *XdgShellInterface::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}

XdgShellSurfaceInterface::Private::Private(XdgShellInterfaceVersion interfaceVersion, XdgShellSurfaceInterface *q, Global *c, SurfaceInterface *surface, wl_resource *parentResource, const wl_interface *interface, const void *implementation)
    : Resource::Private(q, c, parentResource, interface, implementation)
    , GenericShellSurface<XdgShellSurfaceInterface>(q, surface)
    , interfaceVersion(interfaceVersion)
{
}

XdgShellSurfaceInterface::Private::~Private() = default;

XdgShellSurfaceInterface::XdgShellSurfaceInterface(Private *p)
    : Resource(p)
{
}

XdgShellSurfaceInterface::~XdgShellSurfaceInterface() = default;

XdgShellInterfaceVersion XdgShellSurfaceInterface::interfaceVersion() const
{
    Q_D();
    return d->interfaceVersion;
}

quint32 XdgShellSurfaceInterface::configure(States states, const QSize &size)
{
    Q_D();
    return d->configure(states, size);
}

bool XdgShellSurfaceInterface::isConfigurePending() const
{
    Q_D();
    return !d->configureSerials.isEmpty();
}

SurfaceInterface *XdgShellSurfaceInterface::surface() const
{
    Q_D();
    return d->surface;
}

QString XdgShellSurfaceInterface::title() const
{
    Q_D();
    return d->title;
}

QByteArray XdgShellSurfaceInterface::windowClass() const
{
    Q_D();
    return d->windowClass;
}

bool XdgShellSurfaceInterface::isTransient() const
{
    Q_D();
    return !d->parent.isNull();
}

QPointer<XdgShellSurfaceInterface> XdgShellSurfaceInterface::transientFor() const
{
    Q_D();
    return d->parent;
}

void XdgShellSurfaceInterface::close()
{
    Q_D();
    d->close();
}

XdgShellSurfaceInterface::Private *XdgShellSurfaceInterface::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}

XdgShellPopupInterface::Private::Private(XdgShellInterfaceVersion interfaceVersion, XdgShellPopupInterface *q, XdgShellInterface *c, SurfaceInterface *surface, wl_resource *parentResource, const wl_interface *interface, const void *implementation)
    : Resource::Private(q, c, parentResource, interface, implementation)
    , GenericShellSurface<XdgShellPopupInterface>(q, surface)
    , interfaceVersion(interfaceVersion)
{
}

XdgShellPopupInterface::Private::~Private() = default;

XdgShellPopupInterface::XdgShellPopupInterface(Private *p)
    : Resource(p)
{
}

XdgShellPopupInterface::~XdgShellPopupInterface() = default;

SurfaceInterface *XdgShellPopupInterface::surface() const
{
    Q_D();
    return d->surface;
}

QPointer<SurfaceInterface> XdgShellPopupInterface::transientFor() const
{
    Q_D();
    return d->parent;
}

QSize XdgShellPopupInterface::initialSize() const
{
    Q_D();
    return d->initialSize;
}

QPoint XdgShellPopupInterface::transientOffset() const
{
    const QPoint center = anchorRect().center();
    const QRect rect = anchorRect().adjusted(0,0,1,1); //compensate for the stupid QRect::right fiasco

    qDebug() << "anchor edge is " << anchorEdge();

    switch(anchorEdge()) {
        case Qt::TopEdge | Qt::LeftEdge:
            return rect.topLeft();
        case Qt::TopEdge:
            return QPoint(center.x(), rect.y());
        case Qt::TopEdge | Qt::RightEdge:
            return rect.topRight();
        case Qt::RightEdge:
            return QPoint(rect.right(), center.y());
        case Qt::BottomEdge | Qt::RightEdge:
            return rect.bottomRight();
        case Qt::BottomEdge:
                return QPoint(center.x(), anchorRect().bottom());
        case Qt::BottomEdge | Qt::LeftEdge:
            return rect.bottomLeft();
        case Qt::LeftEdge:
                return QPoint(rect.left(), center.y());
        default:
            return center;
    }
    Q_UNREACHABLE();
}

QRect XdgShellPopupInterface::anchorRect() const
{
    Q_D();
    return d->anchorRect;
}

Qt::Edges XdgShellPopupInterface::anchorEdge() const
{
    Q_D();
    return d->anchorEdge;
}

Qt::Edges XdgShellPopupInterface::gravity() const
{
    Q_D();
    return d->gravity;
}

QPoint XdgShellPopupInterface::anchorOffset() const
{
    Q_D();
    return d->anchorOffset;
}

PositionerConstraints XdgShellPopupInterface::constraintAdjustments() const
{
    Q_D();
    return d->constraintAdjustments;
}

void XdgShellPopupInterface::popupDone()
{
    Q_D();
    return d->popupDone();
}

quint32 XdgShellPopupInterface::configure(const QRect &rect)
{
    Q_D();
    return d->configure(rect);
}

XdgShellPopupInterface::Private *XdgShellPopupInterface::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}

}
}
