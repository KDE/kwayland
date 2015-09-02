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
#ifndef WAYLAND_PLASMAWINDOWMANAGEMENT_H
#define WAYLAND_PLASMAWINDOWMANAGEMENT_H

#include <QObject>
#include <QIcon>
#include <QSize>

#include <KWayland/Client/kwaylandclient_export.h>

struct org_kde_plasma_window_management;
struct org_kde_plasma_window;

namespace KWayland
{
namespace Client
{
class EventQueue;
class PlasmaWindow;
class PlasmaWindowModel;

/**
 * @short Wrapper for the org_kde_plasma_window_management interface.
 *
 * This class provides a convenient wrapper for the org_kde_plasma_window_management interface.
 * It's main purpose is to create a PlasmaWindowManagementSurface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the Shell interface:
 * @code
 * PlasmaWindowManagement *s = registry->createPlasmaWindowManagement(name, version);
 * @endcode
 *
 * This creates the PlasmaWindowManagement and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * PlasmaWindowManagement *s = new PlasmaWindowManagement;
 * s->setup(registry->bindPlasmaWindowManagement(name, version));
 * @endcode
 *
 * The PlasmaWindowManagement can be used as a drop-in replacement for any org_kde_plasma_window_management
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 * @see PlasmaWindowManagementSurface
 **/
class KWAYLANDCLIENT_EXPORT PlasmaWindowManagement : public QObject
{
    Q_OBJECT
public:
    explicit PlasmaWindowManagement(QObject *parent = nullptr);
    virtual ~PlasmaWindowManagement();

    /**
     * @returns @c true if managing a org_kde_plasma_window_management.
     **/
    bool isValid() const;
    /**
     * Releases the org_kde_plasma_window_management interface.
     * After the interface has been released the PlasmaWindowManagement instance is no
     * longer valid and can be setup with another org_kde_plasma_window_management interface.
     *
     * Right before the interface is released the signal interfaceAboutToBeReleased is emitted.
     * @see interfaceAboutToBeReleased
     **/
    void release();
    /**
     * Destroys the data held by this PlasmaWindowManagement.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. Once the connection becomes invalid, it's not
     * possible to call release any more as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or setup to a new org_kde_plasma_window_management interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, shell, &PlasmaWindowManagement::destroyed);
     * @endcode
     *
     * Right before the data is destroyed, the signal interfaceAboutToBeDestroyed is emitted.
     *
     * @see release
     * @see interfaceAboutToBeDestroyed
     **/
    void destroy();
    /**
     * Setup this Shell to manage the @p shell.
     * When using Registry::createShell there is no need to call this
     * method.
     **/
    void setup(org_kde_plasma_window_management *shell);

    /**
     * Sets the @p queue to use for creating a Surface.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating a Surface.
     **/
    EventQueue *eventQueue();

    operator org_kde_plasma_window_management*();
    operator org_kde_plasma_window_management*() const;

    bool isShowingDesktop() const;
    void setShowingDesktop(bool show);
    void showDesktop();
    void hideDesktop();

    QList<PlasmaWindow*> windows() const;
    PlasmaWindow *activeWindow() const;
    PlasmaWindowModel *createWindowModel();

Q_SIGNALS:
    /**
     * This signal is emitted right before the interface is released.
     **/
    void interfaceAboutToBeReleased();
    /**
     * This signal is emitted right before the data is destroyed.
     **/
    void interfaceAboutToBeDestroyed();
    void showingDesktopChanged(bool);

    void windowCreated(KWayland::Client::PlasmaWindow *window);
    void activeWindowChanged();

    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the Compositor got created by
     * Registry::createPlasmaWindowManagement
     *
     * @since 5.5
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

/**
 * @short Wrapper for the org_kde_plasma_window interface.
 *
 * This class is a convenient wrapper for the org_kde_plasma_window interface.
 * The PlasmaWindow gets created by PlasmaWindowManagement.
 *
 * @see PlasmaWindowManager
 **/
class KWAYLANDCLIENT_EXPORT PlasmaWindow : public QObject
{
    Q_OBJECT
public:
    virtual ~PlasmaWindow();

    /**
     * Releases the org_kde_plasma_window interface.
     * After the interface has been released the PlasmaWindow instance is no
     * longer valid and can be setup with another org_kde_plasma_window interface.
     **/
    void release();
    /**
     * Destroys the data hold by this PlasmaWindow.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid any more, it's not
     * possible to call release any more as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or setup to a new org_kde_plasma_window interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, source, &PlasmaWindow::destroyed);
     * @endcode
     *
     * @see release
     **/
    void destroy();
    /**
     * @returns @c true if managing a org_kde_plasma_window.
     **/
    bool isValid() const;

    operator org_kde_plasma_window*();
    operator org_kde_plasma_window*() const;

    QString title() const;
    QString appId() const;
    quint32 virtualDesktop() const;
    bool isActive() const;
    bool isFullscreen() const;
    bool isKeepAbove() const;
    bool isKeepBelow() const;
    bool isMinimized() const;
    bool isMaximized() const;
    bool isOnAllDesktops() const;
    bool isDemandingAttention() const;
    bool isCloseable() const;
    bool isMaximizeable() const;
    bool isMinimizeable() const;
    bool isFullscreenable() const;
    QIcon icon() const;

    void requestActivate();
    void requestClose();
    void requestVirtualDesktop(quint32 desktop);

    /**
     * An internal window identifier.
     * This is not a global window identifier.
     * This identifier does not correspond to QWindow::winId in any way.
     **/
    quint32 internalId() const;

Q_SIGNALS:
    void titleChanged();
    void appIdChanged();
    void virtualDesktopChanged();
    void activeChanged();
    void fullscreenChanged();
    void keepAboveChanged();
    void keepBelowChanged();
    void minimizedChanged();
    void maximizedChanged();
    void onAllDesktopsChanged();
    void demandsAttentionChanged();
    void closeableChanged();
    void minimizeableChanged();
    void maximizeableChanged();
    void fullscreenableChanged();
    void iconChanged();
    void unmapped();

private:
    friend class PlasmaWindowManagement;
    explicit PlasmaWindow(PlasmaWindowManagement *parent, org_kde_plasma_window *dataOffer, quint32 internalId);
    class Private;
    QScopedPointer<Private> d;
};

}
}

Q_DECLARE_METATYPE(KWayland::Client::PlasmaWindow*)

#endif
