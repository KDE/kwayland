/****************************************************************************
Copyright 2017  Marco Martin <notmart@gmail.com>

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

#include "xdgforeign_v1_interface.h"
#include "display.h"
#include "global_p.h"
#include "resource_p.h"
#include "surface_interface_p.h"

#include "wayland-xdg-foreign-unstable-v1-server-protocol.h"

#include <QUuid>
#include <QDebug>

namespace KWayland
{
namespace Server
{

class XdgExporterUnstableV1Interface::Private : public Global::Private
{
public:
    Private(XdgExporterUnstableV1Interface *q, Display *d);

private:
    void bind(wl_client *client, uint32_t version, uint32_t id) override;

    static void unbind(wl_resource *resource);
    static Private *cast(wl_resource *r) {
        return reinterpret_cast<Private*>(wl_resource_get_user_data(r));
    }

    static void destroyCallback(wl_client *client, wl_resource *resource);
    static void exportCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource * surface);

    XdgExporterUnstableV1Interface *q;
    static const struct zxdg_exporter_v1_interface s_interface;
    static const quint32 s_version;

    QHash<QString, XdgExportedUnstableV1Interface *> exportedSurfaces;
};

const quint32 XdgExporterUnstableV1Interface::Private::s_version = 1;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zxdg_exporter_v1_interface XdgExporterUnstableV1Interface::Private::s_interface = {
    destroyCallback,
    exportCallback
};
#endif

XdgExporterUnstableV1Interface::XdgExporterUnstableV1Interface(Display *display, QObject *parent)
    : Global(new Private(this, display), parent)
{
}

XdgExporterUnstableV1Interface::~XdgExporterUnstableV1Interface()
{}

void XdgExporterUnstableV1Interface::Private::destroyCallback(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

void XdgExporterUnstableV1Interface::Private::exportCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource * surface)
{
    auto s = cast(resource);
    XdgExportedUnstableV1Interface *e = new XdgExportedUnstableV1Interface(s->q, surface);
    e->create(s->display->getConnection(client), wl_resource_get_version(resource), id);

    if (!e->resource()) {
        wl_resource_post_no_memory(resource);
        delete e;
        return;
    }

    const QString handle = QUuid::createUuid().toString();
    s->exportedSurfaces[handle] = e;
    zxdg_exported_v1_send_handle(e->resource(), handle.toUtf8().constData());
}

XdgExporterUnstableV1Interface::Private::Private(XdgExporterUnstableV1Interface *q, Display *d)
    : Global::Private(d, &zxdg_exporter_v1_interface, s_version)
    , q(q)
{
}

void XdgExporterUnstableV1Interface::Private::bind(wl_client *client, uint32_t version, uint32_t id)
{
    auto c = display->getConnection(client);
    wl_resource *resource = c->createResource(&zxdg_exporter_v1_interface, qMin(version, s_version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &s_interface, this, unbind);
    // TODO: should we track?
}

void XdgExporterUnstableV1Interface::Private::unbind(wl_resource *resource)
{
    Q_UNUSED(resource)
    // TODO: implement?
}

class XdgImporterUnstableV1Interface::Private : public Global::Private
{
public:
    Private(XdgImporterUnstableV1Interface *q, Display *d);

private:
    void bind(wl_client *client, uint32_t version, uint32_t id) override;

    static void unbind(wl_resource *resource);
    static Private *cast(wl_resource *r) {
        return reinterpret_cast<Private*>(wl_resource_get_user_data(r));
    }

    static void destroyCallback(wl_client *client, wl_resource *resource);
    static void importCallback(wl_client *client, wl_resource *resource, uint32_t id, const char * handle);

    XdgImporterUnstableV1Interface *q;
    static const struct zxdg_importer_v1_interface s_interface;
    static const quint32 s_version;
};

const quint32 XdgImporterUnstableV1Interface::Private::s_version = 1;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zxdg_importer_v1_interface XdgImporterUnstableV1Interface::Private::s_interface = {
    destroyCallback,
    importCallback
};
#endif

XdgImporterUnstableV1Interface::XdgImporterUnstableV1Interface(Display *display, QObject *parent)
    : Global(new Private(this, display), parent)
{
}

XdgImporterUnstableV1Interface::~XdgImporterUnstableV1Interface()
{}

void XdgImporterUnstableV1Interface::Private::destroyCallback(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}

void XdgImporterUnstableV1Interface::Private::importCallback(wl_client *client, wl_resource *resource, uint32_t id, const char * handle)
{
    // TODO: implement
}

XdgImporterUnstableV1Interface::Private::Private(XdgImporterUnstableV1Interface *q, Display *d)
    : Global::Private(d, &zxdg_importer_v1_interface, s_version)
    , q(q)
{
}

void XdgImporterUnstableV1Interface::Private::bind(wl_client *client, uint32_t version, uint32_t id)
{
    auto c = display->getConnection(client);
    wl_resource *resource = c->createResource(&zxdg_importer_v1_interface, qMin(version, s_version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &s_interface, this, unbind);
    // TODO: should we track?
}

void XdgImporterUnstableV1Interface::Private::unbind(wl_resource *resource)
{
    Q_UNUSED(resource)
    // TODO: implement?
}

class XdgExportedUnstableV1Interface::Private : public Resource::Private
{
public:
    Private(XdgExportedUnstableV1Interface *q, XdgExporterUnstableV1Interface *c, wl_resource *parentResource);
    ~Private();

private:

    XdgExportedUnstableV1Interface *q_func() {
        return reinterpret_cast<XdgExportedUnstableV1Interface *>(q);
    }

    static const struct zxdg_exported_v1_interface s_interface;
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zxdg_exported_v1_interface XdgExportedUnstableV1Interface::Private::s_interface = {
    resourceDestroyedCallback
};
#endif

XdgExportedUnstableV1Interface::XdgExportedUnstableV1Interface(XdgExporterUnstableV1Interface *parent, wl_resource *parentResource)
    : Resource(new Private(this, parent, parentResource))
{
}

XdgExportedUnstableV1Interface::~XdgExportedUnstableV1Interface()
{}

XdgExportedUnstableV1Interface::Private::Private(XdgExportedUnstableV1Interface *q, XdgExporterUnstableV1Interface *c, wl_resource *parentResource)
    : Resource::Private(q, c, parentResource, &zxdg_exported_v1_interface, &s_interface)
{
}

XdgExportedUnstableV1Interface::Private::~Private()
{
    if (resource) {
        wl_resource_destroy(resource);
        resource = nullptr;
    }
}
class XdgImportedUnstableV1Interface::Private : public Resource::Private
{
public:
    Private(XdgImportedUnstableV1Interface *q, XdgImporterUnstableV1Interface *c, wl_resource *parentResource);
    ~Private();

private:
    static void setParentOfCallback(wl_client *client, wl_resource *resource, wl_resource * surface);

    XdgImportedUnstableV1Interface *q_func() {
        return reinterpret_cast<XdgImportedUnstableV1Interface *>(q);
    }

    static const struct zxdg_imported_v1_interface s_interface;
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zxdg_imported_v1_interface XdgImportedUnstableV1Interface::Private::s_interface = {
    resourceDestroyedCallback,
    setParentOfCallback
};
#endif

XdgImportedUnstableV1Interface::~XdgImportedUnstableV1Interface()
{}

void XdgImportedUnstableV1Interface::Private::setParentOfCallback(wl_client *client, wl_resource *resource, wl_resource * surface)
{
    // TODO: implement
}

XdgImportedUnstableV1Interface::Private::Private(XdgImportedUnstableV1Interface *q, XdgImporterUnstableV1Interface *c, wl_resource *parentResource)
    : Resource::Private(q, c, parentResource, &zxdg_imported_v1_interface, &s_interface)
{
}

XdgImportedUnstableV1Interface::Private::~Private()
{
    if (resource) {
        wl_resource_destroy(resource);
        resource = nullptr;
    }
}

}
}

