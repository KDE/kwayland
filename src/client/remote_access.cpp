/****************************************************************************
Copyright 2016  Oleg Chernovskiy <kanedias@xaker.ru>

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
#include "remote_access.h"
#include "event_queue.h"
#include "wayland_pointer_p.h"
#include "logging_p.h"
// Wayland
#include <wayland-remote-access-client-protocol.h>

namespace KWayland
{
namespace Client
{

class RemoteAccessManager::Private
{
public:
    explicit Private(RemoteAccessManager *ram);
    void setup(org_kde_kwin_remote_access_manager *k);

    WaylandPointer<org_kde_kwin_remote_access_manager, org_kde_kwin_remote_access_manager_release> ram;
    EventQueue *queue = nullptr;
private:
    static const struct org_kde_kwin_remote_access_manager_listener s_listener;
    static void bufferReadyCallback(void *data, org_kde_kwin_remote_access_manager *interface, qint32 buffer_id, wl_output *output);

    RemoteAccessManager *q;
};

RemoteAccessManager::Private::Private(RemoteAccessManager *q)
    : q(q)
{
}

const org_kde_kwin_remote_access_manager_listener RemoteAccessManager::Private::s_listener = {
    bufferReadyCallback
};

void RemoteAccessManager::Private::bufferReadyCallback(void *data, org_kde_kwin_remote_access_manager *interface, qint32 buffer_id, wl_output *output)
{
    auto ramp = reinterpret_cast<RemoteAccessManager::Private*>(data);
    Q_ASSERT(ramp->ram == interface);

    // handle it fully internally, get the buffer immediately
    auto requested = org_kde_kwin_remote_access_manager_get_buffer(ramp->ram, buffer_id);
    auto rbuf = new RemoteBuffer(ramp->q);
    rbuf->setup(requested);
    qCDebug(KWAYLAND_CLIENT) << "Got buffer, server fd:" << buffer_id;

    emit ramp->q->bufferReady(output, rbuf);
}

void RemoteAccessManager::Private::setup(org_kde_kwin_remote_access_manager *k)
{
    Q_ASSERT(k);
    Q_ASSERT(!ram);
    ram.setup(k);
    org_kde_kwin_remote_access_manager_add_listener(k, &s_listener, this);
}

RemoteAccessManager::RemoteAccessManager(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

RemoteAccessManager::~RemoteAccessManager()
{
    release();
}

void RemoteAccessManager::setup(org_kde_kwin_remote_access_manager *ram)
{
    d->setup(ram);
}

void RemoteAccessManager::release()
{
    d->ram.release();
}

void RemoteAccessManager::destroy()
{
    d->ram.destroy();
}

void RemoteAccessManager::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *RemoteAccessManager::eventQueue()
{
    return d->queue;
}

RemoteAccessManager::operator org_kde_kwin_remote_access_manager*() {
    return d->ram;
}

RemoteAccessManager::operator org_kde_kwin_remote_access_manager*() const {
    return d->ram;
}

bool RemoteAccessManager::isValid() const
{
    return d->ram.isValid();
}

class RemoteBuffer::Private
{
public:
    Private(RemoteBuffer *q);
    void setup(org_kde_kwin_remote_buffer *buffer);

    static struct org_kde_kwin_remote_buffer_listener s_listener;
    static void paramsCallback(void *data, org_kde_kwin_remote_buffer *rbuf,
            qint32 fd, quint32 width, quint32 height, quint32 stride, quint32 format);

    WaylandPointer<org_kde_kwin_remote_buffer, org_kde_kwin_remote_buffer_release> remotebuffer;
    RemoteBuffer *q;
    
    qint32 fd = 0;
    quint32 width = 0;
    quint32 height = 0;
    quint32 stride = 0;
    quint32 format = 0;
};

RemoteBuffer::Private::Private(RemoteBuffer *q)
    : q(q)
{
}

void RemoteBuffer::Private::paramsCallback(void *data, org_kde_kwin_remote_buffer *rbuf,
        qint32 fd, quint32 width, quint32 height, quint32 stride, quint32 format)
{
    Q_UNUSED(rbuf)
    Private *p = reinterpret_cast<Private *>(data);
    p->fd = fd;
    p->width = width;
    p->height = height;
    p->stride = stride;
    p->format = format;
    emit p->q->parametersObtained();
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
org_kde_kwin_remote_buffer_listener RemoteBuffer::Private::s_listener = {
    paramsCallback
};
#endif

void RemoteBuffer::Private::setup(org_kde_kwin_remote_buffer *rbuffer)
{
    remotebuffer.setup(rbuffer);
    org_kde_kwin_remote_buffer_add_listener(rbuffer, &s_listener, this);
}

RemoteBuffer::RemoteBuffer(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

RemoteBuffer::~RemoteBuffer()
{
    release();
    qCDebug(KWAYLAND_CLIENT) << "Buffer released";
}

void RemoteBuffer::setup(org_kde_kwin_remote_buffer *remotebuffer)
{
    Q_ASSERT(remotebuffer);
    Q_ASSERT(!d->remotebuffer);
    d->setup(remotebuffer);
}

void RemoteBuffer::release()
{
    d->remotebuffer.release();
}

void RemoteBuffer::destroy()
{
    d->remotebuffer.destroy();
}

RemoteBuffer::operator org_kde_kwin_remote_buffer*() {
    return d->remotebuffer;
}

RemoteBuffer::operator org_kde_kwin_remote_buffer*() const {
    return d->remotebuffer;
}

bool RemoteBuffer::isValid() const
{
    return d->remotebuffer.isValid();
}

qint32 RemoteBuffer::fd() const
{
    return d->fd;
}

quint32 RemoteBuffer::width() const
{
    return d->width;
}

quint32 RemoteBuffer::height() const
{
    return d->height;
}

quint32 RemoteBuffer::stride() const
{
    return d->stride;
}

quint32 RemoteBuffer::format() const
{
    return d->format;
}


}
}
