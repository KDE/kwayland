/****************************************************************************
Copyright 2018  Marco Martin <notmart@gmail.com>

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
#include "plasmavirtualdesktop_interface.h"
#include "display.h"
#include "global_p.h"
#include "resource_p.h"

#include <wayland-server.h>
#include <wayland-org_kde_plasma_virtual_desktop-server-protocol.h>

namespace KWayland
{
namespace Server
{

class PlasmaVirtualDesktopManagementInterface::Private : public Global::Private
{
public:
    Private(PlasmaVirtualDesktopManagementInterface *q, Display *d);

private:
    void bind(wl_client *client, uint32_t version, uint32_t id) override;

    static void unbind(wl_resource *resource);
    static Private *cast(wl_resource *r) {
        return reinterpret_cast<Private*>(wl_resource_get_user_data(r));
    }

    static void getVirtualDesktopCallback(wl_client *client, wl_resource *resource, uint32_t id, uint32_t serial);
    static void releaseCallback(wl_client *client, wl_resource *resource);

    PlasmaVirtualDesktopManagementInterface *q;
    static const struct org_kde_plasma_virtual_desktop_management_interface s_interface;
    static const quint32 s_version;
};

const quint32 PlasmaVirtualDesktopManagementInterface::Private::s_version = 1;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct org_kde_plasma_virtual_desktop_management_interface PlasmaVirtualDesktopManagementInterface::Private::s_interface = {
    getVirtualDesktopCallback,
    releaseCallback
};
#endif

void PlasmaVirtualDesktopManagementInterface::Private::getVirtualDesktopCallback(wl_client *client, wl_resource *resource, uint32_t id, uint32_t serial)
{
    // TODO: implement
}

void PlasmaVirtualDesktopManagementInterface::Private::releaseCallback(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

PlasmaVirtualDesktopManagementInterface::Private::Private(PlasmaVirtualDesktopManagementInterface *q, Display *d)
    : Global::Private(d, &org_kde_plasma_virtual_desktop_management_interface, s_version)
    , q(q)
{
}

void PlasmaVirtualDesktopManagementInterface::Private::bind(wl_client *client, uint32_t version, uint32_t id)
{
    auto c = display->getConnection(client);
    wl_resource *resource = c->createResource(&org_kde_plasma_virtual_desktop_management_interface, qMin(version, s_version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &s_interface, this, unbind);
    // TODO: should we track?
}

void PlasmaVirtualDesktopManagementInterface::Private::unbind(wl_resource *resource)
{
    Q_UNUSED(resource)
    // TODO: implement?
}

PlasmaVirtualDesktopManagementInterface::PlasmaVirtualDesktopManagementInterface(Display *display, QObject *parent)
    : Global(new Private(this, display), parent)
{
}

PlasmaVirtualDesktopManagementInterface::~PlasmaVirtualDesktopManagementInterface()
{}




class PlasmaVirtualDesktopInterface::Private : public Resource::Private
{
public:
    Private(PlasmaVirtualDesktopInterface *q, PlasmaVirtualDesktopManagementInterface *c, wl_resource *parentResource);
    ~Private();

private:
    static void activateCallback(wl_client *client, wl_resource *resource);

    PlasmaVirtualDesktopInterface *q_func() {
        return reinterpret_cast<PlasmaVirtualDesktopInterface *>(q);
    }

    static const struct org_kde_plasma_virtual_desktop_interface s_interface;
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct org_kde_plasma_virtual_desktop_interface PlasmaVirtualDesktopInterface::Private::s_interface = {
    resourceDestroyedCallback,
    activateCallback
};
#endif

void PlasmaVirtualDesktopInterface::Private::activateCallback(wl_client *client, wl_resource *resource)
{
    // TODO: implement
}

PlasmaVirtualDesktopInterface::Private::Private(PlasmaVirtualDesktopInterface *q, PlasmaVirtualDesktopManagementInterface *c, wl_resource *parentResource)
    : Resource::Private(q, c, parentResource, &org_kde_plasma_virtual_desktop_interface, &s_interface)
{
}

PlasmaVirtualDesktopInterface::Private::~Private()
{
    if (resource) {
        wl_resource_destroy(resource);
        resource = nullptr;
    }
}

PlasmaVirtualDesktopInterface::PlasmaVirtualDesktopInterface(PlasmaVirtualDesktopManagementInterface *parent, wl_resource *parentResource)
    : Resource(new Private(this, parent, parentResource))
{
}

PlasmaVirtualDesktopInterface::~PlasmaVirtualDesktopInterface()
{}

}
}

