/********************************************************************
Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
Copyright 2015  Sebastian Kügler <sebas@kde.org>

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
#include "outputconfiguration_interface.h"
#include "outputdevice_interface.h"
//#include "global_p.h"
#include "resource_p.h"
#include "display.h"

#include <wayland-server.h>
#include "wayland-output-management-server-protocol.h"
#include "wayland-org_kde_kwin_outputdevice-server-protocol.h"

#include <QDebug>

namespace KWayland
{
namespace Server
{


class OutputConfigurationInterface::Private : public Resource::Private
{
public:
    Private(OutputConfigurationInterface *q, OutputManagementInterface *c, wl_resource *parentResource);
    ~Private();

    void sendApplied();
    void sendFailed();

    static const quint32 s_version = 1;
    Display *display = nullptr;

private:
    static void enableCallback(wl_client *client, wl_resource *resource,
                               wl_resource * outputdevice, int32_t enable);
    static void modeCallback(wl_client *client, wl_resource *resource,
                             wl_resource * outputdevice, int32_t mode_id);
    static void transformCallback(wl_client *client, wl_resource *resource,
                                  wl_resource * outputdevice, int32_t transform);
    static void positionCallback(wl_client *client, wl_resource *resource,
                                 wl_resource * outputdevice, int32_t x, int32_t y);
    static void scaleCallback(wl_client *client, wl_resource *resource,
                              wl_resource * outputdevice, int32_t scale);
    static void applyCallback(wl_client *client, wl_resource *resource);

    OutputConfigurationInterface *q_func() {
        return reinterpret_cast<OutputConfigurationInterface *>(q);
    }

    static const struct org_kde_kwin_outputconfiguration_interface s_interface;
};

const struct org_kde_kwin_outputconfiguration_interface OutputConfigurationInterface::Private::s_interface = {
    enableCallback,
    modeCallback,
    transformCallback,
    positionCallback,
    scaleCallback,
    applyCallback
};

OutputConfigurationInterface::OutputConfigurationInterface(OutputManagementInterface* parent, wl_resource* parentResource): Resource(new Private(this, parent, parentResource))
{
    qDebug() << "constructing config" << this;
}


Display *OutputConfigurationInterface::display() const
{
    Q_D();
    return d->display;
}

void OutputConfigurationInterface::setDisplay(Display* display)
{
    Q_D();
    d->display = display;
}


OutputConfigurationInterface::~OutputConfigurationInterface()
{
}

void OutputConfigurationInterface::Private::enableCallback(wl_client *client, wl_resource *resource, wl_resource * outputdevice, int32_t enable)
{
    Private *d = cast<Private>(resource);
    qDebug() << "server enable:" << outputdevice << enable << resource;

    OutputDeviceInterface *o = OutputDeviceInterface::get(outputdevice);
    o->setEnabled(enable);
}

void OutputConfigurationInterface::Private::modeCallback(wl_client *client, wl_resource *resource, wl_resource * outputdevice, int32_t mode_id)
{
    // TODO: implement
}

void OutputConfigurationInterface::Private::transformCallback(wl_client *client, wl_resource *resource, wl_resource * outputdevice, int32_t transform)
{
    // TODO: implement
}

void OutputConfigurationInterface::Private::positionCallback(wl_client *client, wl_resource *resource, wl_resource * outputdevice, int32_t x, int32_t y)
{
    // TODO: implement
}

void OutputConfigurationInterface::Private::scaleCallback(wl_client *client, wl_resource *resource, wl_resource * outputdevice, int32_t scale)
{
    // TODO: implement
}

void OutputConfigurationInterface::Private::applyCallback(wl_client *client, wl_resource *resource)
{
    // TODO: implement
}

OutputConfigurationInterface::Private::Private(OutputConfigurationInterface *q, OutputManagementInterface *c, wl_resource *parentResource)
: Resource::Private(q, c, parentResource, &org_kde_kwin_outputconfiguration_interface, &s_interface)
{
}

OutputConfigurationInterface::Private::~Private()
{
    if (resource) {
        wl_resource_destroy(resource);
        resource = nullptr;
    }
}

OutputConfigurationInterface::Private *OutputConfigurationInterface::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}

void OutputConfigurationInterface::setApplied()
{
    Q_D();
    /* ... */
    qDebug() << "set applied";
    d->sendApplied();
    emit applied();
}

void OutputConfigurationInterface::Private::sendApplied()
{
    qDebug() << "Sending";
    foreach (auto r, s_allResources) {
        org_kde_kwin_outputconfiguration_send_applied(r->resource);
        qDebug() << "  Sent!";
    }
}

void OutputConfigurationInterface::setFailed()
{
    Q_D();
    /* ... */

    d->sendFailed();
    emit failed();
}

void OutputConfigurationInterface::Private::sendFailed()
{
    foreach (auto r, s_allResources) {
        org_kde_kwin_outputconfiguration_send_failed(r->resource);
    }
}


}
}
