/********************************************************************
Copyright 2017  Martin Flöser <mgraesslin@kde.org>
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
#ifndef KWAYLAND_SERVER_DATA_CONTROL_OFFERINTERFACE_P_H
#define KWAYLAND_SERVER_DATA_CONTROL_OFFERINTERFACE_P_H
#include "datacontroloffer_interface.h"
#include "datacontrolsource_interface.h"
#include "resource_p.h"
#include <wayland-data-control-v1-server-protocol.h>

namespace KWayland
{
namespace Server
{

class Q_DECL_HIDDEN DataControlOfferInterface::Private : public Resource::Private
{
public:
    Private(DataControlSourceInterface *source, DataControlDeviceInterface *parentInterface, DataControlOfferInterface *q, wl_resource *parentResource);
    ~Private();
    DataControlSourceInterface *source;
    DataControlDeviceInterface *dataDevice;

private:
    DataControlOfferInterface *q_func() {
        return reinterpret_cast<DataControlOfferInterface *>(q);
    }
    void receive(const QString &mimeType, qint32 fd);
    static void receiveCallback(wl_client *client, wl_resource *resource, const char *mimeType, int32_t fd);

    static const struct zwlr_data_control_offer_v1_interface s_interface;
};

}
}

#endif
