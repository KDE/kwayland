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
    * The configuration has been applied. This method is to be called by
    * the compositor after changes have successfully been applied.
    */
    void applied();

private:
    explicit OutputConfigurationInterface(OutputManagementInterface *parent, wl_resource *parentResource);
    friend class OutputManagementInterface;

    class Private;
    Private *d_func() const;
};


}
}

#endif
