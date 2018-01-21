/****************************************************************************
Copyright 2017  Martin Flöser <mgraesslin@kde.org>

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
#include "idleinhibit_interface_p.h"
#include "display.h"
#include "surface_interface_p.h"

namespace KWayland
{
namespace Server
{

class Q_DECL_HIDDEN IdleInhibitManagerUnstableV1Interface::Private : public IdleInhibitManagerInterface::Private
{
public:
    Private(IdleInhibitManagerUnstableV1Interface *q, Display *d);

private:
    void bind(wl_client *client, uint32_t version, uint32_t id) override;

    static void unbind(wl_resource *resource);
    static Private *cast(wl_resource *r) {
        return reinterpret_cast<Private*>(wl_resource_get_user_data(r));
    }

    static void destroyCallback(wl_client *client, wl_resource *resource);
    static void createInhibitorCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource * surface);

    static const struct zwp_idle_inhibit_manager_v1_interface s_interface;
    static const quint32 s_version;
};

const quint32 IdleInhibitManagerUnstableV1Interface::Private::s_version = 1;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zwp_idle_inhibit_manager_v1_interface IdleInhibitManagerUnstableV1Interface::Private::s_interface = {
    destroyCallback,
    createInhibitorCallback
};
#endif

IdleInhibitManagerUnstableV1Interface::Private::Private(IdleInhibitManagerUnstableV1Interface *q, Display *d)
    : IdleInhibitManagerInterface::Private(q, d, &zwp_idle_inhibit_manager_v1_interface, s_version, IdleInhibitManagerInterfaceVersion::UnstableV1)
{
}

void IdleInhibitManagerUnstableV1Interface::Private::destroyCallback(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

void IdleInhibitManagerUnstableV1Interface::Private::createInhibitorCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *surface)
{
    auto s = SurfaceInterface::get(surface);
    if (!s) {
        // send error?
        return;
    }
    auto q = cast(resource);
    auto inhibitor = new IdleInhibitorInterface(q->q, resource);
    inhibitor->d_func()->create(q->display->getConnection(client), version, id);
    s->d_func()->installIdleInhibitor(inhibitor);
}

void IdleInhibitManagerUnstableV1Interface::Private::bind(wl_client *client, uint32_t version, uint32_t id)
{
    auto c = display->getConnection(client);
    wl_resource *resource = c->createResource(&zwp_idle_inhibit_manager_v1_interface, qMin(version, s_version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &s_interface, this, unbind);
    // TODO: should we track?
}

void IdleInhibitManagerUnstableV1Interface::Private::unbind(wl_resource *resource)
{
    Q_UNUSED(resource)
    // TODO: implement?
}

IdleInhibitManagerUnstableV1Interface::IdleInhibitManagerUnstableV1Interface(Display *display, QObject *parent)
    : IdleInhibitManagerInterface(new Private(this, display), parent)
{
}

IdleInhibitManagerUnstableV1Interface::~IdleInhibitManagerUnstableV1Interface() = default;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zwp_idle_inhibitor_v1_interface IdleInhibitorInterface::Private::s_interface = {
    resourceDestroyedCallback
};
#endif

IdleInhibitorInterface::Private::Private(IdleInhibitorInterface *q, IdleInhibitManagerInterface *c, wl_resource *parentResource)
    : Resource::Private(q, c, parentResource, &zwp_idle_inhibitor_v1_interface, &s_interface)
{
}

IdleInhibitorInterface::Private::~Private()
{
    if (resource) {
        wl_resource_destroy(resource);
        resource = nullptr;
    }
}

IdleInhibitorInterface::IdleInhibitorInterface(IdleInhibitManagerInterface *m, wl_resource *parentResource)
    : Resource(new Private(this, m, parentResource))
{
}

IdleInhibitorInterface::~IdleInhibitorInterface() = default;

IdleInhibitorInterface::Private *IdleInhibitorInterface::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}

}
}
