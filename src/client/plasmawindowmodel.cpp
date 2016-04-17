/********************************************************************
Copyright 2015  Eike Hein <hein@kde.org>

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
#include "plasmawindowmodel.h"
#include "plasmawindowmanagement.h"

#include <QMetaEnum>

namespace KWayland
{
namespace Client
{

class PlasmaWindowModel::Private
{
public:
    Private(PlasmaWindowModel *q);
    QList<PlasmaWindow*> windows;
    PlasmaWindow *window = nullptr;

    void addWindow(PlasmaWindow *window);
    void dataChanged(PlasmaWindow *window, int role);

private:
    PlasmaWindowModel *q;
};

PlasmaWindowModel::Private::Private(PlasmaWindowModel *q)
    : q(q)
{
}

void PlasmaWindowModel::Private::addWindow(PlasmaWindow *window)
{
    if (windows.indexOf(window) != -1) {
        return;
    }

    const int count = windows.count();
    q->beginInsertRows(QModelIndex(), count, count);
    windows.append(window);
    q->endInsertRows();

    QObject::connect(window, &PlasmaWindow::unmapped,
        [window, this] {
            const int row = windows.indexOf(window);
            if (row != -1) {
                q->beginRemoveRows(QModelIndex(), row, row);
                windows.removeAt(row);
                q->endRemoveRows();
            }
        }
    );

    QObject::connect(window, &PlasmaWindow::titleChanged,
        [window, this] { this->dataChanged(window, Qt::DisplayRole); }
    );

    QObject::connect(window, &PlasmaWindow::iconChanged,
        [window, this] { this->dataChanged(window, Qt::DecorationRole); }
    );

    QObject::connect(window, &PlasmaWindow::appIdChanged,
        [window, this] { this->dataChanged(window, PlasmaWindowModel::AppId); }
    );

    QObject::connect(window, &PlasmaWindow::activeChanged,
        [window, this] { this->dataChanged(window, IsActive); }
    );

    QObject::connect(window, &PlasmaWindow::fullscreenableChanged,
        [window, this] { this->dataChanged(window, IsFullscreenable); }
    );

    QObject::connect(window, &PlasmaWindow::fullscreenChanged,
        [window, this] { this->dataChanged(window, IsFullscreen); }
    );

    QObject::connect(window, &PlasmaWindow::maximizeableChanged,
        [window, this] { this->dataChanged(window, IsMaximizable); }
    );

    QObject::connect(window, &PlasmaWindow::maximizedChanged,
        [window, this] { this->dataChanged(window, IsMaximized); }
    );

    QObject::connect(window, &PlasmaWindow::minimizeableChanged,
        [window, this] { this->dataChanged(window, IsMinimizable); }
    );

    QObject::connect(window, &PlasmaWindow::minimizedChanged,
        [window, this] { this->dataChanged(window, IsMinimized); }
    );

    QObject::connect(window, &PlasmaWindow::keepAboveChanged,
        [window, this] { this->dataChanged(window, IsKeepAbove); }
    );

    QObject::connect(window, &PlasmaWindow::keepBelowChanged,
        [window, this] { this->dataChanged(window, IsKeepBelow); }
    );

    QObject::connect(window, &PlasmaWindow::virtualDesktopChanged,
        [window, this] { this->dataChanged(window, VirtualDesktop); }
    );

    QObject::connect(window, &PlasmaWindow::onAllDesktopsChanged,
        [window, this] { this->dataChanged(window, IsOnAllDesktops); }
    );

    QObject::connect(window, &PlasmaWindow::demandsAttentionChanged,
        [window, this] { this->dataChanged(window, IsDemandingAttention); }
    );

    QObject::connect(window, &PlasmaWindow::skipTaskbarChanged,
        [window, this] { this->dataChanged(window, SkipTaskbar); }
    );

    QObject::connect(window, &PlasmaWindow::shadableChanged,
        [window, this] { this->dataChanged(window, IsShadable); }
    );

    QObject::connect(window, &PlasmaWindow::shadedChanged,
        [window, this] { this->dataChanged(window, IsShaded); }
    );
}

void PlasmaWindowModel::Private::dataChanged(PlasmaWindow *window, int role)
{
    QModelIndex idx = q->index(windows.indexOf(window));
    emit q->dataChanged(idx, idx, QVector<int>() << role);
}

PlasmaWindowModel::PlasmaWindowModel(PlasmaWindowManagement *parent)
    : QAbstractListModel(parent)
    , d(new Private(this))
{
    connect(parent, &PlasmaWindowManagement::interfaceAboutToBeReleased,
        [this] {
            beginResetModel();
            qDeleteAll(d->windows);
            d->windows.clear();
            endResetModel();
        }
    );

    connect(parent, &PlasmaWindowManagement::windowCreated,
        [this](PlasmaWindow *window) {
            d->addWindow(window);
        }
    );

    for (auto it = parent->windows().constBegin(); it != parent->windows().constEnd(); ++it) {
        d->addWindow(*it);
    }
}

PlasmaWindowModel::~PlasmaWindowModel()
{
}

QHash<int, QByteArray> PlasmaWindowModel::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles.insert(Qt::DisplayRole, "DisplayRole");
    roles.insert(Qt::DecorationRole, "DecorationRole");

    QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("AdditionalRoles"));

    for (int i = 0; i < e.keyCount(); ++i) {
        roles.insert(e.value(i), e.key(i));
    }

    return roles;
}

QVariant PlasmaWindowModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= d->windows.count()) {
        return QVariant();
    }

    const PlasmaWindow *window = d->windows.at(index.row());

    if (role == Qt::DisplayRole) {
        return window->title();
    } else if (role == Qt::DecorationRole) {
        return window->icon();
    } else if (role == AppId) {
        return window->appId();
    } else if (role == IsActive) {
        return window->isActive();
    } else if (role == IsFullscreenable) {
        return window->isFullscreenable();
    } else if (role == IsFullscreen) {
        return window->isFullscreen();
    } else if (role == IsMaximizable) {
        return window->isMaximizeable();
    } else if (role == IsMaximized) {
        return window->isMaximized();
    } else if (role == IsMinimizable) {
        return window->isMinimizeable();
    } else if (role == IsMinimized) {
        return window->isMinimized();
    } else if (role == IsKeepAbove) {
        return window->isKeepAbove();
    } else if (role == IsKeepBelow) {
        return window->isKeepBelow();
    } else if (role == VirtualDesktop) {
        return window->virtualDesktop();
    } else if (role == IsOnAllDesktops) {
        return window->isOnAllDesktops();
    } else if (role == IsDemandingAttention) {
        return window->isDemandingAttention();
    } else if (role == SkipTaskbar) {
        return window->skipTaskbar();
    } else if (role == IsShadable) {
        return window->isShadable();
    } else if (role == IsShaded) {
        return window->isShaded();
    }

    return QVariant();
}

int PlasmaWindowModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : d->windows.count();
}

QModelIndex PlasmaWindowModel::index(int row, int column, const QModelIndex &parent) const
{
    return hasIndex(row, column, parent) ? createIndex(row, column, d->windows.at(row)) : QModelIndex();
}

Q_INVOKABLE void PlasmaWindowModel::requestActivate(int row)
{
    if (row >= 0 && row < d->windows.count()) {
        d->windows.at(row)->requestActivate();
    }
}

Q_INVOKABLE void PlasmaWindowModel::requestClose(int row)
{
    if (row >= 0 && row < d->windows.count()) {
        d->windows.at(row)->requestClose();
    }
}

Q_INVOKABLE void PlasmaWindowModel::requestVirtualDesktop(int row, quint32 desktop)
{
    if (row >= 0 && row < d->windows.count()) {
        d->windows.at(row)->requestVirtualDesktop(desktop);
    }
}

Q_INVOKABLE void PlasmaWindowModel::requestToggleMinimized(int row)
{
    if (row >= 0 && row < d->windows.count()) {
        d->windows.at(row)->requestToggleMinimized();
    }
}

Q_INVOKABLE void PlasmaWindowModel::requestToggleMaximized(int row)
{
    if (row >= 0 && row < d->windows.count()) {
        d->windows.at(row)->requestToggleMaximized();
    }
}

Q_INVOKABLE void PlasmaWindowModel::setMinimizedGeometry(int row, Surface *panel, const QRect &geom)
{
    if (row >= 0 && row < d->windows.count()) {
        d->windows.at(row)->setMinimizedGeometry(panel, geom);
    }
}

Q_INVOKABLE void PlasmaWindowModel::requestToggleShaded(int row)
{
    if (row >= 0 && row < d->windows.count()) {
        d->windows.at(row)->requestToggleShaded();
    }
}

}
}
