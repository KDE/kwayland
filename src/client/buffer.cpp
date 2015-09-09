/********************************************************************
Copyright 2013  Martin Gräßlin <mgraesslin@kde.org>

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
#include "buffer.h"
#include "buffer_p.h"
#include "shm_pool.h"
// system
#include <string.h>
// wayland
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct wl_buffer_listener Buffer::Private::s_listener = {
    Buffer::Private::releasedCallback
};
#endif

Buffer::Private::Private(Buffer *q, ShmPool *parent, wl_buffer *nativeBuffer, const QSize &size, int32_t stride, size_t offset, Format format)
    : shm(parent)
    , nativeBuffer(nativeBuffer)
    , released(false)
    , size(size)
    , stride(stride)
    , offset(offset)
    , used(false)
    , format(format)
    , q(q)
{
    wl_buffer_add_listener(nativeBuffer, &s_listener, this);
}

Buffer::Private::~Private()
{
    nativeBuffer.release();
}

void Buffer::Private::destroy()
{
    nativeBuffer.destroy();
}

void Buffer::Private::releasedCallback(void *data, wl_buffer *buffer)
{
    auto b = reinterpret_cast<Buffer::Private*>(data);
    Q_ASSERT(b->nativeBuffer == buffer);
    b->q->setReleased(true);
}

Buffer::Buffer(ShmPool *parent, wl_buffer *buffer, const QSize &size, int32_t stride, size_t offset, Format format)
    : d(new Private(this, parent, buffer, size, stride, offset, format))
{
}

Buffer::~Buffer() = default;

void Buffer::copy(const void *src)
{
    memcpy(address(), src, d->size.height()*d->stride);
}

uchar *Buffer::address()
{
    return reinterpret_cast<uchar*>(d->shm->poolAddress()) + d->offset;
}

wl_buffer *Buffer::buffer() const
{
    return d->nativeBuffer;
}

Buffer::operator wl_buffer*()
{
    return d->nativeBuffer;
}

Buffer::operator wl_buffer*() const
{
    return d->nativeBuffer;
}

bool Buffer::isReleased() const
{
    return d->released;
}

void Buffer::setReleased(bool released)
{
    d->released = released;
}

QSize Buffer::size() const
{
    return d->size;
}

int32_t Buffer::stride() const
{
    return d->stride;
}

bool Buffer::isUsed() const
{
    return d->used;
}

void Buffer::setUsed(bool used)
{
    d->used = used;
}

Buffer::Format Buffer::format() const
{
    return d->format;
}

quint32 Buffer::getId(wl_buffer *b)
{
    return wl_proxy_get_id(reinterpret_cast<wl_proxy*>(b));
}

}
}
