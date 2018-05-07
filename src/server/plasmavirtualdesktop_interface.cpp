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

#include <QDebug>
#include <QTimer>

#include <wayland-server.h>
#include <wayland-org_kde_plasma_virtual_desktop-server-protocol.h>

namespace KWayland
{
namespace Server
{

class PlasmaVirtualDesktopInterface::Private
{
public:
    Private(PlasmaVirtualDesktopInterface *q, PlasmaVirtualDesktopManagementInterface *c);
    ~Private();
    void createResource(wl_resource *parent, quint32 serial);

    PlasmaVirtualDesktopInterface *q;
    PlasmaVirtualDesktopManagementInterface *vdm;

    QVector<wl_resource*> resources;
    QString id;
    QString name;
    quint32 row = 0;
    quint32 column = 0;
    bool active = false;

private:
    static void unbind(wl_resource *resource);
    static void requestActivateCallback(wl_client *client, wl_resource *resource);

    static Private *cast(wl_resource *resource) {
        return reinterpret_cast<Private*>(wl_resource_get_user_data(resource));
    }

    wl_listener listener;
    static const struct org_kde_plasma_virtual_desktop_interface s_interface;
};


class PlasmaVirtualDesktopManagementInterface::Private : public Global::Private
{
public:
    Private(PlasmaVirtualDesktopManagementInterface *q, Display *d);

    QVector<wl_resource*> resources;
    QMap<QString, PlasmaVirtualDesktopInterface*> desktops;
    quint32 rows = 0;
    quint32 columns = 0;

private:
    void bind(wl_client *client, uint32_t version, uint32_t id) override;

    static void unbind(wl_resource *resource);
    static Private *cast(wl_resource *r) {
        return reinterpret_cast<Private*>(wl_resource_get_user_data(r));
    }

