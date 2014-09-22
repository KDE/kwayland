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
#include "surface.h"
#include "buffer.h"

#include <QRegion>
#include <QVector>
// Wayland
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{

class Surface::Private
{
public:
    Private(Surface *q);
    void setupFrameCallback();

    wl_surface *surface = nullptr;
    bool frameCallbackInstalled = false;
    QSize size;

    static QList<Surface*> s_surfaces;
private:
    void handleFrameCallback();
    static void frameCallback(void *data, wl_callback *callback, uint32_t time);

    Surface *q;
    static const wl_callback_listener s_listener;
};

QList<Surface*> Surface::Private::s_surfaces = QList<Surface*>();

Surface::Private::Private(Surface *q)
    : q(q)
{
}

Surface::Surface(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
    Private::s_surfaces << this;
}

Surface::~Surface()
{
    Private::s_surfaces.removeAll(this);
    release();
}

void Surface::release()
{
    if (!d->surface) {
        return;
    }
    wl_surface_destroy(d->surface);
    d->surface = nullptr;
}

void Surface::destroy()
{
    if (!d->surface) {
        return;
    }
    free(d->surface);
    d->surface = nullptr;
}

void Surface::setup(wl_surface *surface)
{
    Q_ASSERT(surface);
    Q_ASSERT(!d->surface);
    d->surface = surface;
}

void Surface::Private::frameCallback(void *data, wl_callback *callback, uint32_t time)
{
    Q_UNUSED(time)
    auto s = reinterpret_cast<Surface::Private*>(data);
    if (callback) {
        wl_callback_destroy(callback);
    }
    s->handleFrameCallback();
}

void Surface::Private::handleFrameCallback()
{
    frameCallbackInstalled = false;
    emit q->frameRendered();
}

const struct wl_callback_listener Surface::Private::s_listener = {
        frameCallback
};

void Surface::Private::setupFrameCallback()
{
    Q_ASSERT(!frameCallbackInstalled);
    wl_callback *callback = wl_surface_frame(surface);
    wl_callback_add_listener(callback, &s_listener, this);
    frameCallbackInstalled = true;
}

void Surface::setupFrameCallback()
{
    Q_ASSERT(isValid());
    d->setupFrameCallback();
}

void Surface::commit(Surface::CommitFlag flag)
{
    Q_ASSERT(isValid());
    if (flag == CommitFlag::FrameCallback) {
        setupFrameCallback();
    }
    wl_surface_commit(d->surface);
}

void Surface::damage(const QRegion &region)
{
    for (const QRect &r : region.rects()) {
        damage(r);
    }
}

void Surface::damage(const QRect &rect)
{
    Q_ASSERT(isValid());
    wl_surface_damage(d->surface, rect.x(), rect.y(), rect.width(), rect.height());
}

void Surface::attachBuffer(wl_buffer *buffer, const QPoint &offset)
{
    Q_ASSERT(isValid());
    wl_surface_attach(d->surface, buffer, offset.x(), offset.y());
}

void Surface::attachBuffer(Buffer *buffer, const QPoint &offset)
{
    attachBuffer(buffer->buffer(), offset);
}

void Surface::attachBuffer(Buffer::Ptr buffer, const QPoint &offset)
{
    attachBuffer(buffer.toStrongRef().data(), offset);
}

void Surface::setSize(const QSize &size)
{
    if (d->size == size) {
        return;
    }
    d->size = size;
    emit sizeChanged(d->size);
}

Surface *Surface::get(wl_surface *native)
{
    auto it = std::find_if(Private::s_surfaces.constBegin(), Private::s_surfaces.constEnd(),
        [native](Surface *s) {
            return s->d->surface == native;
        }
    );
    if (it != Private::s_surfaces.constEnd()) {
        return *(it);
    }
    return nullptr;
}

const QList< Surface* > &Surface::all()
{
    return Private::s_surfaces;
}

bool Surface::isValid() const
{
    return d->surface != nullptr;
}

QSize Surface::size() const
{
    return d->size;
}

Surface::operator wl_surface*()
{
    return d->surface;
}

Surface::operator wl_surface*() const
{
    return d->surface;
}

}
}
