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
#ifndef KWAYLAND_CLIENT_XDGFOREIGN_P_H
#define KWAYLAND_CLIENT_XDGFOREIGN_P_H

#include "xdgforeign.h"
#include <QObject>

namespace KWayland
{
namespace Client
{

class XdgExportedUnstableV2;
class XdgImportedUnstableV2;

class Q_DECL_HIDDEN XdgExporter::Private
{
public:
    Private();
    virtual ~Private();

    virtual XdgExported *exportTopLevelV2(Surface *surface, QObject *parent) = 0;

    virtual void setupV2(zxdg_exporter_v2 *arg) = 0;
    virtual zxdg_exporter_v2 *exporterV2() = 0;

    virtual void release() = 0;
    virtual void destroy() = 0;
    virtual bool isValid() = 0;

    EventQueue *queue = nullptr;
};

class Q_DECL_HIDDEN XdgImporter::Private
{
public:
    Private();
    virtual ~Private();

    virtual XdgImported *importTopLevelV2(const QString & handle, QObject *parent) = 0;

    virtual void setupV2(zxdg_importer_v2 *arg) = 0;
    virtual zxdg_importer_v2 *importerV2() = 0;

    virtual void release() = 0;
    virtual void destroy() = 0;
    virtual bool isValid() = 0;

    EventQueue *queue = nullptr;
};


class Q_DECL_HIDDEN XdgExported::Private
{
public:
    Private(XdgExported *q);
    virtual ~Private();

    virtual void setupV2(zxdg_exported_v2 *) = 0;
    virtual zxdg_exported_v2 *exportedV2() = 0;

    virtual void release() = 0;
    virtual void destroy() = 0;
    virtual bool isValid() = 0;

    QString handle;

protected:
    XdgExported *q;

};


class Q_DECL_HIDDEN XdgImported::Private
{
public:
    Private(XdgImported *q);
    virtual ~Private();

    virtual void setupV2(zxdg_imported_v2 *) = 0;
    virtual zxdg_imported_v2 *importedV2() = 0;

    virtual void setParentOf(Surface *surface) = 0;
    virtual void release() = 0;
    virtual void destroy() = 0;
    virtual bool isValid() = 0;

protected:
    XdgImported *q;
};

}
}

#endif
