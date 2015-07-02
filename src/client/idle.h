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
#ifndef KWAYLAND_IDLE_H
#define KWAYLAND_IDLE_H

#include <QObject>

#include <KWayland/Client/kwaylandclient_export.h>

struct org_kde_kwin_idle;
struct org_kde_kwin_idle_timeout;

namespace KWayland
{
namespace Client
{

class EventQueue;
class IdleTimeout;
class Seat;

/**
 * @short Wrapper for the org_kde_kwin_idle interface.
 *
 * This class provides a convenient wrapper for the org_kde_kwin_idle interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the Idle interface:
 * @code
 * Idle *m = registry->createIdle(name, version);
 * @endcode
 *
 * This creates the Idle and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * Idle *m = new Idle;
 * m->setup(registry->bindIdle(name, version));
 * @endcode
 *
 * The Idle can be used as a drop-in replacement for any org_kde_kwin_idle
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 **/
class KWAYLANDCLIENT_EXPORT Idle : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new Idle.
     * Note: after constructing the Idle it is not yet valid and one needs
     * to call setup. In order to get a ready to use Idle prefer using
     * Registry::createIdle.
     **/
    explicit Idle(QObject *parent = nullptr);
    virtual ~Idle();

    /**
     * @returns @c true if managing a org_kde_kwin_idle.
     **/
    bool isValid() const;
    /**
     * Setup this Idle to manage the @p manager.
     * When using Registry::createIdle there is no need to call this
     * method.
     **/
    void setup(org_kde_kwin_idle *manager);
    /**
     * Releases the org_kde_kwin_idle interface.
     * After the interface has been released the Idle instance is no
     * longer valid and can be setup with another org_kde_kwin_idle interface.
     **/
    void release();
    /**
     * Destroys the data hold by this Idle.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid any more, it's not
     * possible to call release any more as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or setup to a new org_kde_kwin_idle interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, manager, &Idle::destroyed);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating a IdleTimeout.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating a IdleTimeout.
     **/
    EventQueue *eventQueue();

    IdleTimeout *getTimeout(quint32 msecs, Seat *seat, QObject *parent = nullptr);

    operator org_kde_kwin_idle*();
    operator org_kde_kwin_idle*() const;

private:
    class Private;
    QScopedPointer<Private> d;
};

/**
 * @short Wrapper for the org_kde_kwin_idle_timeout interface.
 *
 * This class is a convenient wrapper for the org_kde_kwin_idle_timeout interface.
 * To create a IdleTimeout call IdleTimeoutManager::getIdleTimeout.
 *
 * @see IdleTimeoutManager
 **/
class KWAYLANDCLIENT_EXPORT IdleTimeout : public QObject
{
    Q_OBJECT
public:
    explicit IdleTimeout(QObject *parent = nullptr);
    virtual ~IdleTimeout();

    /**
     * Setup this IdleTimeout to manage the @p timeout.
     * When using IdleTimeoutManager::createIdleTimeout there is no need to call this
     * method.
     **/
    void setup(org_kde_kwin_idle_timeout *timeout);
    /**
     * Releases the org_kde_kwin_idle_timeout interface.
     * After the interface has been released the IdleTimeout instance is no
     * longer valid and can be setup with another org_kde_kwin_idle_timeout interface.
     **/
    void release();
    /**
     * Destroys the data hold by this IdleTimeout.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid any more, it's not
     * possible to call release any more as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or setup to a new org_kde_kwin_idle_timeout interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, source, &IdleTimeout::destroyed);
     * @endcode
     *
     * @see release
     **/
    void destroy();
    /**
     * @returns @c true if managing a org_kde_kwin_idle_timeout.
     **/
    bool isValid() const;

    operator org_kde_kwin_idle_timeout*();
    operator org_kde_kwin_idle_timeout*() const;

    void simulateUserActivity();

Q_SIGNALS:
    void idle();
    void resumeFromIdle();

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
