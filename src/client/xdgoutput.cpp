/****************************************************************************
Copyright 2018  David Edmundson <kde@davidedmundson.co.uk>

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
#include "xdgoutput.h"
#include "event_queue.h"
#include "wayland_pointer_p.h"
#include "output.h"

#include <wayland-xdg-output-unstable-v1-client-protocol.h>
#include <wayland-client-protocol.h>

#include <QDebug>

namespace KWayland
{
namespace Client
{

class XdgOutputManager::Private
{
public:
    Private() = default;

    void setup(zxdg_output_manager_v1 *arg);

    WaylandPointer<zxdg_output_manager_v1, zxdg_output_manager_v1_destroy> xdgoutputmanager;
    EventQueue *queue = nullptr;
};

XdgOutputManager::XdgOutputManager(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

void XdgOutputManager::Private::setup(zxdg_output_manager_v1 *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!xdgoutputmanager);
    xdgoutputmanager.setup(arg);
}

XdgOutputManager::~XdgOutputManager()
{
    release();
}

void XdgOutputManager::setup(zxdg_output_manager_v1 *xdgoutputmanager)
{
    d->setup(xdgoutputmanager);
}

void XdgOutputManager::release()
{
    d->xdgoutputmanager.release();
}

void XdgOutputManager::destroy()
{
    d->xdgoutputmanager.destroy();
}

XdgOutputManager::operator zxdg_output_manager_v1*() {
    return d->xdgoutputmanager;
}

XdgOutputManager::operator zxdg_output_manager_v1*() const {
    return d->xdgoutputmanager;
}

bool XdgOutputManager::isValid() const
{
    return d->xdgoutputmanager.isValid();
}

void XdgOutputManager::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *XdgOutputManager::eventQueue()
{
    return d->queue;
}

XdgOutput *XdgOutputManager::getXdgOutput(Output *output, QObject *parent)
{
    Q_ASSERT(isValid());
    auto p = new XdgOutput(parent);
    auto w = zxdg_output_manager_v1_get_xdg_output(d->xdgoutputmanager, *output);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    p->setup(w);
    return p;
}

struct XdgOutputBuffer
{
    QPoint logicalPosition;
    QSize logicalSize;
};

class XdgOutput::Private
{
public:
    Private(XdgOutput *q);

    void setup(zxdg_output_v1 *arg);

    WaylandPointer<zxdg_output_v1, zxdg_output_v1_destroy> xdgoutput;

    XdgOutputBuffer current;
    XdgOutputBuffer pending;

private:
    XdgOutput *q;

private:
    static void logical_positionCallback(void *data, zxdg_output_v1 *zxdg_output_v1, int32_t x, int32_t y);
    static void logical_sizeCallback(void *data, zxdg_output_v1 *zxdg_output_v1, int32_t width, int32_t height);
    static void doneCallback(void *data, zxdg_output_v1 *zxdg_output_v1);

    static const zxdg_output_v1_listener s_listener;
};

const zxdg_output_v1_listener XdgOutput::Private::s_listener = {
    logical_positionCallback,
    logical_sizeCallback,
    doneCallback
};

void XdgOutput::Private::logical_positionCallback(void *data, zxdg_output_v1 *zxdg_output_v1, int32_t x, int32_t y)
{
    auto p = reinterpret_cast<XdgOutput::Private*>(data);
    Q_ASSERT(p->xdgoutput == zxdg_output_v1);
    p->pending.logicalPosition = QPoint(x,y);
}

void XdgOutput::Private::logical_sizeCallback(void *data, zxdg_output_v1 *zxdg_output_v1, int32_t width, int32_t height)
{
    auto p = reinterpret_cast<XdgOutput::Private*>(data);
    Q_ASSERT(p->xdgoutput == zxdg_output_v1);
    p->pending.logicalSize = QSize(width,height);
}

void XdgOutput::Private::doneCallback(void *data, zxdg_output_v1 *zxdg_output_v1)
{
    auto p = reinterpret_cast<XdgOutput::Private*>(data);
    Q_ASSERT(p->xdgoutput == zxdg_output_v1);
    std::swap(p->current, p->pending);

    if (p->current.logicalSize != p->pending.logicalSize ||
        p->current.logicalPosition != p->pending.logicalPosition) {
        emit p->q->changed();
    }
}

XdgOutput::Private::Private(XdgOutput *qptr)
    : q(qptr)
{
}

XdgOutput::XdgOutput(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

void XdgOutput::Private::setup(zxdg_output_v1 *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!xdgoutput);
    xdgoutput.setup(arg);
    zxdg_output_v1_add_listener(xdgoutput, &s_listener, this);
}

XdgOutput::~XdgOutput()
{
    release();
}

void XdgOutput::setup(zxdg_output_v1 *xdgoutput)
{
    d->setup(xdgoutput);
}

void XdgOutput::release()
{
    d->xdgoutput.release();
}

void XdgOutput::destroy()
{
    d->xdgoutput.destroy();
}

QSize XdgOutput::logicalSize() const
{
    return d->current.logicalSize;
}

QPoint XdgOutput::logicalPosition() const
{
    return d->current.logicalPosition;
}

XdgOutput::operator zxdg_output_v1*() {
    return d->xdgoutput;
}

XdgOutput::operator zxdg_output_v1*() const {
    return d->xdgoutput;
}

bool XdgOutput::isValid() const
{
    return d->xdgoutput.isValid();
}


}
}

