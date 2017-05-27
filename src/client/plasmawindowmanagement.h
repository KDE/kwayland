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
class Surface;

/**
 * @short Wrapper for the org_kde_plasma_window_management interface.
 *
 * PlasmaWindowManagement is a privileged interface. A Wayland compositor is allowed to ignore
 * any requests. The PlasmaWindowManagement allows to get information about the overall windowing
 * system. It allows to see which windows are currently available and thus is the base to implement
 * e.g. a task manager.
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
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_plasma_window_management interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, shell, &PlasmaWindowManagement::destroy);
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

    /**
     * Whether the system is currently showing the desktop.
     * This means that the system focuses on the desktop and hides other windows.
     * @see setShowingDesktop
     * @see showDesktop
     * @see hideDesktop
     * @see showingDesktopChanged
     **/
    bool isShowingDesktop() const;
    /**
     * Requests to change the showing desktop state to @p show.
     * @see isShowingDesktop
     * @see showDesktop
     * @see hideDesktop
     **/
    void setShowingDesktop(bool show);
    /**
     * Same as calling setShowingDesktop with @c true.
     * @see setShowingDesktop
     **/
    void showDesktop();
    /**
     * Same as calling setShowingDesktop with @c false.
     * @see setShowingDesktop
     **/
    void hideDesktop();

    /**
     * @returns All windows currently known to the PlasmaWindowManagement
     * @see windowCreated
     **/
    QList<PlasmaWindow*> windows() const;
    /**
     * @returns The currently active PlasmaWindow, the PlasmaWindow which
     * returns @c true in @link{PlasmaWindow::isActive} or @c nullptr in case
     * there is no active window.
     **/
    PlasmaWindow *activeWindow() const;
    /**
     * Factory method to create a PlasmaWindowModel.
     * @returns a new created PlasmaWindowModel
     **/
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
    /**
     * The showing desktop state changed.
     * @see isShowingDesktop
     **/
    void showingDesktopChanged(bool);

    /**
     * A new @p window got created.
     * @see windows
     **/
    void windowCreated(KWayland::Client::PlasmaWindow *window);
    /**
     * The active window changed.
     * @see activeWindow
     **/
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
 * A PlasmaWindow gets created by the PlasmaWindowManagement and announced through
 * the @link{PlasmaWindowManagement::windowCreated} signal. The PlasmaWindow encapsulates
 * state about a window managed by the Wayland server and allows to request state changes.
 *
 * The PlasmaWindow will be automatically deleted when the PlasmaWindow gets unmapped.
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
     * Destroys the data held by this PlasmaWindow.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_plasma_window interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, source, &PlasmaWindow::destroy);
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

    /**
     * @returns the window title.
     * @see titleChanged
     **/
    QString title() const;
    /**
     * @returns the application id which should reflect the name of a desktop file.
     * @see appIdChanged
     **/
    QString appId() const;
    /**
     * @returns the id of the virtual desktop this PlasmaWindow is on
     * @see virtualDesktopChanged
     **/
    quint32 virtualDesktop() const;
    /**
     * @returns Whether the window is currently the active Window.
     * @see activeChanged
     **/
    bool isActive() const;
    /**
     * @returns Whether the window is fullscreen
     * @see fullscreenChanged
     **/
    bool isFullscreen() const;
    /**
     * @returns Whether the window is kept above other windows.
     * @see keepAboveChanged
     **/
    bool isKeepAbove() const;
    /**
     * @returns Whether the window is kept below other window
     * @see keepBelowChanged
     **/
    bool isKeepBelow() const;
    /**
     * @returns Whether the window is currently minimized
     * @see minimizedChanged
     **/
    bool isMinimized() const;
    /**
     * @returns Whether the window is maximized.
     * @see maximizedChanged
     **/
    bool isMaximized() const;
    /**
     * @returns Whether the window is shown on all desktops.
     * @see virtualDesktop
     * @see onAllDesktopsChanged
     **/
    bool isOnAllDesktops() const;
    /**
     * @returns Whether the window is demanding attention.
     * @see demandsAttentionChanged
     **/
    bool isDemandingAttention() const;
    /**
     * @returns Whether the window can be closed.
     * @see closeableChanged
     **/
    bool isCloseable() const;
    /**
     * @returns Whether the window can be maximized.
     * @see maximizeableChanged
     **/
    bool isMaximizeable() const;
    /**
     * @returns Whether the window can be minimized.
     * @see minimizeableChanged
     **/
    bool isMinimizeable() const;
    /**
     * @returns Whether the window can be set to fullscreen.
     * @see fullscreenableChanged
     **/
    bool isFullscreenable() const;
    /**
     * @returns Whether the window should be ignored by a task bar.
     * @see skipTaskbarChanged
     **/
    bool skipTaskbar() const;
    /**
     * @returns The icon of the window.
     * @see iconChanged
     **/
    QIcon icon() const;
    /**
     * @returns Whether the window can be set to the shaded state.
     * @see isShaded
     * @see shadeableChanged
     * @since 5.22
     */
    bool isShadeable() const;
    /**
     * @returns Whether the window is shaded, that is reduced to the window decoration
     * @see shadedChanged
     * @since 5.22
     */
    bool isShaded() const;
    /**
     * @returns Whether the window can be moved.
     * @see movableChanged
     * @since 5.22
     */
    bool isMovable() const;
    /**
     * @returns Whether the window can be resized.
     * @see resizableChanged
     * @since 5.22
     */
    bool isResizable() const;
    /**
     * @returns Whether the virtual desktop can be changed.
     * @see virtualDesktopChangeableChanged
     * @since 5.22
     */
    bool isVirtualDesktopChangeable() const;
    /**
     * @returns The process id this window belongs to.
     * or 0 if unset
     * @since 5.35
     */
    quint32 pid() const;

    /**
     * Requests to activate the window.
     **/
    void requestActivate();
    /**
     * Requests to close the window.
     **/
    void requestClose();
    /**
     * Requests to start an interactive window move operation.
     * @since 5.22
     */
    void requestMove();
    /**
     * Requests to start an interactive resize operation.
     * @since 5.22
     */
    void requestResize();
    /**
     * Requests to send the window to virtual @p desktop.
     **/
    void requestVirtualDesktop(quint32 desktop);

    /**
     * Requests the window at this model row index have its keep above state toggled.
     * @since 5.35
     */
    void requestToggleKeepAbove();

    /**
     * Requests the window at this model row index have its keep below state toggled.
     * @since 5.35
     */
    void requestToggleKeepBelow();

    /**
     * Requests the window at this model row index have its minimized state toggled.
     */
    void requestToggleMinimized();

    /**
     * Requests the window at this model row index have its maximized state toggled.
     */
    void requestToggleMaximized();

    /**
     * Sets the geometry of the taskbar entry for this window
     * relative to a panel in particular
     * @since 5.5
     */
    void setMinimizedGeometry(Surface *panel, const QRect &geom);

    /**
     * Remove the task geometry information for a particular panel
     * @since 5.5
     */
    void unsetMinimizedGeometry(Surface *panel);

    /**
     * Requests the window at this model row index have its shaded state toggled.
     * @since 5.22
     */
    void requestToggleShaded();

    /**
     * An internal window identifier.
     * This is not a global window identifier.
     * This identifier does not correspond to QWindow::winId in any way.
     **/
    quint32 internalId() const;

    /**
     * The parent window of this PlasmaWindow.
     *
     * If there is a parent window, this window is a transient window for the
     * parent window. If this method returns a null PlasmaWindow it means this
     * window is a top level window and is not a transient window.
     *
     * @see parentWindowChanged
     * @since 5.24
     **/
    QPointer<PlasmaWindow> parentWindow() const;

    /**
     * @returns The window geometry in absolute coordinates.
     * @see geometryChanged
     * @since 5.25
     **/
    QRect geometry() const;

