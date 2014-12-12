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
#ifndef WAYLAND_SURFACE_H
#define WAYLAND_SURFACE_H

#include "buffer.h"

#include <QObject>
#include <QPoint>
#include <QSize>

#include <KWayland/Client/kwaylandclient_export.h>

struct wl_buffer;
struct wl_surface;

namespace KWayland
{
namespace Client
{

class Region;

/**
 * @short Wrapper for the wl_surface interface.
 *
 * This class is a convenient wrapper for the wl_surface interface.
 * To create a Surface call Compositor::createSurface.
 *
 * The main purpose of this class is to setup the next frame which
 * should be rendered. Therefore it provides methods to add damage
 * and to attach a new Buffer and to finalize the frame by calling
 * commit.
 *
 * @see Compositor
 **/
class KWAYLANDCLIENT_EXPORT Surface : public QObject
{
    Q_OBJECT
public:
    explicit Surface(QObject *parent = nullptr);
    virtual ~Surface();

    /**
     * Setup this Surface to manage the @p surface.
     * When using Compositor::createSurface there is no need to call this
     * method.
     **/
    void setup(wl_surface *surface);
    /**
     * Releases the wl_surface interface.
     * After the interface has been released the Surface instance is no
     * longer valid and can be setup with another wl_surface interface.
     **/
    void release();
    /**
     * Destroys the data hold by this Surface.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid any more, it's not
     * possible to call release any more as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or setup to a new wl_surface interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, surface, &Surface::destroyed);
     * @endcode
     *
     * @see release
     **/
    void destroy();
    /**
     * @returns @c true if managing a wl_surface.
     **/
    bool isValid() const;
    /**
     * Registers a frame rendered callback.
     * This registers a callback in the Wayland server to be notified once the
     * next frame for this Surface has been rendered. The Surface will emit the
     * signal frameRendered after receiving the callback from the server.
     *
     * Instead of using this method one should prefer using the CommitFlag::FrameCallback
     * in commit. This method is intended for cases when the Surface is going to
     * be committed on other ways, e.g. through the OpenGL/EGL stack.
     *
     * @see frameRendered
     * @see commit
     **/
    void setupFrameCallback();
    /**
     * Flags to be added to commit.
     * @li None: no flag
     * @li FrameCallback: register a frame rendered callback
     *
     * Instead of setting the FrameCallback flag one can also call
     * setupFrameCallback. If one uses setupFrameCallback one may not
     * use the FrameCallback flag when committing the Surface.
     *
     * @see commit
     * @see setupFrameCallback
     **/
    enum class CommitFlag {
        None,
        FrameCallback
    };
    void commit(CommitFlag flag = CommitFlag::FrameCallback);
    /**
     * Mark @p rect as damaged for the next frame.
     **/
    void damage(const QRect &rect);
    /**
     * Mark @p region as damaged for the next frame.
     **/
    void damage(const QRegion &region);
    /**
     * Attaches the @p buffer to this Surface for the next frame.
     * @param buffer The buffer to attach to this Surface
     * @param offset Position of the new upper-left corner in relation to previous frame
     **/
    void attachBuffer(wl_buffer *buffer, const QPoint &offset = QPoint());
    /**
     * Overloaded method for convenience.
     **/
    void attachBuffer(Buffer *buffer, const QPoint &offset = QPoint());
    /**
     * Overloaded method for convenience.
     **/
    void attachBuffer(Buffer::Ptr buffer, const QPoint &offset = QPoint());
    /**
     * Sets the input region to @p region.
     *
     * This is a double buffered state and will be applied with the next Surface
     * commit. Initially the Surface is set up to an infinite input region.
     * By passing @c null as the input region, it gets reset to an infinite input
     * region.
     *
     * Note: the Region is being copied and can be destroyed directly after passing
     * to this method.
     *
     * @param region The new input region or an infinite region if @c null
     * @see commit
     **/
    void setInputRegion(const Region *region = nullptr);
    /**
     * Sets the opaque region to @p region.
     *
     * This is a double buffered state and will be applied with the next Surface
     * commit. Initially the Surface is set up to an empty opaque region.
     * By passing @c null as the opaque region, it gets reset to an empty opaque
     * region.
     *
     * Note: the Region is being copied and can be destroyed directly after passing
     * to this method.
     *
     * @param region The new opaque region or an empty region if @c null
     * @see commit
     **/
    void setOpaqueRegion(const Region *region = nullptr);
    void setSize(const QSize &size);
    QSize size() const;

    operator wl_surface*();
    operator wl_surface*() const;

    /**
     * All Surfaces which are currently created.
     * TODO: KF6 return QList<Surface*> instead of const-ref
     **/
    static const QList<Surface*> &all(); // krazy:exclude=constref
    /**
     * @returns The Surface referencing the @p native wl_surface or @c null if there is no such Surface.
     **/
    static Surface *get(wl_surface *native);

Q_SIGNALS:
    /**
     * Emitted when the server indicates that the last committed frame has been rendered.
     * The signal will only be emitted if a callback has been registered by either calling
     * setupFrameCallback or calling commit with the CommitFlag::FrameCallback.
     * @see setupFrameCallback
     * @see commit
     **/
    void frameRendered();
    void sizeChanged(const QSize&);

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
