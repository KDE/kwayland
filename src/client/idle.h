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
 * With the help of Idle it is possible to get notified when a Seat is not being
 * used. E.g. a chat application which wants to set the user automatically to away
 * if the user did not interact with the Seat for 5 minutes can create an IdleTimeout
 * to get notified when the Seat has been idle for the given amount of time.
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
     * Destroys the data held by this Idle.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_kwin_idle interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * Idle gets destroyed.
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

    /**
     * Creates a new IdleTimeout for the @p seat. If the @p seat has been idle,
     * that is none of the connected input devices got used for @p msec, the
     * IdleTimeout will emit the {@link IdleTimeout::idle} signal.
     *
     * It is not guaranteed that the signal will be emitted exactly at the given
     * timeout. A Wayland server might for example have a minimum timeout which is
     * larger than @p msec.
     *
     * @param msec The duration in milliseconds after which an idle timeout should fire
     * @param seat The Seat on which the user activity should be monitored.
     **/
    IdleTimeout *getTimeout(quint32 msecs, Seat *seat, QObject *parent = nullptr);

    operator org_kde_kwin_idle*();
    operator org_kde_kwin_idle*() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the Compositor got created by
     * Registry::createIdle
     *
     * @since 5.5
     **/
    void removed();

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
    /**
     * To create an IdleTimeout prefer using {@link Idle::getTimeout} which sets up the
     * IdleTimeout to be fully functional.
     **/
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
     * Destroys the data held by this IdleTimeout.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_kwin_idle_timeout interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, source, &IdleTimeout::destroy);
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

    /**
     * Simulates user activity. If the IdleTimeout is in idle state this will trigger the
     * {@link resumeFromIdle} signal. The current idle duration is reset, so the {@link idle}
     * will only be emitted after a complete idle duration as requested for this IdleTimeout.
     */
    void simulateUserActivity();

Q_SIGNALS:
    /**
     * Emitted when this IdleTimeout triggered. This means the system has been idle for
     * the duration specified when creating the IdleTimeout.
     * @see Idle::getTimeout.
     * @see resumeFromIdle
     **/
    void idle();
    /**
     * Emitted when the system shows activity again after the idle state was reached.
     * @see idle
     **/
    void resumeFromIdle();

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
