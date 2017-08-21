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

#include "xdgforeign_interface.h"
#include "xdgforeign_v1_interface_p.h"
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

class Q_DECL_HIDDEN XdgExporterUnstableV1Interface::Private : public Global::Private
{
public:
    Private(XdgExporterUnstableV1Interface *q, Display *d, XdgForeignUnstableInterface *foreignInterface);

    XdgForeignUnstableInterface *foreignInterface;
    QHash<QString, XdgExportedUnstableV1Interface *> exportedSurfaces;

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
};

const quint32 XdgExporterUnstableV1Interface::Private::s_version = 1;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zxdg_exporter_v1_interface XdgExporterUnstableV1Interface::Private::s_interface = {
    destroyCallback,
    exportCallback
};
#endif

XdgExporterUnstableV1Interface::XdgExporterUnstableV1Interface(Display *display, XdgForeignUnstableInterface *parent)
    : Global(new Private(this, display, parent), parent)
{
}

XdgExporterUnstableV1Interface::~XdgExporterUnstableV1Interface()
{}

XdgExporterUnstableV1Interface::Private *XdgExporterUnstableV1Interface::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}

XdgExportedUnstableV1Interface *XdgExporterUnstableV1Interface::exportedSurface(const QString &handle)
{
    Q_D();

    auto it = d->exportedSurfaces.constFind(handle);
    if (it != d->exportedSurfaces.constEnd()) {
        return it.value();
    }
    return nullptr;
}

void XdgExporterUnstableV1Interface::Private::destroyCallback(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
}

void XdgExporterUnstableV1Interface::Private::exportCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource * surface)
{
    auto s = cast(resource);
    QPointer <XdgExportedUnstableV1Interface> e = new XdgExportedUnstableV1Interface(s->q, surface);

    e->create(s->display->getConnection(client), wl_resource_get_version(resource), id);

    if (!e->resource()) {
        wl_resource_post_no_memory(resource);
        delete e;
        return;
    }

    const QString handle = QUuid::createUuid().toString();

    //a surface not exported anymore
    connect(e.data(), &XdgExportedUnstableV1Interface::unbound,
            s->q, [s, handle]() {
                s->exportedSurfaces.remove(handle);
                emit s->q->surfaceUnexported(handle);
            });

    //if the surface dies before this, this dies too
    connect(SurfaceInterface::get(surface), &Resource::unbound,
            s->q, [s, e, handle]() {
                if (e) {
                    e->deleteLater();
                }
                s->exportedSurfaces.remove(handle);
                emit s->q->surfaceUnexported(handle);
            });

    s->exportedSurfaces[handle] = e;
    zxdg_exported_v1_send_handle(e->resource(), handle.toUtf8().constData());
    emit s->q->surfaceExported(handle, e);
}

XdgExporterUnstableV1Interface::Private::Private(XdgExporterUnstableV1Interface *q, Display *d,XdgForeignUnstableInterface *foreignInterface)
    : Global::Private(d, &zxdg_exporter_v1_interface, s_version)
    , foreignInterface(foreignInterface)
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

class Q_DECL_HIDDEN XdgImporterUnstableV1Interface::Private : public Global::Private
{
public:
    Private(XdgImporterUnstableV1Interface *q, Display *d, XdgForeignUnstableInterface *foreignInterface);

    XdgForeignUnstableInterface *foreignInterface;

    QHash<QString, XdgImportedUnstableV1Interface *> importedSurfaces;

    //child->parent hash
    QHash<SurfaceInterface *, XdgImportedUnstableV1Interface *> parents;
    //parent->child hash
    QHash<XdgImportedUnstableV1Interface *, SurfaceInterface *> children;

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

XdgImporterUnstableV1Interface::XdgImporterUnstableV1Interface(Display *display, XdgForeignUnstableInterface *parent)
    : Global(new Private(this, display, parent), parent)
{
}

XdgImporterUnstableV1Interface::~XdgImporterUnstableV1Interface()
{}

XdgImportedUnstableV1Interface *XdgImporterUnstableV1Interface::importedSurface(const QString &handle)
{
    Q_D();

    auto it = d->importedSurfaces.constFind(handle);
    if (it != d->importedSurfaces.constEnd()) {
        return it.value();
    }
    return nullptr;
}

SurfaceInterface *XdgImporterUnstableV1Interface::transientFor(SurfaceInterface *surface)
{
    Q_D();

    auto it = d->parents.constFind(surface);
    if (it == d->parents.constEnd()) {
        return nullptr;
    }
    return SurfaceInterface::get((*it)->parentResource());
}

XdgImporterUnstableV1Interface::Private *XdgImporterUnstableV1Interface::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}

void XdgImporterUnstableV1Interface::Private::destroyCallback(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
}

