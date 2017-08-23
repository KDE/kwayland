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
#include "xdgforeign_v1.h"
#include "xdgforeign_p.h"
#include "event_queue.h"
#include "wayland_pointer_p.h"

#include <wayland-xdg-foreign-unstable-v1-client-protocol.h>

#include <QDebug>

namespace KWayland
{
namespace Client
{

class Q_DECL_HIDDEN XdgExporterUnstableV1::Private : public XdgExporterUnstable::Private
{
public:
    Private();

    XdgExportedUnstable *exportTopLevelV1(Surface *surface, QObject *parent) override;
    void setupV1(zxdg_exporter_v1 *arg) override;
    zxdg_exporter_v1 *exporterV1() override;

    void release() override;
    void destroy() override;
    bool isValid() override;

    WaylandPointer<zxdg_exporter_v1, zxdg_exporter_v1_destroy> exporter;
};

XdgExporterUnstableV1::Private::Private()
    : XdgExporterUnstable::Private()
{}

zxdg_exporter_v1 *XdgExporterUnstableV1::Private::exporterV1()
{
    return exporter;
}

void XdgExporterUnstableV1::Private::release()
{
    exporter.release();
}

void XdgExporterUnstableV1::Private::destroy()
{
    exporter.destroy();
}

bool XdgExporterUnstableV1::Private::isValid()
{
    return exporter.isValid();
}

XdgExportedUnstable *XdgExporterUnstableV1::Private::exportTopLevelV1(Surface *surface, QObject *parent)
{
    Q_ASSERT(isValid());
    auto p = new XdgExportedUnstableV1(parent);
    auto w = zxdg_exporter_v1_export_toplevel(exporter, *surface);
    if (queue) {
        queue->addProxy(w);
    }
    p->setup(w);
    return p;
}


XdgExporterUnstableV1::XdgExporterUnstableV1(QObject *parent)
    : XdgExporterUnstable(new Private, parent)
{
}

void XdgExporterUnstableV1::Private::setupV1(zxdg_exporter_v1 *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!exporter);
    exporter.setup(arg);
}

XdgExporterUnstableV1::~XdgExporterUnstableV1()
{
}

class Q_DECL_HIDDEN XdgImporterUnstableV1::Private : public XdgImporterUnstable::Private
{
public:
    Private();

    XdgImportedUnstable *importTopLevelV1(const QString & handle, QObject *parent) override;
    void setupV1(zxdg_importer_v1 *arg) override;
    zxdg_importer_v1 *importerV1() override;

    void release() override;
    void destroy() override;
    bool isValid() override;

    WaylandPointer<zxdg_importer_v1, zxdg_importer_v1_destroy> importer;
    EventQueue *queue = nullptr;
};

XdgImporterUnstableV1::Private::Private()
    : XdgImporterUnstable::Private()
{}

zxdg_importer_v1 *XdgImporterUnstableV1::Private::importerV1()
{
    return importer;
}

void XdgImporterUnstableV1::Private::release()
{
    importer.release();
}

void XdgImporterUnstableV1::Private::destroy()
{
    importer.destroy();
}

bool XdgImporterUnstableV1::Private::isValid()
{
    return importer.isValid();
}

XdgImportedUnstable *XdgImporterUnstableV1::Private::importTopLevelV1(const QString & handle, QObject *parent)
{
    Q_ASSERT(isValid());
    auto p = new XdgImportedUnstableV1(parent);
    auto w = zxdg_importer_v1_import_toplevel(importer, handle.toUtf8());
    if (queue) {
        queue->addProxy(w);
    }
    p->setup(w);
    return p;
}


XdgImporterUnstableV1::XdgImporterUnstableV1(QObject *parent)
    : XdgImporterUnstable(new Private, parent)
{
}

void XdgImporterUnstableV1::Private::setupV1(zxdg_importer_v1 *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!importer);
    importer.setup(arg);
}

XdgImporterUnstableV1::~XdgImporterUnstableV1()
{
}


class Q_DECL_HIDDEN XdgExportedUnstableV1::Private : public XdgExportedUnstable::Private
{
public:
    Private(XdgExportedUnstableV1 *q);

