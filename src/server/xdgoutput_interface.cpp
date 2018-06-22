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
#include "xdgoutput_interface.h"
#include "display.h"
#include "global_p.h"
#include "resource_p.h"
#include "output_interface.h"

#include <wayland-xdg-output-server-protocol.h>

namespace KWayland
{
namespace Server
{

class XdgOutputManagerInterface::Private : public Global::Private
{
public:
    Private(XdgOutputManagerInterface *q, Display *d);
    QHash<OutputInterface*, XdgOutputInterface*> outputs;

private:
    void bind(wl_client *client, uint32_t version, uint32_t id) override;

    static void unbind(wl_resource *resource);
    static Private *cast(wl_resource *r) {
        return reinterpret_cast<Private*>(wl_resource_get_user_data(r));
    }

    static void destroyCallback(wl_client *client, wl_resource *resource);
    static void getXdgOutputCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource * output);

    XdgOutputManagerInterface *q;
    static const struct zxdg_output_manager_v1_interface s_interface;
    static const quint32 s_version;
};

const quint32 XdgOutputManagerInterface::Private::s_version = 1;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zxdg_output_manager_v1_interface XdgOutputManagerInterface::Private::s_interface = {
    destroyCallback,
    getXdgOutputCallback
};
#endif

class XdgOutputV1Interface: public Resource
{
public:
    XdgOutputV1Interface(XdgOutputManagerInterface *parent, wl_resource *parentResource);
    ~XdgOutputV1Interface();
    void setLogicalSize(const QSize &size);
    void setLogicalPosition(const QPoint &pos);
    void done();
private:
    class Private;
};

class XdgOutputInterface::Private
{
public:
    void resourceConnected(XdgOutputV1Interface *resource);
    void resourceDisconnected(XdgOutputV1Interface *resource);
    QPoint pos;
    QSize size;
    bool doneOnce = false;
    QList<XdgOutputV1Interface*> resources;
};


XdgOutputManagerInterface::XdgOutputManagerInterface(Display *display, QObject *parent)
    : Global(new XdgOutputManagerInterface::Private(this, display), parent)
{
}

XdgOutputManagerInterface::~XdgOutputManagerInterface()
{}

XdgOutputInterface* XdgOutputManagerInterface::createXdgOutput(OutputInterface *output, QObject *parent)
{
    Q_D();
    if (!d->outputs.contains(output)) {
        auto xdgOutput = new XdgOutputInterface(parent);
        d->outputs[output] = xdgOutput;
        //as XdgOutput lifespan is managed by user, delete our mapping when either
        //it or the relevant Output gets deleted
        connect(output, &QObject::destroyed, this, [this, output]() {
            Q_D();
            d->outputs.remove(output);
        });
        connect(xdgOutput, &QObject::destroyed, this, [this, output]() {
            Q_D();
            d->outputs.remove(output);
        });

    }
    return d->outputs[output];
}

XdgOutputManagerInterface::Private* XdgOutputManagerInterface::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}

void XdgOutputManagerInterface::Private::destroyCallback(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

void XdgOutputManagerInterface::Private::getXdgOutputCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource * outputResource)
{
    auto d = cast(resource);
    auto output = OutputInterface::get(outputResource);
    if (!output) { // output client is requesting XdgOutput for an Output that doesn't exist
        return;
    }
    if (!d->outputs.contains(output)) {
        return; //server hasn't created an XdgOutput for this output yet, give the client nothing
    }
    auto iface = new XdgOutputV1Interface(d->q, resource);
    iface->create(d->display->getConnection(client), wl_resource_get_version(resource), id);
    if (!iface->resource()) {
        wl_resource_post_no_memory(resource);
        delete iface;
        return;
    }

    auto xdgOutput = d->outputs[output];
    xdgOutput->d->resourceConnected(iface);
    connect(iface, &XdgOutputV1Interface::unbound, xdgOutput, [xdgOutput, iface]() {
        xdgOutput->d->resourceDisconnected(iface);
    });
}

