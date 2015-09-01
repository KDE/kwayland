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
#include "output_management_interface.h"
#include "global_p.h"
#include "display.h"

#include <wayland-server.h>
#include "wayland-org_kde_kwin_output_management-server-protocol.h"

#include <QDebug>

namespace KWayland
{
namespace Server
{

static const quint32 s_version = 1;

class OutputManagementInterface::Private : public Global::Private
{
public:
    struct ResourceData {
        wl_resource *resource;
        uint32_t version;
    };
    Private(OutputManagementInterface *q, Display *d);

    QList<ResourceData> resources;
    QList<DisabledOutput> disabledOutputs;

private:
    static void unbind(wl_resource *resource);
    void bind(wl_client *client, uint32_t version, uint32_t id) override;
    void sendDone();

    OutputManagementInterface *q;
};

OutputManagementInterface::Private::Private(OutputManagementInterface *q, Display *d)
    : Global::Private(d, &org_kde_kwin_output_management_interface, s_version)
    , q(q)
{

}

OutputManagementInterface::OutputManagementInterface(Display *display, QObject *parent)
    : Global(new Private(this, display), parent)
{
    Q_D();
}

OutputManagementInterface::~OutputManagementInterface() = default;


OutputManagementInterface::Private *OutputManagementInterface::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}

void OutputManagementInterface::Private::bind(wl_client *client, uint32_t version, uint32_t id)
{
    //qDebug() << "Bound!";
    auto c = display->getConnection(client);
    wl_resource *resource = c->createResource(&org_kde_kwin_output_management_interface, qMin(version, s_version), id);
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

    foreach (auto op, disabledOutputs) {
        org_kde_kwin_output_management_send_disabled_output_added(resource,
                                                       qPrintable(op.edid),
                                                       qPrintable(op.name),
                                                       qPrintable(op.connector));
    }

    sendDone();

    c->flush();
    //qDebug() << "Flushed";
}

void OutputManagementInterface::Private::unbind(wl_resource *resource)
{
    auto o = reinterpret_cast<OutputManagementInterface::Private*>(wl_resource_get_user_data(resource));
    auto it = std::find_if(o->resources.begin(), o->resources.end(), [resource](const ResourceData &r) { return r.resource == resource; });
    if (it != o->resources.end()) {
        o->resources.erase(it);
    }
}

void OutputManagementInterface::addDisabledOutput(const OutputManagementInterface::DisabledOutput& output)
{
    Q_D();

    //qDebug() << "New Output! :: " << output.edid << output.name << output.connector;

    d->disabledOutputs << output;

    foreach (auto r, d->resources) {
        wl_resource *resource = r.resource;
        org_kde_kwin_output_management_send_disabled_output_added(resource,
                                                       qPrintable(output.edid),
                                                       qPrintable(output.name),
                                                       qPrintable(output.connector));
    }

}

void OutputManagementInterface::removeDisabledOutput(const QString& name, const QString& connector)
{
    Q_D();
    //qDebug() << "removeDisabledOutput" << name << connector << d->disabledOutputs.count();

    QList<DisabledOutput>::iterator i;
    for (i = d->disabledOutputs.begin(); i != d->disabledOutputs.end(); ++i) {
        if ((*i).name == name) {
            //qDebug() << "Kill me" << name;
            foreach (auto r, d->resources) {
                wl_resource *resource = r.resource;
                org_kde_kwin_output_management_send_disabled_output_removed(resource,
                                                                          qPrintable((*i).name),
                                                                          qPrintable((*i).connector));
            }
            d->disabledOutputs.erase(i);
        }
        i++;
    }
    //qDebug() << "d->DisabledOutputs is now " << d->disabledOutputs.count();
}


void OutputManagementInterface::Private::sendDone()
{
    foreach (auto r, resources) {
        wl_resource *resource = r.resource;
        org_kde_kwin_output_management_send_done(resource);
    }
}


}
}