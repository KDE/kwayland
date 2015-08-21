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
 * Its main purpose is to hold the information about one KWinScreenManagement.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create an KWinScreenManagement interface:
 * @code
 * KWinScreenManagement *c = registry->createKWinScreenManagement(name, version);
 * @endcode
 *
 * This creates the KWinScreenManagement and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * KWinScreenManagement *c = new KWinScreenManagement;
 * c->setup(registry->bindKWinScreenManagement(name, version));
 * @endcode
 *
 * The KWinScreenManagement can be used as a drop-in replacement for any org_kde_kwin_screen_management
 * pointer as it provides matching cast operators.
 *
 * Please note that all properties of KWinScreenManagement are not valid until the
 * changed signal has been emitted. The wayland server is pushing the
 * information in an async way to the KWinScreenManagement instance. By emitting changed
 * the KWinScreenManagement indicates that all relevant information is available.
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
     * Setup this Compositor to manage the @p output.
     * When using Registry::createKWinScreenManagement there is no need to call this
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
     * Emitted whenever at least one of the data changed.
     **/
    void done();
    void disabledOutputAdded(const QString &edid, const QString &name, const QString &connector);
    void disabledOutputRemoved(const QString &name, const QString &connector);

    /**
     * This signal is emitted right before the interface is released.
     **/
    void interfaceAboutToBeReleased();
    /**
     * This signal is emitted right before the data is destroyed.
     **/
    void interfaceAboutToBeDestroyed();

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