Q_SIGNALS:
    /**
     * The window title changed.
     * @see title
     **/
    void titleChanged();
    /**
     * The application id changed.
     * @see appId
     **/
    void appIdChanged();
    /**
     * The virtual desktop changed.
     * @see virtualDesktop
     **/
    void virtualDesktopChanged();
    /**
     * The window became active or inactive.
     * @see isActive
     **/
    void activeChanged();
    /**
     * The fullscreen state changed.
     * @see isFullscreen
     **/
    void fullscreenChanged();
    /**
     * The keep above state changed.
     * @see isKeepAbove
     **/
    void keepAboveChanged();
    /**
     * The keep below state changed.
     * @see isKeepBelow
     **/
    void keepBelowChanged();
    /**
     * The minimized state changed.
     * @see isMinimized
     **/
    void minimizedChanged();
    /**
     * The maximized state changed.
     * @see isMaximized
     **/
    void maximizedChanged();
    /**
     * The on all desktops state changed.
     * @see isOnAllDesktops
     **/
    void onAllDesktopsChanged();
    /**
     * The demands attention state changed.
     * @see isDemandingAttention
     **/
    void demandsAttentionChanged();
    /**
     * The closeable state changed.
     * @see isCloseable
     **/
    void closeableChanged();
    /**
     * The minimizeable state changed.
     * @see isMinimizeable
     **/
    void minimizeableChanged();
    /**
     * The maximizeable state changed.
     * @see isMaximizeable
     **/
    void maximizeableChanged();
    /**
     * The fullscreenable state changed.
     * @see isFullscreenable
     **/
    void fullscreenableChanged();
    /**
     * The skip taskbar state changed.
     * @see skipTaskbar
     **/
    void skipTaskbarChanged();
    /**
     * The window icon changed.
     * @see icon
     **/
    void iconChanged();
    /**
     * The shadeable state changed.
     * @see isShadeable
     * @since 5.22
     */
    void shadeableChanged();
    /**
     * The shaded state changed.
     * @see isShaded
     * @since 5.22
     */
    void shadedChanged();
    /**
     * The movable state changed.
     * @see isMovable
     * @since 5.22
     */
    void movableChanged();
    /**
     * The resizable state changed.
     * @see isResizable
     * @since 5.22
     */
    void resizableChanged();
    /**
     * The virtual desktop changeable state changed.
     * @see virtualDesktopChangeable
     * @since 5.22
     */
    void virtualDesktopChangeableChanged();
    /**
     * The window got unmapped and is no longer available to the Wayland server.
     * This instance will be automatically deleted and one should connect to this
     * signal to perform cleanup.
     **/
    void unmapped();
    /**
     * This signal is emitted whenever the parent window changes.
     * @see parentWindow
     * @since 5.24
     **/
    void parentWindowChanged();
    /**
     * This signal is emitted whenever the window geometry changes.
     * @see geometry
     * @since 5.25
     **/
    void geometryChanged();

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
