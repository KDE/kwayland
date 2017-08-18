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
#include "event_queue.h"
#include "wayland_pointer_p.h"

#include <wayland-xdg-foreign-unstable-v1-client-protocol.h>

#include <QDebug>

namespace KWayland
{
namespace Client
{

class Q_DECL_HIDDEN XdgExporterUnstableV1::Private
{
public:
    Private() = default;

    void setup(zxdg_exporter_v1 *arg);

    WaylandPointer<zxdg_exporter_v1, zxdg_exporter_v1_destroy> exporter;
    EventQueue *queue = nullptr;
};

XdgExporterUnstableV1::XdgExporterUnstableV1(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

void XdgExporterUnstableV1::Private::setup(zxdg_exporter_v1 *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!exporter);
    exporter.setup(arg);
}

XdgExporterUnstableV1::~XdgExporterUnstableV1()
{
    release();
}

void XdgExporterUnstableV1::setup(zxdg_exporter_v1 *exporter)
{
    d->setup(exporter);
}

void XdgExporterUnstableV1::release()
{
    d->exporter.release();
}

void XdgExporterUnstableV1::destroy()
{
    d->exporter.destroy();
}

XdgExporterUnstableV1::operator zxdg_exporter_v1*() {
    return d->exporter;
}

XdgExporterUnstableV1::operator zxdg_exporter_v1*() const {
    return d->exporter;
}

bool XdgExporterUnstableV1::isValid() const
{
    return d->exporter.isValid();
}

void XdgExporterUnstableV1::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *XdgExporterUnstableV1::eventQueue()
{
    return d->queue;
}

XdgExportedUnstableV1 *XdgExporterUnstableV1::exportSurface(Surface *surface, QObject *parent)
{
    Q_ASSERT(isValid());
    auto p = new XdgExportedUnstableV1(parent);
    auto w = zxdg_exporter_v1_export_toplevel(d->exporter, *surface);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    p->setup(w);
    return p;
}

class Q_DECL_HIDDEN XdgImporterUnstableV1::Private
{
public:
    Private() = default;

    void setup(zxdg_importer_v1 *arg);

    WaylandPointer<zxdg_importer_v1, zxdg_importer_v1_destroy> importer;
    EventQueue *queue = nullptr;
};

XdgImporterUnstableV1::XdgImporterUnstableV1(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

void XdgImporterUnstableV1::Private::setup(zxdg_importer_v1 *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!importer);
    importer.setup(arg);
}

XdgImporterUnstableV1::~XdgImporterUnstableV1()
{
    release();
}

void XdgImporterUnstableV1::setup(zxdg_importer_v1 *importer)
{
    d->setup(importer);
}

void XdgImporterUnstableV1::release()
{
    d->importer.release();
}

void XdgImporterUnstableV1::destroy()
{
    d->importer.destroy();
}

XdgImporterUnstableV1::operator zxdg_importer_v1*() {
    return d->importer;
}

XdgImporterUnstableV1::operator zxdg_importer_v1*() const {
    return d->importer;
}

bool XdgImporterUnstableV1::isValid() const
{
    return d->importer.isValid();
}

void XdgImporterUnstableV1::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *XdgImporterUnstableV1::eventQueue()
{
    return d->queue;
}

XdgImportedUnstableV1 *XdgImporterUnstableV1::import(const QString & handle, QObject *parent)
{
    Q_ASSERT(isValid());
    auto p = new XdgImportedUnstableV1(parent);
    auto w = zxdg_importer_v1_import_toplevel(d->importer, handle.toUtf8());
    if (d->queue) {
        d->queue->addProxy(p);
    }
    p->setup(w);
    return p;
}

class Q_DECL_HIDDEN XdgExportedUnstableV1::Private
{
public:
    Private(XdgExportedUnstableV1 *q);

    void setup(zxdg_exported_v1 *arg);

    WaylandPointer<zxdg_exported_v1, zxdg_exported_v1_destroy> exported;

    QString handle;

private:
    XdgExportedUnstableV1 *q;

private:
    static void handleCallback(void *data, zxdg_exported_v1 *zxdg_exported_v1, const char * handle);

    static const zxdg_exported_v1_listener s_listener;
};

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
    : QObject(parent)
    , d(new Private(this))
{
}

void XdgExportedUnstableV1::Private::setup(zxdg_exported_v1 *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!exported);
    exported.setup(arg);
    zxdg_exported_v1_add_listener(exported, &s_listener, this);
}

XdgExportedUnstableV1::Private::Private(XdgExportedUnstableV1 *q)
    : q(q)
{
}

XdgExportedUnstableV1::~XdgExportedUnstableV1()
{
    release();
}

void XdgExportedUnstableV1::setup(zxdg_exported_v1 *exported)
{
    d->setup(exported);
}

void XdgExportedUnstableV1::release()
{
    d->exported.release();
}

void XdgExportedUnstableV1::destroy()
{
    d->exported.destroy();
}

QString XdgExportedUnstableV1::handle() const
{
    return d->handle;
}

XdgExportedUnstableV1::operator zxdg_exported_v1*() {
    return d->exported;
}

XdgExportedUnstableV1::operator zxdg_exported_v1*() const {
    return d->exported;
}

bool XdgExportedUnstableV1::isValid() const
{
    return d->exported.isValid();
}

class Q_DECL_HIDDEN XdgImportedUnstableV1::Private
{
public:
    Private(XdgImportedUnstableV1 *q);

    void setup(zxdg_imported_v1 *arg);

    WaylandPointer<zxdg_imported_v1, zxdg_imported_v1_destroy> imported;

private:
    XdgImportedUnstableV1 *q;

private:
    static void destroyedCallback(void *data, zxdg_imported_v1 *zxdg_imported_v1);

    static const zxdg_imported_v1_listener s_listener;
};

const zxdg_imported_v1_listener XdgImportedUnstableV1::Private::s_listener = {
    destroyedCallback
};

void XdgImportedUnstableV1::Private::destroyedCallback(void *data, zxdg_imported_v1 *zxdg_imported_v1)
{
    auto p = reinterpret_cast<XdgImportedUnstableV1::Private*>(data);
    Q_ASSERT(p->imported == zxdg_imported_v1);
    // TODO: implement
    p->q->release();
    emit p->q->importedDestroyed();
}

XdgImportedUnstableV1::Private::Private(XdgImportedUnstableV1 *q)
    : q(q)
{
}

XdgImportedUnstableV1::XdgImportedUnstableV1(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

void XdgImportedUnstableV1::Private::setup(zxdg_imported_v1 *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!imported);
    imported.setup(arg);
    zxdg_imported_v1_add_listener(imported, &s_listener, this);
}

XdgImportedUnstableV1::~XdgImportedUnstableV1()
{
    release();
}

void XdgImportedUnstableV1::setup(zxdg_imported_v1 *imported)
{
    d->setup(imported);
}

void XdgImportedUnstableV1::release()
{
    d->imported.release();
}

void XdgImportedUnstableV1::destroy()
{
    d->imported.destroy();
}

XdgImportedUnstableV1::operator zxdg_imported_v1*() {
    return d->imported;
}

XdgImportedUnstableV1::operator zxdg_imported_v1*() const {
    return d->imported;
}

bool XdgImportedUnstableV1::isValid() const
{
    return d->imported.isValid();
}

void XdgImportedUnstableV1::setParentOf(Surface *surface)
{
    Q_ASSERT(isValid());
    zxdg_imported_v1_set_parent_of(d->imported, *surface);
}


}
}

