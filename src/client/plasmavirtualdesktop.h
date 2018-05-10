/****************************************************************************
Copyright 2018  Marco Martin <notmart@gmail.com>

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
****************************************************************************/
#ifndef KWAYLAND_CLIENT_PLASMAVIRTUALDESKTOP_H
#define KWAYLAND_CLIENT_PLASMAVIRTUALDESKTOP_H

#include <QObject>

#include <KWayland/Client/kwaylandclient_export.h>

struct org_kde_plasma_virtual_desktop_management;
struct org_kde_plasma_virtual_desktop;

namespace KWayland
{
namespace Client
{

class EventQueue;
class PlasmaVirtualDesktop;

/**
 * @short Wrapper for the org_kde_plasma_virtual_desktop_management interface.
 *
 * This class provides a convenient wrapper for the org_kde_plasma_virtual_desktop_management interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the PlasmaVirtualDesktopManagement interface:
 * @code
 * PlasmaVirtualDesktopManagement *c = registry->createPlasmaVirtualDesktopManagement(name, version);
 * @endcode
 *
 * This creates the PlasmaVirtualDesktopManagement and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * PlasmaVirtualDesktopManagement *c = new PlasmaVirtualDesktopManagement;
 * c->setup(registry->bindPlasmaVirtualDesktopManagement(name, version));
 * @endcode
 *
 * The PlasmaVirtualDesktopManagement can be used as a drop-in replacement for any org_kde_plasma_virtual_desktop_management
 * pointer as it provides matching cast operators.
 * @since 5.46
 *
 * @see Registry
 **/
class KWAYLANDCLIENT_EXPORT PlasmaVirtualDesktopManagement : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new PlasmaVirtualDesktopManagement.
     * Note: after constructing the PlasmaVirtualDesktopManagement it is not yet valid and one needs
     * to call setup. In order to get a ready to use PlasmaVirtualDesktopManagement prefer using
     * Registry::createPlasmaVirtualDesktopManagement.
     **/
    explicit PlasmaVirtualDesktopManagement(QObject *parent = nullptr);
    virtual ~PlasmaVirtualDesktopManagement();

    /**
     * Setup this PlasmaVirtualDesktopManagement to manage the @p plasmavirtualdesktopmanagement.
     * When using Registry::createPlasmaVirtualDesktopManagement there is no need to call this
     * method.
     **/
    void setup(org_kde_plasma_virtual_desktop_management *plasmavirtualdesktopmanagement);
    /**
     * @returns @c true if managing a org_kde_plasma_virtual_desktop_management.
     **/
    bool isValid() const;
    /**
     * Releases the org_kde_plasma_virtual_desktop_management interface.
     * After the interface has been released the PlasmaVirtualDesktopManagement instance is no
     * longer valid and can be setup with another org_kde_plasma_virtual_desktop_management interface.
     **/
    void release();
    /**
     * Destroys the data held by this PlasmaVirtualDesktopManagement.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_plasma_virtual_desktop_management interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, plasmavirtualdesktopmanagement, &PlasmaVirtualDesktopManagement::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating objects with this PlasmaVirtualDesktopManagement.
     **/
    void setEventQueue(EventQueue *queue);

    /**
     * @returns The event queue to use for creating objects with this PlasmaVirtualDesktopManagement.
     * The object is owned by the manager and the caller should not delete it.
     **/
    EventQueue *eventQueue();

    PlasmaVirtualDesktop *getVirtualDesktop(const QString &id);

    /**
     * @returns All the existent virtual desktops
     */
    QList <PlasmaVirtualDesktop *> desktops() const;

    /**
     * @returns How many rows the visual representation should have
     */
    quint32 rows() const;

    /**
     * @returns How many columns the visual representation should have
     */
    quint32 columns() const;

    operator org_kde_plasma_virtual_desktop_management*();
    operator org_kde_plasma_virtual_desktop_management*() const;

Q_SIGNALS:
    void removed();

    /**
     * Emitted when a new desktop has been added
     */
    void desktopAdded(const QString &id);

    /**
     * Emitted when a desktop has been removed
     */
    void desktopRemoved(const QString &id);

    /**
     * Metaphorically arrange desktops in a grid, which can be used for
     * the pager or for the animation between desktops, or effects like the desktop grid.
     * Emitted when the server decided a new layout.
     */
    void layout(quint32 row, quint32 column);

    /**
     * This event is sent after all other properties has been
     * sent after binding to the desktop manager object and after any
     * other property changes done after that. This allows
     * changes to the org_kde_plasma_virtual_desktop_management properties
     * to be seen as atomic, even if they happen via multiple events.
     */
    void done();

private:
    class Private;
    QScopedPointer<Private> d;
};

class KWAYLANDCLIENT_EXPORT PlasmaVirtualDesktop : public QObject
{
    Q_OBJECT
public:
    virtual ~PlasmaVirtualDesktop();

    /**
     * Setup this PlasmaVirtualDesktop to manage the @p plasmavirtualdesktop.
     * When using PlasmaVirtualDesktopManagement::createPlasmaVirtualDesktop there is no need to call this
     * method.
     **/
    void setup(org_kde_plasma_virtual_desktop *plasmavirtualdesktop);

    /**
     * @returns @c true if managing a org_kde_plasma_virtual_desktop.
     **/
    bool isValid() const;

    /**
     * Releases the org_kde_plasma_virtual_desktop interface.
     * After the interface has been released the PlasmaVirtualDesktop instance is no
     * longer valid and can be setup with another org_kde_plasma_virtual_desktop interface.
     **/
    void release();

    /**
     * Destroys the data held by this PlasmaVirtualDesktop.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_plasma_virtual_desktop interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, plasmavirtualdesktop, &PlasmaVirtualDesktop::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Requests this desktop to be activated.
     * The server may or may not decide to consent to the request.
     */
    void requestActivate();

    /**
     * @returns The unique id of this desktop. The format of the id is decided by the compositor
     */
    QString id() const;


    /**
     * @returns User readable name for the desktop.
     */
    QString name() const;

    /**
     * @returns The row position of this desktop in the layout
     */
    quint32 row() const;

    /**
     * @returns The column position of this desktop in the layout.
     */
    quint32 column() const;

    /**
     * @returns True if the desktop is the active one.
     * when this property changes, activated or deactivated will be emitted.
     * @see activated
     * @see deactivated
     */
    bool active() const;

    operator org_kde_plasma_virtual_desktop*();
    operator org_kde_plasma_virtual_desktop*() const;

Q_SIGNALS:
    /**
     * TODO: activeChanged(bool)?
     * Emitted when this desktop has been activated by the server
     */
    void activated();

    /**
     * Emitted when this desktop has been activated by the server
     */
    void deactivated();

    /**
     * This event is sent after all other properties has been
     * sent after binding to the desktop manager object and after any
     * other property changes done after that. This allows
     * changes to the org_kde_plasma_virtual_desktop properties
     * to be seen as atomic, even if they happen via multiple events.
     */
    void done();

    /**
     * This virtual desktop has just been removed by the server:
     * This object itself is about to be deleted. All windows will
     * lose the association to this desktop.
     */
    void removed();

private:
    friend class PlasmaVirtualDesktopManagement;
    explicit PlasmaVirtualDesktop(QObject *parent = nullptr);
    class Private;
    QScopedPointer<Private> d;
};


}
}

#endif
