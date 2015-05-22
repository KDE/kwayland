/********************************************************************
Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
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
#ifndef WAYLAND_SERVER_KWIN_INTERFACE_H
#define WAYLAND_SERVER_KWIN_INTERFACE_H

#include <QObject>
#include <QPoint>
#include <QSize>

#include <KWayland/Server/kwaylandserver_export.h>
#include "global.h"

struct wl_global;
struct wl_client;
struct wl_resource;

namespace KWayland
{
namespace Server
{

class Display;

class KWAYLANDSERVER_EXPORT KWinOutputConnectorsInterface : public Global
{
    Q_OBJECT

public:
    virtual ~KWinOutputConnectorsInterface();

    void getDisabledOutputs();

Q_SIGNALS:
    void outputAppeared(const QString &edid, const QString &name, const QString &connector);
    void outputDisappeared(const QString &name, const QString &connector);
    void sync();

private:
    explicit KWinOutputConnectorsInterface(Display *display, QObject *parent = nullptr);
    friend class Display;
    class Private;
    Private *d_func() const;
};

}
}

#endif
