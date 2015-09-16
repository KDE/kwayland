/********************************************************************
Copyright 2015  Martin Gräßlin <mgraesslin@kde.org>

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
#ifndef KWAYLAND_SERVER_IDLE_INTERFACE_H
#define KWAYLAND_SERVER_IDLE_INTERFACE_H

#include <KWayland/Server/kwaylandserver_export.h>
#include "global.h"
#include "resource.h"

namespace KWayland
{
namespace Server
{

class Display;
class SeatInterface;

/**
 * @brief Global representing the org_kde_kwin_idle interface.
 *
 * The IdleInterface allows to register callbacks which are invoked if there has
 * not been any user activity (no input) for a specified time span on a seat.
 *
 * A client can bind an idle timeout for a SeatInterface and through that register
 * an idle timeout. The complete interaction is handled internally, thus the API
 * user only needs to create the IdleInterface in order to provide this feature.
 *
 * This interface is useful for clients as it allows them to perform power management,
 * chat applications might want to set to away after no user input for some time, etc.
 *
 * Of course this exposes the global input usage to all clients. Normally clients don't
 * know whether the input devices are used, only if their surfaces have focus. With this
 * interface it is possible to notice that there are input events. A server should consider
 * this to decide whether it wants to provide this feature!
 *
 * @since 5.4
 **/
class KWAYLANDSERVER_EXPORT IdleInterface : public Global
{
    Q_OBJECT
public:
    virtual ~IdleInterface();

private:
    explicit IdleInterface(Display *display, QObject *parent = nullptr);
    friend class Display;
    class Private;
};

// TODO: KF6 make private class
class KWAYLANDSERVER_EXPORT IdleTimeoutInterface : public Resource
{
    Q_OBJECT
public:
    virtual ~IdleTimeoutInterface();

private:
    explicit IdleTimeoutInterface(SeatInterface *seat, IdleInterface *parent, wl_resource *parentResource);
    friend class IdleInterface;
    class Private;
    Private *d_func() const;
};

}
}

#endif
