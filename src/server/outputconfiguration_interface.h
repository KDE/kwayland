/****************************************************************************
* Copyright 2015  Sebastian Kügler <sebas@kde.org>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) version 3, or any
* later version accepted by the membership of KDE e.V. (or its
* successor approved by the membership of KDE e.V.), which shall
* act as a proxy defined in Section 6 of version 3 of the license.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/
#ifndef KWAYLAND_SERVER_OUTPUTCONFIGURATION_H
#define KWAYLAND_SERVER_OUTPUTCONFIGURATION_H

#include "global.h"
#include "resource.h"
#include "outputmanagement_interface.h"

#include <KWayland/Server/kwaylandserver_export.h>

namespace KWayland
{
namespace Server
{

class Display;


class KWAYLANDSERVER_EXPORT OutputConfigurationInterface : public Resource
{
    Q_OBJECT
public:
    virtual ~OutputConfigurationInterface();

public Q_SLOTS:
    /**
     * Called by the compositor once the changes have successfully been applied.
     * OutputDevices are updated here, then the applied signal is sent to the
     * client.
     */
    void setApplied();
    /**
     * Called by the compositor when the changes as a whole are rejected or
     * failed to apply.
     */
    void setFailed();

Q_SIGNALS:
    void applyRequested();

private:
    explicit OutputConfigurationInterface(OutputManagementInterface *parent, wl_resource *parentResource);
    friend class OutputManagementInterface;

    class Private;
    Private *d_func() const;
};


}
}

Q_DECLARE_METATYPE(KWayland::Server::OutputConfigurationInterface*)

#endif
