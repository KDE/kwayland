/********************************************************************
Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
Copyright 2019  David Edmundson <davidedmundson@kde.org>

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
*********************************************************************/
#ifndef WAYLAND_SERVER_DATA_CONTROL_DEVICE_MANAGER_INTERFACE_H
#define WAYLAND_SERVER_DATA_CONTROL_DEVICE_MANAGER_INTERFACE_H

#include <QObject>

#include <KWayland/Server/kwaylandserver_export.h>
#include "global.h"
#include "datacontroldevice_interface.h"

namespace KWayland
{
namespace Server
{

class Display;
class DataControlSourceInterface;

/**
 * @brief Represents the Global for zwlr_data_control_manager_v1 interface.
 *
 **/
class KWAYLANDSERVER_EXPORT DataControlDeviceManagerInterface : public Global
{
    Q_OBJECT
public:
    virtual ~DataControlDeviceManagerInterface();

Q_SIGNALS:
    void dataSourceCreated(KWayland::Server::DataControlSourceInterface*);
    void dataDeviceCreated(KWayland::Server::DataControlDeviceInterface*);

private:
    explicit DataControlDeviceManagerInterface(Display *display, QObject *parent = nullptr);
    friend class Display;
    class Private;
};

}
}

#endif
