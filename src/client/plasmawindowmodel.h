/********************************************************************
Copyright 2015  Eike Hein <hein.org>

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
#ifndef WAYLAND_PLASMAWINDOWMODEL_H
#define WAYLAND_PLASMAWINDOWMODEL_H

#include <QAbstractListModel>

#include <KWayland/Client/kwaylandclient_export.h>

namespace KWayland
{
namespace Client
{

class PlasmaWindowManagement;
class Surface;

/**
 * @short Exposes the window list and window state as a Qt item model.
 *
 * This class is a QAbstractListModel implementation that exposes information
 * from a PlasmaWindowManagement instance passed as parent and enables convenient
 * calls to PlasmaWindow methods through a model row index.
 *
 * The model is destroyed when the PlasmaWindowManagement parent is.
 *
 * The model resets when the PlasmaWindowManagement parent signals that its
 * interface is about to be destroyed.
 *
 * To use this class you can create an instance yourself, or preferrably use the
 * convenience method in PlasmaWindowManagement:
 * @code
 * PlasmaWindowModel *model = wm->createWindowModel();
 * @endcode
 *
 * @see PlasmaWindowManagement
 * @see PlasmaWindow
 **/

class KWAYLANDCLIENT_EXPORT PlasmaWindowModel : public QAbstractListModel
{
    Q_OBJECT
    Q_ENUMS(AdditionalRoles)

public:
    enum AdditionalRoles {
        AppId = Qt::UserRole + 1,
        IsActive,
        IsFullscreenable,
        IsFullscreen,
        IsMaximizable,
        IsMaximized,
        IsMinimizable,
        IsMinimized,
        IsKeepAbove,
        IsKeepBelow,
        VirtualDesktop,
        IsOnAllDesktops,
        IsDemandingAttention,
        SkipTaskbar,
        /**
        * @since 5.7
        */
        IsShadeable,
        /**
        * @since 5.7
        */
        IsShaded,
        /**
        * @since 5.7
        */
        IsMovable,
        /**
        * @since 5.7
        */
        IsResizable,
        /**
        * @since 5.7
        */
        IsVirtualDesktopChangeable
    };

    explicit PlasmaWindowModel(PlasmaWindowManagement *parent);
    virtual ~PlasmaWindowModel();

    virtual QHash<int, QByteArray> roleNames() const;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;

    /**
     * Returns an index with internalPointer() pointing to a PlasmaWindow instance.
     **/
    virtual QModelIndex	index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;


    /**
     * Request the window at this model row index be activated.
     **/
    Q_INVOKABLE void requestActivate(int row);

    /**
     * Request the window at this model row index be closed.
     **/
    Q_INVOKABLE void requestClose(int row);

    /**
     * Request an interactive move for the window at this model row index.
     * @since 5.7
     **/
    Q_INVOKABLE void requestMove(int row);

    /**
     * Request an interactive resize for the window at this model row index.
     * @since 5.7
     **/
    Q_INVOKABLE void requestResize(int row);

    /**
     * Request the window at this model row index be moved to this virtual desktop.
     **/
    Q_INVOKABLE void requestVirtualDesktop(int row, quint32 desktop);

    /**
     * Requests the window at this model row index have its minimized state toggled.
     */
    Q_INVOKABLE void requestToggleMinimized(int row);

    /**
     * Requests the window at this model row index have its maximized state toggled.
     */
    Q_INVOKABLE void requestToggleMaximized(int row);

    /**
     * Sets the geometry of the taskbar entry for the window at the model row
     * relative to a panel in particular. QRectF, intended for use from QML
     * @since 5.5
     */
    Q_INVOKABLE void setMinimizedGeometry(int row, Surface *panel, const QRect &geom);

    /**
     * Requests the window at this model row index have its shaded state toggled.
     * @since 5.7
     */
    Q_INVOKABLE void requestToggleShaded(int row);

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
