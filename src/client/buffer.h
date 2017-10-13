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
#ifndef WAYLAND_BUFFER_H
#define WAYLAND_BUFFER_H

#include <QSize>
#include <QScopedPointer>
#include <QWeakPointer>

#include <KWayland/Client/kwaylandclient_export.h>

struct wl_buffer;

namespace KWayland
{
namespace Client
{

class ShmPool;

/**
 * @short Wrapper class for wl_buffer interface.
 *
 * The Buffer is provided by ShmPool and is owned by ShmPool.
 *
 * @see ShmPool
 **/
class KWAYLANDCLIENT_EXPORT Buffer
{
public:
    /**
     * All image formats supported by the implementation.
     **/
    enum class Format {
        ARGB32, ///< 32-bit ARGB format, can be used for QImage::Format_ARGB32 and QImage::Format_ARGB32_Premultiplied
        RGB32 ///< 32-bit RGB format, can be used for QImage::Format_RGB32
    };

    ~Buffer();
    /**
     * Copies the data from @p src into the Buffer.
     **/
    void copy(const void *src);
    /**
     * Sets the Buffer as @p released.
     * This is automatically invoked when the Wayland server sends the release event.
     * @param released Whether the Buffer got released by the Wayland server.
     * @see isReleased
     **/
    void setReleased(bool released);
    /**
     * Sets whether the Buffer is used.
     * If the Buffer may not be reused when it gets released, the user of a Buffer should
     * mark the Buffer as used. This is needed for example when the memory is shared with
     * a QImage. As soon as the Buffer can be reused again one should call this method with
     * @c false again.
     *
     * By default a Buffer is not used.
     *
     * @param used Whether the Bufer should be marked as used.
     * @see isUsed
     **/
    void setUsed(bool used);

    wl_buffer *buffer() const;
    /**
     * @returns The size of this Buffer.
     **/
    QSize size() const;
    /**
     * @returns The stride (bytes per line) of this Buffer.
     **/
    int32_t stride() const;
    /**
     * @returns @c true if the Wayland server doesn't need the Buffer anymore.
     **/
    bool isReleased() const;
    /**
     * @returns @c true if the Buffer's user is still needing the Buffer.
     **/
    bool isUsed() const;
    /**
     * @returns the memory address of this Buffer.
     **/
    uchar *address();
    /**
     * @returns The image format used by this Buffer.
     **/
    Format format() const;

    operator wl_buffer*();
    operator wl_buffer*() const;

    typedef QWeakPointer<Buffer> Ptr;

    /**
     * Helper method to get the id for a provided native buffer.
     * @since 5.3
     **/
    static quint32 getId(wl_buffer *b);

private:
    friend class ShmPool;
    explicit Buffer(ShmPool *parent, wl_buffer *buffer, const QSize &size, int32_t stride, size_t offset, Format format);
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
