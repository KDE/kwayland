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
#ifndef KWAYLAND_CLIENT_XDGFOREIGN_V1_H
#define KWAYLAND_CLIENT_XDGFOREIGN_V1_H

#include "xdgforeign.h"
#include "surface.h"

#include <QObject>

#include <KWayland/Client/kwaylandclient_export.h>

struct zxdg_exporter_v1;
struct zxdg_importer_v1;
struct zxdg_exported_v1;
struct zxdg_imported_v1;

namespace KWayland
{
namespace Client
{

class EventQueue;
class Surface;
class XdgExportedUnstableV1;
class XdgImportedUnstableV1;

/**
 * @short Wrapper for the zxdg_exporter_v1 interface.
 *
 * This class provides a convenient wrapper for the zxdg_exporter_v1 interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the  interface:
 * @code
 *  *c = registry->create(name, version);
 * @endcode
 *
 * This creates the  and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 *  *c = new ;
 * c->setup(registry->bind(name, version));
 * @endcode
 *
 * The  can be used as a drop-in replacement for any zxdg_exporter_v1
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 **/
class Q_DECL_HIDDEN XdgExporterUnstableV1 : public XdgExporterUnstable
{
    Q_OBJECT
public:
    /**
     * Creates a new .
     * Note: after constructing the  it is not yet valid and one needs
     * to call setup. In order to get a ready to use  prefer using
     * Registry::create.
     **/
    explicit XdgExporterUnstableV1(QObject *parent = nullptr);
    virtual ~XdgExporterUnstableV1();

private:
    class Private;
};

/**
 * @short Wrapper for the zxdg_importer_v1 interface.
 *
 * This class provides a convenient wrapper for the zxdg_importer_v1 interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the  interface:
 * @code
 *  *c = registry->create(name, version);
 * @endcode
 *
 * This creates the  and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 *  *c = new ;
 * c->setup(registry->bind(name, version));
 * @endcode
 *
 * The  can be used as a drop-in replacement for any zxdg_importer_v1
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 **/
class Q_DECL_HIDDEN XdgImporterUnstableV1 : public XdgImporterUnstable
{
    Q_OBJECT
public:
    /**
     * Creates a new .
     * Note: after constructing the  it is not yet valid and one needs
     * to call setup. In order to get a ready to use  prefer using
     * Registry::create.
     **/
    explicit XdgImporterUnstableV1(QObject *parent = nullptr);
    virtual ~XdgImporterUnstableV1();

private:
    class Private;
};

class Q_DECL_HIDDEN XdgExportedUnstableV1 : public XdgExportedUnstable
{
    Q_OBJECT
public:
    virtual ~XdgExportedUnstableV1();

private:
    friend class XdgExporterUnstableV1;
    explicit XdgExportedUnstableV1(QObject *parent = nullptr);
    class Private;
};

class Q_DECL_HIDDEN XdgImportedUnstableV1 : public XdgImportedUnstable
{
    Q_OBJECT
public:
    virtual ~XdgImportedUnstableV1();

private:
    friend class XdgImporterUnstableV1;
    explicit XdgImportedUnstableV1(QObject *parent = nullptr);
    class Private;
};


}
}

#endif
