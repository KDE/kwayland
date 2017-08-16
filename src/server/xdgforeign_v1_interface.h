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
#ifndef KWAYLAND_SERVER_XDGFOREIGNV1_H
#define KWAYLAND_SERVER_XDGFOREIGNV1_H

#include "global.h"
#include "resource.h"

#include <KWayland/Server/kwaylandserver_export.h>

namespace KWayland
{
namespace Server
{

class Display;
class SurfaceInterface;
class XdgExporterUnstableV1Interface;
class XdgImporterUnstableV1Interface;
class XdgExportedUnstableV1Interface;
class XdgImportedUnstableV1Interface;

class KWAYLANDSERVER_EXPORT XdgForeignUnstableV1Interface : public QObject
{
    Q_OBJECT
public:
    XdgForeignUnstableV1Interface(Display *display, QObject *parent = nullptr);
    ~XdgForeignUnstableV1Interface();

    void create();
    bool isValid();

    XdgExportedUnstableV1Interface *exportedSurface(const QString &handle);
    XdgImportedUnstableV1Interface *importedSurface(const QString &handle);

    SurfaceInterface *transientFor(SurfaceInterface *surface);

Q_SIGNALS:
    void surfaceImported(const QString &handle, XdgImportedUnstableV1Interface *imported);
    void surfaceExported(const QString &handle, XdgExportedUnstableV1Interface *exported);
    void transientChanged(KWayland::Server::SurfaceInterface *child, KWayland::Server::SurfaceInterface *parent);

private:
    friend class Display;
    class Private;
    Private *d;
};

class KWAYLANDSERVER_EXPORT XdgExportedUnstableV1Interface : public Resource
{
    Q_OBJECT
public:
    virtual ~XdgExportedUnstableV1Interface();

private:
    explicit XdgExportedUnstableV1Interface(XdgExporterUnstableV1Interface *parent, wl_resource *parentResource);
    friend class XdgExporterUnstableV1Interface;

    class Private;
    Private *d_func() const;
};

class KWAYLANDSERVER_EXPORT XdgImportedUnstableV1Interface : public Resource
{
    Q_OBJECT
public:
    virtual ~XdgImportedUnstableV1Interface();

    SurfaceInterface *child() const;

Q_SIGNALS:
    void childChanged(KWayland::Server::SurfaceInterface *child);

private:
    explicit XdgImportedUnstableV1Interface(XdgImporterUnstableV1Interface *parent, wl_resource *parentResource);
    friend class XdgImporterUnstableV1Interface;

    class Private;
    Private *d_func() const;
};


}
}

#endif