    void setupV1(zxdg_exported_v1 *arg) override;
    zxdg_exported_v1 *exportedV1() override;

    void release() override;
    void destroy() override;
    bool isValid() override;

    WaylandPointer<zxdg_exported_v1, zxdg_exported_v1_destroy> exported;

private:
    static void handleCallback(void *data, zxdg_exported_v1 *zxdg_exported_v1, const char * handle);

    static const zxdg_exported_v1_listener s_listener;
};

zxdg_exported_v1 *XdgExportedUnstableV1::Private::exportedV1()
{
    return exported;
}

void XdgExportedUnstableV1::Private::release()
{
    exported.release();
}

void XdgExportedUnstableV1::Private::destroy()
{
    exported.destroy();
}

bool XdgExportedUnstableV1::Private::isValid()
{
    return exported.isValid();
}


const zxdg_exported_v1_listener XdgExportedUnstableV1::Private::s_listener = {
    handleCallback
};

void XdgExportedUnstableV1::Private::handleCallback(void *data, zxdg_exported_v1 *zxdg_exported_v1, const char * handle)
{
    auto p = reinterpret_cast<XdgExportedUnstableV1::Private*>(data);
    Q_ASSERT(p->exported == zxdg_exported_v1);

    p->handle = handle;
    emit p->q->done();
}

XdgExportedUnstableV1::XdgExportedUnstableV1(QObject *parent)
    : XdgExportedUnstable(new Private(this), parent)
{
}

void XdgExportedUnstableV1::Private::setupV1(zxdg_exported_v1 *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!exported);
    exported.setup(arg);
    zxdg_exported_v1_add_listener(exported, &s_listener, this);
}

XdgExportedUnstableV1::Private::Private(XdgExportedUnstableV1 *q)
    : XdgExportedUnstable::Private::Private(q)
{
}

XdgExportedUnstableV1::~XdgExportedUnstableV1()
{
}

class Q_DECL_HIDDEN XdgImportedUnstableV1::Private : public XdgImportedUnstable::Private
{
public:
    Private(XdgImportedUnstableV1 *q);

    void setupV1(zxdg_imported_v1 *arg) override;
    zxdg_imported_v1 *importedV1() override;

    void setParentOf(Surface *surface) override;
    void release() override;
    void destroy() override;
    bool isValid() override;

    WaylandPointer<zxdg_imported_v1, zxdg_imported_v1_destroy> imported;

private:
    static void destroyedCallback(void *data, zxdg_imported_v1 *zxdg_imported_v1);

    static const zxdg_imported_v1_listener s_listener;
};

XdgImportedUnstableV1::Private::Private(XdgImportedUnstableV1 *q)
    : XdgImportedUnstable::Private::Private(q)
{
}

zxdg_imported_v1 *XdgImportedUnstableV1::Private::importedV1()
{
    return imported;
}

void XdgImportedUnstableV1::Private::release()
{
    imported.release();
}

void XdgImportedUnstableV1::Private::destroy()
{
    imported.destroy();
}

bool XdgImportedUnstableV1::Private::isValid()
{
    return imported.isValid();
}

void XdgImportedUnstableV1::Private::setParentOf(Surface *surface)
{
    Q_ASSERT(isValid());
    zxdg_imported_v1_set_parent_of(imported, *surface);
}

const zxdg_imported_v1_listener XdgImportedUnstableV1::Private::s_listener = {
    destroyedCallback
};

void XdgImportedUnstableV1::Private::destroyedCallback(void *data, zxdg_imported_v1 *zxdg_imported_v1)
{
    auto p = reinterpret_cast<XdgImportedUnstableV1::Private*>(data);
    Q_ASSERT(p->imported == zxdg_imported_v1);

    p->q->release();
    emit p->q->importedDestroyed();
}



XdgImportedUnstableV1::XdgImportedUnstableV1(QObject *parent)
    : XdgImportedUnstable(new Private(this), parent)
{
}

void XdgImportedUnstableV1::Private::setupV1(zxdg_imported_v1 *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!imported);
    imported.setup(arg);
    zxdg_imported_v1_add_listener(imported, &s_listener, this);
}

XdgImportedUnstableV1::~XdgImportedUnstableV1()
{
}


}
}

