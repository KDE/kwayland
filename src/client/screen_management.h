/********************************************************************
Copyright 2013  Martin Gräßlin <mgraesslin@kde.org>
Copyright 2015  Sebastian Kügler <sebas@kde.org>

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
#ifndef WAYLAND_KWIN_SCREEN_MANAGEMENT_H
#define WAYLAND_KWIN_SCREEN_MANAGEMENT_H

#include <QObject>
#include <QPointer>
#include <QSize>

#include "disabledoutput.h"

#include <KWayland/Client/kwaylandclient_export.h>

struct org_kde_kwin_screen_management;
class QPoint;
class QRect;

namespace KWayland
{
namespace Client
{
class EventQueue;

/**
 * @short Wrapper for the org_kde_kwin_screen_management interface.
 *
 * This class provides a convenient wrapper for the org_kde_kwin_screen_management interface.
 * Its main purpose is to provide information about connected, but disabled screens, i.e.
 * outputs that are not visible in the wl_output interface, but could be enabled by the
 * compositor.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create an KWinScreenManagement interface:
 * @code
 * KWayland::Client::ScreenManagement *c = registry->createScreenManagement(name, version);
 * @endcode
 *
 * This creates the KWinScreenManagement and sets it up directly. As an alternative this
 *
 * The ScreenManagement can be used as a drop-in replacement for any
 * org_kde_kwin_screen_management * pointer as it provides matching cast operators.
 *
 * Please note that all properties of ScreenManagement are not valid until the
 * done signal has been emitted. The wayland server is pushing the
 * information in an async way to the ScreenManagement instance. By emitting done,
 * the ScreenManagement indicates that all relevant information is available.
 *
 * @see Registry
 **/
class KWAYLANDCLIENT_EXPORT ScreenManagement : public QObject
{
    Q_OBJECT
public:
    explicit ScreenManagement(QObject *parent = nullptr);
    virtual ~ScreenManagement();

    /**
     * Setup this ScreenManagement.
     * When using Registry::createScreenManagement there is no need to call this
     * method.
     **/
    void setup(org_kde_kwin_screen_management *screen_management);

    void release();
    void destroy();

    /**
     * Sets the @p queue to use for creating a Surface.
     **/
    void setEventQueue(EventQueue *queue);

    /**
     * @returns The event queue to use for creating a Surface.
     **/
    EventQueue *eventQueue();

    /**
     * @returns @c true if managing a org_kde_kwin_screen_management.
     **/
    bool isValid() const;
    operator org_kde_kwin_screen_management*();
    operator org_kde_kwin_screen_management*() const;

    QList<DisabledOutput*> disabledOutputs() const;

Q_SIGNALS:
    /**
     * Emitted after all DisabledOutputs have been announced initially. This signal
     * can be tracked to get notified once all currently connected, but
     * disabled outputs have been signalled. After done() is fired, disabledOutputs()
     * is up to date.
     **/
    void done();

    /**
     * An output has been connected, but is not enabled yet.
     * @param output A pointer to the DisabledOutput. This pointers lifetime is
     * managed by the ScreenManagement class. Do not delete it yourself.
     */
    void disabledOutputAdded(const KWayland::Client::DisabledOutput*);

    /**
     * A disabled output has been disconnected.
     * @param output A pointer to the DisabledOutput. This pointer may already have
     * been deleted, so do not dereference any of its data, only use its address to
     * identify it.
     */
    void disabledOutputRemoved(const KWayland::Client::DisabledOutput*);

    /**
     * This signal is emitted right before the interface is released.
     **/
    void interfaceAboutToBeReleased();
    /**
     * This signal is emitted right before the interface is destroyed.
     **/
    void interfaceAboutToBeDestroyed();

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
