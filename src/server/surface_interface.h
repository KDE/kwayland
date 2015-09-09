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
#ifndef WAYLAND_SERVER_SURFACE_INTERFACE_H
#define WAYLAND_SERVER_SURFACE_INTERFACE_H

#include "resource.h"
#include "output_interface.h"

#include <QObject>
#include <QPointer>
#include <QRegion>

#include <KWayland/Server/kwaylandserver_export.h>

namespace KWayland
{
namespace Server
{
class BlurManagerInterface;
class BlurInterface;
class BufferInterface;
class ContrastInterface;
class ContrastManagerInterface;
class CompositorInterface;
class ShadowManagerInterface;
class ShadowInterface;
class SlideInterface;
class SubSurfaceInterface;

class KWAYLANDSERVER_EXPORT SurfaceInterface : public Resource
{
    Q_OBJECT
    Q_PROPERTY(QRegion damage READ damage NOTIFY damaged)
    Q_PROPERTY(QRegion opaque READ opaque NOTIFY opaqueChanged)
    Q_PROPERTY(QRegion input READ input NOTIFY inputChanged)
    Q_PROPERTY(qint32 scale READ scale NOTIFY scaleChanged)
    Q_PROPERTY(KWayland::Server::OutputInterface::Transform transform READ transform NOTIFY transformChanged)
    Q_PROPERTY(QSize size READ size NOTIFY sizeChanged)
public:
    virtual ~SurfaceInterface();

    void frameRendered(quint32 msec);

    QRegion damage() const;
    QRegion opaque() const;
    QRegion input() const;
    bool inputIsInfitine() const;
    qint32 scale() const;
    OutputInterface::Transform transform() const;
    BufferInterface *buffer();
    QPoint offset() const;
    /**
     * The size of the Surface.
     * @since 5.3
     **/
    QSize size() const;

    /**
     * @returns The SubSurface for this Surface in case there is one.
     **/
    QPointer<SubSurfaceInterface> subSurface() const;
    /**
     * @returns Children in stacking order from bottom (first) to top (last).
     **/
    QList<QPointer<SubSurfaceInterface>> childSubSurfaces() const;

    /**
     * @returns The Shadow for this Surface.
     * @since 5.4
     **/
    QPointer<ShadowInterface> shadow() const;

    /**
     * @returns The Blur for this Surface.
     * @since 5.5
     **/
    QPointer<BlurInterface> blur() const;

    /**
     * @returns The Slide for this Surface.
     * @since 5.5
     **/
    QPointer<SlideInterface> slideOnShowHide() const;

    /**
     * @returns The Contrast for this Surface.
     * @since 5.5
     **/
    QPointer<ContrastInterface> contrast() const;

    static SurfaceInterface *get(wl_resource *native);
    /**
     * @returns The SurfaceInterface with given @p id for @p client, if it exists, otherwise @c nullptr.
     * @since 5.3
     **/
    static SurfaceInterface *get(quint32 id, const ClientConnection *client);

Q_SIGNALS:
    void damaged(const QRegion&);
    void opaqueChanged(const QRegion&);
    void inputChanged(const QRegion&);
    void scaleChanged(qint32);
    void transformChanged(KWayland::Server::OutputInterface::Transform);
    /**
     * Emitted when the Surface removes its content
     **/
    void unmapped();
    /**
     * @since 5.3
     **/
    void sizeChanged();
    /**
     * @since 5.4
     **/
    void shadowChanged();
    /**
     * @since 5.5
     **/
    void blurChanged();
    /**
     * @since 5.5
     **/
    void slideOnShowHideChanged();
    /**
     * @since 5.5
     **/
    void contrastChanged();

private:
    friend class CompositorInterface;
    friend class SubSurfaceInterface;
    friend class ShadowManagerInterface;
    friend class BlurManagerInterface;
    friend class SlideManagerInterface;
    friend class ContrastManagerInterface;
    explicit SurfaceInterface(CompositorInterface *parent, wl_resource *parentResource);

    class Private;
    Private *d_func() const;
};

}
}

#endif