void XdgImporterUnstableV1Interface::Private::importCallback(wl_client *client, wl_resource *resource, uint32_t id, const char * handle)
{
    auto s = cast(resource);

    Q_ASSERT(s->foreignInterface);

    XdgExportedUnstableV1Interface *exp = s->foreignInterface->d->exporter->exportedSurface(QString::fromUtf8(handle));
    if (!exp) {
        zxdg_imported_v1_send_destroyed(resource);
        return;
    }

    wl_resource *surface = exp->parentResource();
    if (!surface) {
        zxdg_imported_v1_send_destroyed(resource);
        return;
    }

    QPointer<XdgImportedUnstableV1Interface> imp = new XdgImportedUnstableV1Interface(s->q, surface);
    imp->create(s->display->getConnection(client), wl_resource_get_version(resource), id);

    //surface no longer exported
    connect(exp, &XdgExportedUnstableV1Interface::unbound,
            s->q, [s, imp, handle]() {
                //imp valid when the exported is deleted before the imported
                if (imp) {
                    zxdg_imported_v1_send_destroyed(imp->resource());
                    imp->deleteLater();
                }
                s->importedSurfaces.remove(QString::fromUtf8(handle));
                emit s->q->surfaceUnimported(QString::fromUtf8(handle));
            });

    connect(imp.data(), &XdgImportedUnstableV1Interface::childChanged,
            s->q, [s, imp](SurfaceInterface *child) {
                //remove any previous association
                auto it = s->children.find(imp);
                if (it != s->children.end()) {
                    s->parents.remove(*it);
                    s->children.erase(it);
                }

                s->parents[child] = imp;
                s->children[imp] = child;
                SurfaceInterface *parent = SurfaceInterface::get(imp->parentResource());
                emit s->q->transientChanged(child, parent);

                //child surface destroyed
                connect(child, &Resource::unbound,
                        s->q, [s, child]() {
                            auto it = s->parents.find(child);
                            if (it != s->parents.end()) {
                                s->children.remove(*it);
                                s->parents.erase(it);
                                emit s->q->transientChanged(nullptr, SurfaceInterface::get((*it)->parentResource()));
                            }
                        });
            });

    //surface no longer imported
    connect(imp.data(), &XdgImportedUnstableV1Interface::unbound,
            s->q, [s, handle, imp]() {
                s->importedSurfaces.remove(QString::fromUtf8(handle));
                emit s->q->surfaceUnimported(QString::fromUtf8(handle));

                auto it = s->children.find(imp);
                if (it != s->children.end()) {
                    s->parents.remove(*it);
                    s->children.erase(it);
                    emit s->q->transientChanged(*it, nullptr);
                }
            });

    if (!imp->resource()) {
        wl_resource_post_no_memory(resource);
        delete imp;
        return;
    }

    s->importedSurfaces[QString::fromUtf8(handle)] = imp;
    emit s->q->surfaceImported(QString::fromUtf8(handle), imp);
}

XdgImporterUnstableV1Interface::Private::Private(XdgImporterUnstableV1Interface *q, Display *d, XdgForeignUnstableInterface *foreignInterface)
    : Global::Private(d, &zxdg_importer_v1_interface, s_version)
    , foreignInterface(foreignInterface)
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

class Q_DECL_HIDDEN XdgExportedUnstableV1Interface::Private : public Resource::Private
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

XdgExportedUnstableV1Interface::Private *XdgExportedUnstableV1Interface::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}

XdgExportedUnstableV1Interface::Private::Private(XdgExportedUnstableV1Interface *q, XdgExporterUnstableV1Interface *c, wl_resource *parentResource)
    : Resource::Private(q, c, parentResource, &zxdg_exported_v1_interface, &s_interface)
{
}

XdgExportedUnstableV1Interface::Private::~Private()
{}

class Q_DECL_HIDDEN XdgImportedUnstableV1Interface::Private : public Resource::Private
{
public:
    Private(XdgImportedUnstableV1Interface *q, XdgImporterUnstableV1Interface *c, wl_resource *parentResource);
    ~Private();

    QPointer<SurfaceInterface> parentOf;

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

XdgImportedUnstableV1Interface::XdgImportedUnstableV1Interface(XdgImporterUnstableV1Interface *parent, wl_resource *parentResource)
    : Resource(new Private(this, parent, parentResource))
{
}

XdgImportedUnstableV1Interface::~XdgImportedUnstableV1Interface()
{}

XdgImportedUnstableV1Interface::Private *XdgImportedUnstableV1Interface::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}

SurfaceInterface *XdgImportedUnstableV1Interface::child() const
{
    Q_D();
    return d->parentOf.data(); 
}

void XdgImportedUnstableV1Interface::Private::setParentOfCallback(wl_client *client, wl_resource *resource, wl_resource * surface)
{
    auto s = cast<Private>(resource);
    SurfaceInterface *surf = SurfaceInterface::get(surface);

    if (!surf) {
        return;
    }

    s->parentOf = surf;
    emit s->q_func()->childChanged(surf);
}

XdgImportedUnstableV1Interface::Private::Private(XdgImportedUnstableV1Interface *q, XdgImporterUnstableV1Interface *c, wl_resource *parentResource)
    : Resource::Private(q, c, parentResource, &zxdg_imported_v1_interface, &s_interface)
{
}

XdgImportedUnstableV1Interface::Private::~Private()
{}

}
}

