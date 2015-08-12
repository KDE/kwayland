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
#ifndef WAYLAND_SERVER_PLASMA_WINDOW_MANAGEMENT_INTERFACE_H
#define WAYLAND_SERVER_PLASMA_WINDOW_MANAGEMENT_INTERFACE_H

#include <QObject>

#include <KWayland/Server/kwaylandserver_export.h>

#include "global.h"
#include "resource.h"

class QSize;

namespace KWayland
{
namespace Server
{

class Display;
class PlasmaWindowInterface;

class KWAYLANDSERVER_EXPORT PlasmaWindowManagementInterface : public Global
{
    Q_OBJECT
public:
    virtual ~PlasmaWindowManagementInterface();
    enum class ShowingDesktopState {
        Disabled,
        Enabled
    };
    void setShowingDesktopState(ShowingDesktopState state);

    PlasmaWindowInterface *createWindow(QObject *parent);
    QList<PlasmaWindowInterface*> windows() const;

Q_SIGNALS:
    void requestChangeShowingDesktop(ShowingDesktopState requestedState);

private:
    friend class Display;
    explicit PlasmaWindowManagementInterface(Display *display, QObject *parent);
    class Private;
    Private *d_func() const;
};

class KWAYLANDSERVER_EXPORT PlasmaWindowInterface : public QObject
{
    Q_OBJECT
public:
    virtual ~PlasmaWindowInterface();

    void setTitle(const QString &title);
    void setAppId(const QString &appId);
    void setVirtualDesktop(quint32 desktop);
    void setActive(bool set);
    void setMinimized(bool set);
    void setMaximized(bool set);
    void setFullscreen(bool set);
    void setKeepAbove(bool set);
    void setKeepBelow(bool set);
    void setOnAllDesktops(bool set);
    void setDemandsAttention(bool set);
    void setCloseable(bool set);
    void setMinimizeable(bool set);
    void setMaximizeable(bool set);
    void setFullscreenable(bool set);
    void setThemedIconName(const QString &iconName);

    void unmap();

Q_SIGNALS:
    void closeRequested();
    void virtualDesktopRequested(quint32 desktop);
    void activeRequested(bool set);
    void minimizedRequested(bool set);
    void maximizedRequested(bool set);
    void fullscreenRequested(bool set);
    void keepAboveRequested(bool set);
    void keepBelowRequested(bool set);
    void demandsAttentionRequested(bool set);
    void closeableRequested(bool set);
    void minimizeableRequested(bool set);
    void maximizeableRequested(bool set);
    void fullscreenableRequested(bool set);

private:
    friend class PlasmaWindowManagementInterface;
    explicit PlasmaWindowInterface(PlasmaWindowManagementInterface *wm, QObject *parent);

    class Private;
    const QScopedPointer<Private> d;
};

}
}

#endif
