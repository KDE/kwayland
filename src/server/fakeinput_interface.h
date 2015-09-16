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
#ifndef KWAYLAND_SERVER_FAKEINPUT_INTERFACE_H
#define KWAYLAND_SERVER_FAKEINPUT_INTERFACE_H

#include <KWayland/Server/kwaylandserver_export.h>
#include "global.h"

struct wl_resource;

namespace KWayland
{
namespace Server
{

class Display;
class FakeInputDevice;

/**
 * @brief Represents the Global for org_kde_kwin_fake_input interface.
 *
 * The fake input interface allows clients to send fake input events to the
 * Wayland server. For the actual events it creates a FakeInputDevice. Whenever
 * the FakeInputInterface creates a device the signal deviceCreated gets emitted.
 *
 * Accepting fake input events is a security risk. The server should make a
 * dedicated decision about whether it wants to accept fake input events from a
 * device. Because of that by default no events are forwarded to the server. The
 * device needs to request authentication and the server must explicitly authenticate
 * the device. The recommendation is that the server only accepts input for in some
 * way trusted clients.
 *
 * @see FakeInputDevice
 * @since 5.4
 **/
class KWAYLANDSERVER_EXPORT FakeInputInterface : public Global
{
    Q_OBJECT
public:
    virtual ~FakeInputInterface();

Q_SIGNALS:
    /**
     * Signal emitted whenever a client bound the fake input @p device.
     * @param device The created FakeInputDevice
     **/
    void deviceCreated(KWayland::Server::FakeInputDevice *device);

private:
    explicit FakeInputInterface(Display *display, QObject *parent = nullptr);
    friend class Display;
    class Private;
};

/**
 * @brief Represents the Resource for a org_kde_kwin_fake_input interface.
 *
 * @see FakeInputInterface
 * @since 5.4
 **/
class KWAYLANDSERVER_EXPORT FakeInputDevice : public QObject
{
    Q_OBJECT
public:
    virtual ~FakeInputDevice();
    /**
     * @returns the native wl_resource.
     **/
    wl_resource *resource();

    /**
     * Authenticate this device to send events. If @p authenticated is @c true events are
     * accepted, for @c false events are no longer accepted.
     *
     * @param authenticated Whether the FakeInputDevice should be considered authenticated
     **/
    void setAuthentication(bool authenticated);
    /**
     * @returns whether the FakeInputDevice is authenticated and allowed to send events, default is @c false.
     **/
    bool isAuthenticated() const;

Q_SIGNALS:
    /**
     * Request for authentication.
     *
     * The server might use the provided information to make a decision on whether the
     * FakeInputDevice should get authenticated. It is recommended to not trust the data
     * and to combine it with information from ClientConnection.
     *
     * @param application A textual description of the application
     * @param reason A textual description of the reason why the application wants to send fake input events
     **/
    void authenticationRequested(const QString &application, const QString &reason);
    /**
     * Request a pointer motion by @p delta.
     **/
    void pointerMotionRequested(const QSizeF &delta);
    /**
     * Requests a pointer button pressed for @p button.
     **/
    void pointerButtonPressRequested(quint32 button);
    /**
     * Requests a pointer button release for @p button.
     **/
    void pointerButtonReleaseRequested(quint32 button);
    /**
     * Requests a pointer axis for the given @p orientation by @p delta.
     **/
    void pointerAxisRequested(Qt::Orientation orientation, qreal delta);

private:
    friend class FakeInputInterface;
    FakeInputDevice(wl_resource *resource, FakeInputInterface *parent);
    class Private;
    QScopedPointer<Private> d;
};

}
}

Q_DECLARE_METATYPE(KWayland::Server::FakeInputDevice*)

#endif
