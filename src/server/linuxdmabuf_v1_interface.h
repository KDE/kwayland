/********************************************************************
Copyright © 2018 Fredrik Höglund <fredrik@kde.org>

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

#ifndef WAYLAND_SERVER_LINUXDMABUF_INTERFACE_H
#define WAYLAND_SERVER_LINUXDMABUF_INTERFACE_H

#include <KWayland/Server/kwaylandserver_export.h>
#include "global.h"
#include "resource.h"

#include <QSize>


struct wl_buffer_interface;

namespace KWayland
{

namespace Server
{

namespace LinuxDmabuf
{
    enum Flag {
        YInverted        = (1 << 0), /// Contents are y-inverted
        Interlaced       = (1 << 1), /// Content is interlaced
        BottomFieldFirst = (1 << 2)  /// Bottom field first
    };

    Q_DECLARE_FLAGS(Flags, Flag)

    /**
     * Represents a plane in the buffer
     */
    struct Plane {
        int fd;              /// The dmabuf file descriptor
        uint32_t offset;     /// The offset from the start of buffer
        uint32_t stride;     /// The distance from the start of a row to the next row in bytes
        uint64_t modifier;   /// The layout modifier
    };

    /**
     * The base class for linux-dmabuf buffers
     *
     * Compositors should reimplement this class to store objects specific
     * to the underlying graphics stack.
     */
    class Buffer {
    public:
        /**
         * Creates a new Buffer.
         */
        Buffer(uint32_t format, const QSize &size) : m_format(format), m_size(size) {}

        /**
         * Destroys the Buffer.
         */
        virtual ~Buffer() = default;

        /**
         * Returns the DRM format code for the buffer.
         */
        uint32_t format() const { return m_format; }

        /**
         * Returns the size of the buffer.
         */
        QSize size() const { return m_size; }

    private:
        uint32_t m_format;
        QSize m_size;
    };
}


/**
 * Represents the global zpw_linux_dmabuf_v1 interface.
 *
 * This interface provides a way for clients to create generic dmabuf based wl_buffers.
 */
class KWAYLANDSERVER_EXPORT LinuxDmabufUnstableV1Interface : public Global
{
    Q_OBJECT

public:
    /**
     * The Bridge class provides an interface from the LinuxDmabufInterface to the compositor
     */
    class Bridge {
    public:
        Bridge() = default;
        virtual ~Bridge() = default;

        /**
         * Returns the DRM format codes supported by the compositor.
         */
        virtual QVector<uint32_t> supportedFormats() const = 0;

        /**
         * Returns the layout-modifiers supported for the given DRM format code.
         */
        virtual QVector<uint64_t> supportedModifiers(uint32_t format) const = 0;

        /**
         * Imports a linux-dmabuf buffer into the compositor.
         *
         * The returned buffer takes ownership of the file descriptor for each plane.
         * Note that it is the responsibility of the caller to close the file descriptors
         * when the import fails.
         *
         * @return The imported buffer on success, and nullptr otherwise.
         */
        virtual LinuxDmabuf::Buffer *importBuffer(const QVector<LinuxDmabuf::Plane> &planes,
                                                  uint32_t format,
                                                  const QSize &size,
                                                  LinuxDmabuf::Flags flags) = 0;
    };

    /**
     * Destroys the LinuxDmabufUnstableV1Interface.
     */
    virtual ~LinuxDmabufUnstableV1Interface();

    /**
     * Sets the compositor bridge for the dmabuf interface.
     */
    void setBridge(Bridge *bridge);

    /**
     * Returns the compositor bridge for the dmabuf interface.
     */
    Bridge *bridge() const;

    /**
     * Returns the LinuxDmabufInterface for the given resource.
     **/
    static LinuxDmabufUnstableV1Interface *get(wl_resource *native);

    /**
     * Returns a pointer to the wl_buffer implementation for imported dmabufs.
     */
    static const struct wl_buffer_interface *bufferImplementation();

private:
    /**
     * @internal
     */
    explicit LinuxDmabufUnstableV1Interface(Display *display, QObject *parent = nullptr);
    friend class Display;

private:
    class Private;
    Private *d_func() const;
};


} // namespace Server
} // namespace KWayland

Q_DECLARE_METATYPE(KWayland::Server::LinuxDmabufUnstableV1Interface*)
Q_DECLARE_OPERATORS_FOR_FLAGS(KWayland::Server::LinuxDmabuf::Flags)

#endif // WAYLAND_SERVER_LINUXDMABUF_INTERFACE_H
