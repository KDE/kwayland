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
#include "xdgforeign.h"
#include "xdgforeign_p.h"
#include "event_queue.h"
#include "wayland_pointer_p.h"

#include <wayland-xdg-foreign-unstable-v1-client-protocol.h>

#include <QDebug>

namespace KWayland
{
namespace Client
{

XdgExporterUnstable::Private::Private()
{
}

XdgExporterUnstable::Private::~Private()
{
}

XdgExporterUnstable::XdgExporterUnstable(Private *p, QObject *parent)
    : QObject(parent)
    , d(p)
{
}

XdgExporterUnstable::~XdgExporterUnstable()
{
    release();
}

void XdgExporterUnstable::setup(zxdg_exporter_v1 *exporter)
{
    d->setupV1(exporter);
}

void XdgExporterUnstable::release()
{
    d->release();
}

void XdgExporterUnstable::destroy()
{
    d->destroy();
}

XdgExporterUnstable::operator zxdg_exporter_v1*() {
    return d->exporterV1();
}

XdgExporterUnstable::operator zxdg_exporter_v1*() const {
    return d->exporterV1();
}

bool XdgExporterUnstable::isValid() const
{
    return d->isValid();
}

void XdgExporterUnstable::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *XdgExporterUnstable::eventQueue()
{
    return d->queue;
}

XdgExportedUnstable *XdgExporterUnstable::exportSurface(Surface *surface, QObject *parent)
{
    return d->exportTopLevelV1(surface, parent);
}



XdgImporterUnstable::Private::Private()
{
}

XdgImporterUnstable::Private::~Private()
{
}

XdgImporterUnstable::XdgImporterUnstable(Private *p, QObject *parent)
    : QObject(parent)
    , d(p)
{
}

XdgImporterUnstable::~XdgImporterUnstable()
{
    release();
}

void XdgImporterUnstable::setup(zxdg_importer_v1 *importer)
{
    d->setupV1(importer);
}

void XdgImporterUnstable::release()
{
    d->release();
}

void XdgImporterUnstable::destroy()
{
    d->destroy();
}

XdgImporterUnstable::operator zxdg_importer_v1*() {
    return d->importerV1();
}

XdgImporterUnstable::operator zxdg_importer_v1*() const {
    return d->importerV1();
}

bool XdgImporterUnstable::isValid() const
{
    return d->isValid();
}

void XdgImporterUnstable::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *XdgImporterUnstable::eventQueue()
{
    return d->queue;
}

XdgImportedUnstable *XdgImporterUnstable::import(const QString & handle, QObject *parent)
{
    Q_ASSERT(isValid());
    return d->importTopLevelV1(handle, parent);
}

XdgExportedUnstable::XdgExportedUnstable(Private *p, QObject *parent)
    : QObject(parent)
    , d(p)
{
}

XdgExportedUnstable::Private::Private(XdgExportedUnstable *q)
    : q(q)
{
}

XdgExportedUnstable::Private::~Private()
{
}

XdgExportedUnstable::~XdgExportedUnstable()
{
    release();
}

void XdgExportedUnstable::setup(zxdg_exported_v1 *exported)
{
    d->setupV1(exported);
}

void XdgExportedUnstable::release()
{
    d->release();
}

void XdgExportedUnstable::destroy()
{
    d->destroy();
}

QString XdgExportedUnstable::handle() const
{
    return d->handle;
}

XdgExportedUnstable::operator zxdg_exported_v1*() {
    return d->exportedV1();
}

XdgExportedUnstable::operator zxdg_exported_v1*() const {
    return d->exportedV1();
}

bool XdgExportedUnstable::isValid() const
{
    return d->isValid();
}

XdgImportedUnstable::Private::Private(XdgImportedUnstable *q)
    : q(q)
{
}

XdgImportedUnstable::Private::~Private()
{
}

XdgImportedUnstable::XdgImportedUnstable(Private *p, QObject *parent)
    : QObject(parent)
    , d(p)
{
}

XdgImportedUnstable::~XdgImportedUnstable()
{
    release();
}

void XdgImportedUnstable::setup(zxdg_imported_v1 *imported)
{
    d->setupV1(imported);
}

void XdgImportedUnstable::release()
{
    d->release();
}

void XdgImportedUnstable::destroy()
{
    d->destroy();
}

XdgImportedUnstable::operator zxdg_imported_v1*() {
    return d->importedV1();
}

XdgImportedUnstable::operator zxdg_imported_v1*() const {
    return d->importedV1();
}

bool XdgImportedUnstable::isValid() const
{
    return d->isValid();
}

void XdgImportedUnstable::setParentOf(Surface *surface)
{
    Q_ASSERT(isValid());
    d->setParentOf(surface);
}


}
}

