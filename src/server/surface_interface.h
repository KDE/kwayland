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

/**
 * @brief Resource representing a wl_surface.
 *
 * The SurfaceInterface gets created by the CompositorInterface. A SurfaceInterface normally
 * takes up a role by being "attached" to either a ShellSurfaceInterface, a SubSurfaceInterface
 * or a Cursor.
 *
 * The implementation of the SurfaceInterface does not only wrap the features exposed by wl_surface,
 * but goes further by integrating the information added to a SurfaceInterface by other interfaces.
 * This should make interacting from the server easier, it only needs to monitor the SurfaceInterface
 * and does not need to track each specific interface.
 *
 * The SurfaceInterface takes care of reference/unreferencing the BufferInterface attached to it.
 * As long as a BufferInterface is attached, the released signal won't be sent. If the BufferInterface
 * is no longer needed by the SurfaceInterface, it will get unreferenced and might be automatically
 * deleted (if it's no longer referenced).
 *
 * @see CompositorInterface
 * @see BufferInterface
 * @see SubSurfaceInterface
 * @see BlurInterface
 * @see ContrastInterface
 * @see ShadowInterface
 * @see SlideInterface
 **/
class KWAYLANDSERVER_EXPORT SurfaceInterface : public Resource
{
    Q_OBJECT
    /**
     * The current damage region.
     **/
    Q_PROPERTY(QRegion damage READ damage NOTIFY damaged)
    /**
     * The opaque region for a translucent buffer.
     **/
    Q_PROPERTY(QRegion opaque READ opaque NOTIFY opaqueChanged)
    /**
     * The current input region.
     **/
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
    /**
     * Use Surface::inputIsInfinite instead.
     * @deprecated
     * @see inputIsInfinite
     */
    bool inputIsInfitine() const;
    /**
     * Replaces Surface::inputIsInfitine instead.
     * @since 5.5
     */
    bool inputIsInfinite() const;
    qint32 scale() const;
    OutputInterface::Transform transform() const;
    /**
     * @returns the current BufferInterface, might be @c nullptr.
     **/
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

    /**
     * @returns The SurfaceInterface for the @p native resource.
     **/
    static SurfaceInterface *get(wl_resource *native);
    /**
     * @returns The SurfaceInterface with given @p id for @p client, if it exists, otherwise @c nullptr.
     * @since 5.3
     **/
    static SurfaceInterface *get(quint32 id, const ClientConnection *client);

Q_SIGNALS:
    /**
     * Emitted whenever the SurfaceInterface got damaged.
     * The signal is only emitted during the commit of state.
     * A damage means that a new BufferInterface got attached.
     *
     * @see buffer
     * @see damage
     **/
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
