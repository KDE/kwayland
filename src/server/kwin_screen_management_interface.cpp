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
#include "kwin_screen_management_interface.h"
#include "global_p.h"
#include "display.h"

#include <wayland-server.h>
#include "wayland-org_kde_kwin_screen_management-server-protocol.h"

#include <QDebug>

namespace KWayland
{
namespace Server
{

static const quint32 s_version = 1;

class KWinScreenManagementInterface::Private : public Global::Private
{
public:
    struct ResourceData {
        wl_resource *resource;
        uint32_t version;
    };
    Private(KWinScreenManagementInterface *q, Display *d);

    QString manufacturer = QStringLiteral("org.kde.kwin");
    QString model = QStringLiteral("none");
    QList<ResourceData> resources;

private:
    static void unbind(wl_resource *resource);
    void bind(wl_client *client, uint32_t version, uint32_t id) override;

    KWinScreenManagementInterface *q;
};

KWinScreenManagementInterface::Private::Private(KWinScreenManagementInterface *q, Display *d)
    : Global::Private(d, &org_kde_kwin_screen_management_interface, s_version)
    , q(q)
{
}

KWinScreenManagementInterface::KWinScreenManagementInterface(Display *display, QObject *parent)
    : Global(new Private(this, display), parent)
{
    Q_D();
    qDebug() << "New output interface";
}

KWinScreenManagementInterface::~KWinScreenManagementInterface() = default;


KWinScreenManagementInterface::Private *KWinScreenManagementInterface::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}

void KWinScreenManagementInterface::Private::bind(wl_client *client, uint32_t version, uint32_t id)
{
    qDebug() << "Bound!";
    auto c = display->getConnection(client);
    wl_resource *resource = c->createResource(&org_kde_kwin_screen_management_interface, qMin(version, s_version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_user_data(resource, this);
    wl_resource_set_destructor(resource, unbind);
    ResourceData r;
    r.resource = resource;
    r.version = version;
    resources << r;

    org_kde_kwin_screen_management_send_outputAppeared(resource, "", "DiscoScreen", "HDMI1");
    org_kde_kwin_screen_management_send_outputAppeared(resource, "INVALID_EDID_INFO", "LargeMonitor", "DisplayPort-0");
    org_kde_kwin_screen_management_send_done(resource);

    c->flush();
    qDebug() << "Flushed";
}

void KWinScreenManagementInterface::Private::unbind(wl_resource *resource)
{
    auto o = reinterpret_cast<KWinScreenManagementInterface::Private*>(wl_resource_get_user_data(resource));
    auto it = std::find_if(o->resources.begin(), o->resources.end(), [resource](const ResourceData &r) { return r.resource == resource; });
    if (it != o->resources.end()) {
        o->resources.erase(it);
    }
}

void KWinScreenManagementInterface::getDisabledOutputs()
{
    Q_ASSERT(isValid());
    qDebug() << "getDisabledOutputs!";
//     org_kde_kwin_screen_management_send_outputAppeared(resource, "", "DiscoScreen", "HDMI1");
//     org_kde_kwin_screen_management_send_done(resource);
}


}
}
