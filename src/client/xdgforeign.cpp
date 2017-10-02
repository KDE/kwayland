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

#include <wayland-xdg-foreign-unstable-v2-client-protocol.h>

#include <QDebug>

namespace KWayland
{
namespace Client
{

XdgExporter::Private::Private()
{
}

XdgExporter::Private::~Private()
{
}

XdgExporter::XdgExporter(Private *p, QObject *parent)
    : QObject(parent)
    , d(p)
{
}

XdgExporter::~XdgExporter()
{
    release();
}

void XdgExporter::setup(zxdg_exporter_v2 *exporter)
{
    d->setupV2(exporter);
}

void XdgExporter::release()
{
    d->release();
}

void XdgExporter::destroy()
{
    d->destroy();
}

XdgExporter::operator zxdg_exporter_v2*() {
    return d->exporterV2();
}

XdgExporter::operator zxdg_exporter_v2*() const {
    return d->exporterV2();
}

bool XdgExporter::isValid() const
{
    return d->isValid();
}

void XdgExporter::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *XdgExporter::eventQueue()
{
    return d->queue;
}

XdgExported *XdgExporter::exportSurface(Surface *surface, QObject *parent)
{
    return d->exportTopLevelV2(surface, parent);
}



XdgImporter::Private::Private()
{
}

XdgImporter::Private::~Private()
{
}

XdgImporter::XdgImporter(Private *p, QObject *parent)
    : QObject(parent)
    , d(p)
{
}

XdgImporter::~XdgImporter()
{
    release();
}

void XdgImporter::setup(zxdg_importer_v2 *importer)
{
    d->setupV2(importer);
}

void XdgImporter::release()
{
    d->release();
}

void XdgImporter::destroy()
{
    d->destroy();
}

XdgImporter::operator zxdg_importer_v2*() {
    return d->importerV2();
}

XdgImporter::operator zxdg_importer_v2*() const {
    return d->importerV2();
}

bool XdgImporter::isValid() const
{
    return d->isValid();
}

void XdgImporter::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *XdgImporter::eventQueue()
{
    return d->queue;
}

XdgImported *XdgImporter::import(const QString & handle, QObject *parent)
{
    Q_ASSERT(isValid());
    return d->importTopLevelV2(handle, parent);
}

XdgExported::XdgExported(Private *p, QObject *parent)
    : QObject(parent)
    , d(p)
{
}

XdgExported::Private::Private(XdgExported *q)
    : q(q)
{
}

XdgExported::Private::~Private()
{
}

XdgExported::~XdgExported()
{
    release();
}

void XdgExported::setup(zxdg_exported_v2 *exported)
{
    d->setupV2(exported);
}

void XdgExported::release()
{
    d->release();
}

void XdgExported::destroy()
{
    d->destroy();
}

QString XdgExported::handle() const
{
    return d->handle;
}

XdgExported::operator zxdg_exported_v2*() {
    return d->exportedV2();
}

XdgExported::operator zxdg_exported_v2*() const {
    return d->exportedV2();
}

bool XdgExported::isValid() const
{
    return d->isValid();
}

XdgImported::Private::Private(XdgImported *q)
    : q(q)
{
}

XdgImported::Private::~Private()
{
}

XdgImported::XdgImported(Private *p, QObject *parent)
    : QObject(parent)
    , d(p)
{
}

XdgImported::~XdgImported()
{
    release();
}

void XdgImported::setup(zxdg_imported_v2 *imported)
{
    d->setupV2(imported);
}

void XdgImported::release()
{
    d->release();
}

void XdgImported::destroy()
{
    d->destroy();
}

XdgImported::operator zxdg_imported_v2*() {
    return d->importedV2();
}

XdgImported::operator zxdg_imported_v2*() const {
    return d->importedV2();
}

bool XdgImported::isValid() const
{
    return d->isValid();
}

void XdgImported::setParentOf(Surface *surface)
{
    Q_ASSERT(isValid());
    d->setParentOf(surface);
}


}
}