    static void getVirtualDesktopCallback(wl_client *client, wl_resource *resource, uint32_t serial, const char *id);
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

void PlasmaVirtualDesktopManagementInterface::Private::getVirtualDesktopCallback(wl_client *client, wl_resource *resource, uint32_t serial, const char *id)
{
    Q_UNUSED(client)
    auto s = cast(resource);

    auto i = s->desktops.constFind(QString::fromUtf8(id));
    if (i == s->desktops.constEnd()) {
        return;
    }

    (*i)->d->createResource(resource, serial);
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
    resources << resource;

    wl_resource_set_implementation(resource, &s_interface, this, unbind);

    for (auto it = desktops.constBegin(); it != desktops.constEnd(); ++it) {
        org_kde_plasma_virtual_desktop_management_send_desktop_added(resource, (*it)->id().toUtf8().constData());
    }
    org_kde_plasma_virtual_desktop_management_send_layout(resource, rows, columns);
}

void PlasmaVirtualDesktopManagementInterface::Private::unbind(wl_resource *resource)
{
    auto dm = reinterpret_cast<Private*>(wl_resource_get_user_data(resource));
    dm->resources.removeAll(resource);
}

PlasmaVirtualDesktopManagementInterface::PlasmaVirtualDesktopManagementInterface(Display *display, QObject *parent)
    : Global(new Private(this, display), parent)
{
}

PlasmaVirtualDesktopManagementInterface::~PlasmaVirtualDesktopManagementInterface()
{}

PlasmaVirtualDesktopManagementInterface::Private *PlasmaVirtualDesktopManagementInterface::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}

void PlasmaVirtualDesktopManagementInterface::setLayout(quint32 rows, quint32 columns)
{
    Q_D();
    if (d->rows == rows && d->columns == columns) {
        return;
    }

    d->rows = rows;
    d->columns = columns;

    for (auto it = d->resources.constBegin(); it != d->resources.constEnd(); ++it) {
        org_kde_plasma_virtual_desktop_management_send_layout(*it, rows, columns);
    }
}

quint32 PlasmaVirtualDesktopManagementInterface::rows()
{
    Q_D();
    return d->rows;
}

quint32 PlasmaVirtualDesktopManagementInterface::columns()
{
    Q_D();
    return d->columns;
}

PlasmaVirtualDesktopInterface *PlasmaVirtualDesktopManagementInterface::createDesktop(const QString &id)
{
    Q_D();
    
    auto i = d->desktops.constFind(id);
    if (i != d->desktops.constEnd()) {
        return *i;
    }

    PlasmaVirtualDesktopInterface *desktop = new PlasmaVirtualDesktopInterface(this);
    desktop->setId(id);

    //activate the first desktop TODO: to be done here?
    if (d->desktops.isEmpty()) {
        desktop->d->active = true;
    }

    d->desktops[id] = desktop;
    connect(desktop, &QObject::destroyed, this,
        [this, id] {
            Q_D();
            d->desktops.remove(id);
            //TODO: activate another desktop?
        }
    );

    for (auto it = d->resources.constBegin(); it != d->resources.constEnd(); ++it) {
        org_kde_plasma_virtual_desktop_management_send_desktop_added(*it, id.toUtf8().constData());
    }

    return desktop;
}

void PlasmaVirtualDesktopManagementInterface::removeDesktop(const QString &id)
{
    Q_D();
    
    auto deskIt = d->desktops.constFind(id);
    if (deskIt == d->desktops.constEnd()) {
        return;
    }

    for (auto it = (*deskIt)->d->resources.constBegin(); it != (*deskIt)->d->resources.constEnd(); ++it) {
        org_kde_plasma_virtual_desktop_send_removed(*it);
    }

    for (auto it = d->resources.constBegin(); it != d->resources.constEnd(); ++it) {
        org_kde_plasma_virtual_desktop_management_send_desktop_removed(*it, id.toUtf8().constData());
    }

    (*deskIt)->deleteLater();
}

QList <PlasmaVirtualDesktopInterface *> PlasmaVirtualDesktopManagementInterface::desktops() const
{
    Q_D();
    return d->desktops.values();
}

void PlasmaVirtualDesktopManagementInterface::sendDone()
{
    Q_D();
    for (auto it = d->resources.constBegin(); it != d->resources.constEnd(); ++it) {
        org_kde_plasma_virtual_desktop_management_send_done(*it);
    }
}

void PlasmaVirtualDesktopManagementInterface::setActiveDesktop(const QString &id)
{
    Q_D();
    for (auto it = d->desktops.constBegin(); it != d->desktops.constEnd(); ++it) {
        auto desktop = *it;
        if (desktop->id() == id) {
            desktop->d->active = true;
            for (auto it = desktop->d->resources.constBegin(); it != desktop->d->resources.constEnd(); ++it) {
                org_kde_plasma_virtual_desktop_send_activated(*it);
            }
        } else {
            if (desktop->d->active) {
                desktop->d->active = false;
                for (auto it = desktop->d->resources.constBegin(); it != desktop->d->resources.constEnd(); ++it) {
                    org_kde_plasma_virtual_desktop_send_deactivated(*it);
                }
            }
        }
    }
}



//// PlasmaVirtualDesktopInterface

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct org_kde_plasma_virtual_desktop_interface PlasmaVirtualDesktopInterface::Private::s_interface = {
    requestActivateCallback
};
#endif

void PlasmaVirtualDesktopInterface::Private::requestActivateCallback(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    auto s = cast(resource);
    emit s->q->activateRequested();
}

PlasmaVirtualDesktopInterface::Private::Private(PlasmaVirtualDesktopInterface *q, PlasmaVirtualDesktopManagementInterface *c)
    : q(q),
      vdm(c)
{
}

PlasmaVirtualDesktopInterface::Private::~Private()
{
   // need to copy, as destroy goes through the destroy listener and modifies the list as we iterate
    const auto c = resources;
    for (const auto &r : c) {
        auto client = wl_resource_get_client(r);
        org_kde_plasma_virtual_desktop_send_removed(r);
        wl_resource_destroy(r);
        wl_client_flush(client);
    }
}

void PlasmaVirtualDesktopInterface::Private::unbind(wl_resource *resource)
{
    Private *p = reinterpret_cast<Private*>(wl_resource_get_user_data(resource));
    p->resources.removeAll(resource);
    if (p->resources.isEmpty()) {
        p->q->deleteLater();
    }
}

void PlasmaVirtualDesktopInterface::Private::createResource(wl_resource *parent, quint32 serial)
{
    ClientConnection *c = vdm->display()->getConnection(wl_resource_get_client(parent));
    wl_resource *resource = c->createResource(&org_kde_plasma_virtual_desktop_interface, wl_resource_get_version(parent), serial);
    if (!resource) {
        return;
    }
    wl_resource_set_implementation(resource, &s_interface, this, unbind);
    resources << resource;

    org_kde_plasma_virtual_desktop_send_id(resource, id.toUtf8().constData());
    if (!name.isEmpty()) {
        org_kde_plasma_virtual_desktop_send_name(resource, name.toUtf8().constData());
    }
    org_kde_plasma_virtual_desktop_send_layout_position(resource, row, column);

    if (active) {
        org_kde_plasma_virtual_desktop_send_activated(resource);
    }

    c->flush();
}

PlasmaVirtualDesktopInterface::PlasmaVirtualDesktopInterface(PlasmaVirtualDesktopManagementInterface *parent)
    : QObject(parent),
      d(new Private(this, parent))
{
}

PlasmaVirtualDesktopInterface::~PlasmaVirtualDesktopInterface()
{}

void PlasmaVirtualDesktopInterface::setId(const QString &id)
{
    if (d->id == id) {
        return;
    }

    d->id = id;
    for (auto it = d->resources.constBegin(); it != d->resources.constEnd(); ++it) {
        org_kde_plasma_virtual_desktop_send_id(*it, id.toUtf8().constData());
    }
}

QString PlasmaVirtualDesktopInterface::id() const
{
    return d->id;
}

void PlasmaVirtualDesktopInterface::setName(const QString &name)
{
    if (d->name == name) {
        return;
    }

    d->name = name;
    for (auto it = d->resources.constBegin(); it != d->resources.constEnd(); ++it) {
        org_kde_plasma_virtual_desktop_send_name(*it, name.toUtf8().constData());
    }
}

QString PlasmaVirtualDesktopInterface::name() const
{
    return d->name;
}

void PlasmaVirtualDesktopInterface::setLayoutPosition(quint32 row, quint32 column)
{
    if (d->row == row && d->column == column) {
        return;
    }

    d->row = row;
    d->column = column;

    for (auto it = d->resources.constBegin(); it != d->resources.constEnd(); ++it) {
        org_kde_plasma_virtual_desktop_send_layout_position(*it, row, column);
    }
}

quint32 PlasmaVirtualDesktopInterface::row() const
{
    return d->row;
}

quint32 PlasmaVirtualDesktopInterface::column() const
{
    return d->column;
}

bool PlasmaVirtualDesktopInterface::active() const
{
    return d->active;
}

void PlasmaVirtualDesktopInterface::sendDone()
{
    for (auto it = d->resources.constBegin(); it != d->resources.constEnd(); ++it) {
        org_kde_plasma_virtual_desktop_send_done(*it);
    }
}

}
}