XdgOutputManagerInterface::Private::Private(XdgOutputManagerInterface *q, Display *d)
    : Global::Private(d, &zxdg_output_manager_v1_interface, s_version)
    , q(q)
{
}

void XdgOutputManagerInterface::Private::bind(wl_client *client, uint32_t version, uint32_t id)
{
    auto c = display->getConnection(client);
    wl_resource *resource = c->createResource(&zxdg_output_manager_v1_interface, qMin(version, s_version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &s_interface, this, unbind);
}

void XdgOutputManagerInterface::Private::unbind(wl_resource *resource)
{
    Q_UNUSED(resource)
}

XdgOutputInterface::XdgOutputInterface(QObject *parent):
    QObject(parent),
    d(new XdgOutputInterface::Private)
{
}

XdgOutputInterface::~XdgOutputInterface()
{}

void XdgOutputInterface::setLogicalSize(const QSize &size)
{
    if (size == d->size) {
        return;
    }
    d->size = size;
    for(auto resource: d->resources) {
        resource->setLogicalSize(size);
    }
}

QSize XdgOutputInterface::logicalSize() const
{
    return d->size;
}

void XdgOutputInterface::setLogicalPosition(const QPoint &pos)
{
    if (pos == d->pos) {
        return;
    }
    d->pos = pos;
    for(auto resource: d->resources) {
        resource->setLogicalPosition(pos);
    }
}

QPoint XdgOutputInterface::logicalPosition() const
{
    return d->pos;
}

void XdgOutputInterface::done()
{
    d->doneOnce = true;
    for(auto resource: d->resources) {
        resource->done();
    }
}

void XdgOutputInterface::Private::resourceConnected(XdgOutputV1Interface *resource)
{
    resource->setLogicalPosition(pos);
    resource->setLogicalSize(size);
    if (doneOnce) {
        resource->done();
    }
    resources << resource;
}

void XdgOutputInterface::Private::resourceDisconnected(XdgOutputV1Interface *resource)
{
    resources.removeOne(resource);
}


class XdgOutputV1Interface::Private : public Resource::Private
{
public:
    Private(XdgOutputV1Interface *q, XdgOutputManagerInterface *c, wl_resource *parentResource);
    ~Private();

private:

    XdgOutputV1Interface *q_func() {
        return reinterpret_cast<XdgOutputV1Interface *>(q);
    }

    static const struct zxdg_output_v1_interface s_interface;
};

XdgOutputV1Interface::XdgOutputV1Interface(XdgOutputManagerInterface *parent, wl_resource *parentResource)
    :Resource(new XdgOutputV1Interface::Private(this, parent, parentResource))
{}

XdgOutputV1Interface::~XdgOutputV1Interface()
{}

void XdgOutputV1Interface::setLogicalSize(const QSize &size)
{
    if (!d->resource) {
        return;
    }
    zxdg_output_v1_send_logical_size(d->resource, size.width(), size.height());
}

void XdgOutputV1Interface::setLogicalPosition(const QPoint &pos)
{
    if (!d->resource) {
        return;
    }
    zxdg_output_v1_send_logical_position(d->resource, pos.x(), pos.y());
}

void XdgOutputV1Interface::done()
{
    if (!d->resource) {
        return;
    }
    zxdg_output_v1_send_done(d->resource);
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zxdg_output_v1_interface XdgOutputV1Interface::Private::s_interface = {
    resourceDestroyedCallback
};
#endif

XdgOutputV1Interface::Private::Private(XdgOutputV1Interface *q, XdgOutputManagerInterface *c, wl_resource *parentResource)
    : Resource::Private(q, c, parentResource, &zxdg_output_v1_interface, &s_interface)
{
}

XdgOutputV1Interface::Private::~Private()
{
    if (resource) {
        wl_resource_destroy(resource);
        resource = nullptr;
    }
}

}
}

