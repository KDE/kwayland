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
#include "shm_pool.h"
#include "buffer.h"
#include "buffer_p.h"
// Qt
#include <QDebug>
#include <QImage>
#include <QTemporaryFile>
// system
#include <unistd.h>
#include <sys/mman.h>
// wayland
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{

class ShmPool::Private
{
public:
    Private(ShmPool *q);
    bool createPool();
    bool resizePool(int32_t newSize);
    wl_shm *shm = nullptr;
    wl_shm_pool *pool = nullptr;
    void *poolData = nullptr;
    int32_t size = 1024;
    QScopedPointer<QTemporaryFile> tmpFile;
    bool valid = false;
    int offset = 0;
    QList<Buffer*> buffers;
private:
    ShmPool *q;
};

ShmPool::Private::Private(ShmPool *q)
    : tmpFile(new QTemporaryFile())
    , q(q)
{
}


ShmPool::ShmPool(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

ShmPool::~ShmPool()
{
    release();
}

void ShmPool::release()
{
    qDeleteAll(d->buffers);
    d->buffers.clear();
    if (d->poolData) {
        munmap(d->poolData, d->size);
        d->poolData = nullptr;
    }
    if (d->pool) {
        wl_shm_pool_destroy(d->pool);
        d->pool = nullptr;
    }
    if (d->shm) {
        wl_shm_destroy(d->shm);
        d->shm = nullptr;
    }
    d->tmpFile->close();
    d->valid = false;
    d->offset = 0;
}

void ShmPool::destroy()
{
    for (auto b : d->buffers) {
        b->d->destroy();
    }
    qDeleteAll(d->buffers);
    d->buffers.clear();
    if (d->poolData) {
        munmap(d->poolData, d->size);
        d->poolData = nullptr;
    }
    if (d->pool) {
        free(d->pool);
        d->pool = nullptr;
    }
    if (d->shm) {
        free(d->shm);
        d->shm = nullptr;
    }
    d->tmpFile->close();
    d->valid = false;
    d->offset = 0;
}

void ShmPool::setup(wl_shm *shm)
{
    Q_ASSERT(shm);
    Q_ASSERT(!d->shm);
    d->shm = shm;
    d->valid = d->createPool();
}

bool ShmPool::Private::createPool()
{
    if (!tmpFile->open()) {
        qDebug() << "Could not open temporary file for Shm pool";
        return false;
    }
    if (ftruncate(tmpFile->handle(), size) < 0) {
        qDebug() << "Could not set size for Shm pool file";
        return false;
    }
    poolData = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, tmpFile->handle(), 0);
    pool = wl_shm_create_pool(shm, tmpFile->handle(), size);

    if (!poolData || !pool) {
        qDebug() << "Creating Shm pool failed";
        return false;
    }
    return true;
}

bool ShmPool::Private::resizePool(int32_t newSize)
{
    if (ftruncate(tmpFile->handle(), newSize) < 0) {
        qDebug() << "Could not set new size for Shm pool file";
        return false;
    }
    wl_shm_pool_resize(pool, newSize);
    munmap(poolData, size);
    poolData = mmap(NULL, newSize, PROT_READ | PROT_WRITE, MAP_SHARED, tmpFile->handle(), 0);
    size = newSize;
    if (!poolData) {
        qDebug() << "Resizing Shm pool failed";
        return false;
    }
    emit q->poolResized();
    return true;
}

static Buffer::Format toBufferFormat(const QImage &image)
{
    switch (image.format()) {
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
        return Buffer::Format::ARGB32;
    case QImage::Format_RGB32:
        return Buffer::Format::RGB32;
    default:
        qWarning() << "Unsupported image format: " << image.format() << "going to use ARGB32, expect rendering errors";
        return Buffer::Format::ARGB32;
    }
};

Buffer *ShmPool::createBuffer(const QImage& image)
{
    if (image.isNull() || !d->valid) {
        return NULL;
    }
    Buffer *buffer = getBuffer(image.size(), image.bytesPerLine(), toBufferFormat(image));
    if (!buffer) {
        return NULL;
    }
    buffer->copy(image.bits());
    return buffer;
}

Buffer *ShmPool::createBuffer(const QSize &size, int32_t stride, const void *src, Buffer::Format format)
{
    if (size.isEmpty() || !d->valid) {
        return NULL;
    }
    Buffer *buffer = getBuffer(size, stride, format);
    if (!buffer) {
        return NULL;
    }
    buffer->copy(src);
    return buffer;
}

static wl_shm_format toWaylandFormat(Buffer::Format format)
{
    switch (format) {
    case Buffer::Format::ARGB32:
        return WL_SHM_FORMAT_ARGB8888;
    case Buffer::Format::RGB32:
        return WL_SHM_FORMAT_XRGB8888;
    }
    abort();
}

Buffer *ShmPool::getBuffer(const QSize &size, int32_t stride, Buffer::Format format)
{
    Q_FOREACH (Buffer *buffer, d->buffers) {
        if (!buffer->isReleased() || buffer->isUsed()) {
            continue;
        }
        if (buffer->size() != size || buffer->stride() != stride || buffer->format() != format) {
            continue;
        }
        buffer->setReleased(false);
        return buffer;
    }
    const int32_t byteCount = size.height() * stride;
    if (d->offset + byteCount > d->size) {
        if (!d->resizePool(d->size + byteCount)) {
            return NULL;
        }
    }
    // we don't have a buffer which we could reuse - need to create a new one
    wl_buffer *native = wl_shm_pool_create_buffer(d->pool, d->offset, size.width(), size.height(),
                                                  stride, toWaylandFormat(format));
    if (!native) {
        return NULL;
    }
    Buffer *buffer = new Buffer(this, native, size, stride, d->offset, format);
    d->offset += byteCount;
    d->buffers.append(buffer);
    return buffer;
}

bool ShmPool::isValid() const
{
    return d->valid;
}

void* ShmPool::poolAddress() const
{
    return d->poolData;
}

wl_shm *ShmPool::shm()
{
    return d->shm;
}

}
}
