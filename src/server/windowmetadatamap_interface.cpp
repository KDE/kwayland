/****************************************************************************
Copyright 2016  Sebastian Kügler <sebas@kde.org>

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
#include "windowmetadatamap_interface.h"
#include "wayland-windowmetadatamap-server-protocol.h"
#include "display.h"
#include "global_p.h"
#include "resource_p.h"

namespace KWayland
{
namespace Server
{

class WindowMetadataMapInterface::Private : public Global::Private
{
public:
    Private(WindowMetadataMapInterface *q, Display *d);

private:
    void bind(wl_client *client, uint32_t version, uint32_t id) override;

    static void unbind(wl_resource *resource);
    static Private *cast(wl_resource *r) {
        return reinterpret_cast<Private*>(wl_resource_get_user_data(r));
    }

    static void registerClientCallback(wl_client *client, wl_resource *resource, const char * service_name, wl_resource * surface);
    static void destroyCallback(wl_client *client, wl_resource *resource);

    WindowMetadataMapInterface *q;
    static const struct org_kde_kwin_windowmetadatamap_interface s_interface;
    static const quint32 s_version;
};

const quint32 WindowMetadataMapInterface::Private::s_version = 1;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct org_kde_kwin_windowmetadatamap_interface WindowMetadataMapInterface::Private::s_interface = {
    registerClientCallback,
    destroyCallback
};
#endif

WindowMetadataMapInterface::~WindowMetadataMapInterface()
{

}

void WindowMetadataMapInterface::Private::registerClientCallback(wl_client *client, wl_resource *resource, const char * service_name, wl_resource * surface)
{
    // TODO: implement
}

void WindowMetadataMapInterface::Private::destroyCallback(wl_client *client, wl_resource *resource)
{
    // TODO: implement
}

WindowMetadataMapInterface::Private::Private(WindowMetadataMapInterface *q, Display *d)
    : Global::Private(d, &org_kde_kwin_windowmetadatamap_interface, s_version)
    , q(q)
{
}

void WindowMetadataMapInterface::Private::bind(wl_client *client, uint32_t version, uint32_t id)
{
    auto c = display->getConnection(client);
    wl_resource *resource = c->createResource(&org_kde_kwin_windowmetadatamap_interface, qMin(version, s_version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &s_interface, this, unbind);
    // TODO: should we track?
}

void WindowMetadataMapInterface::Private::unbind(wl_resource *resource)
{
    Q_UNUSED(resource)
    // TODO: implement?
}


}
}

#include "windowmetadatamap_interface.moc"