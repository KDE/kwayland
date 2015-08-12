/********************************************************************
Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>

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
#ifndef WAYLANDCLIENTTEST_H
#define WAYLANDCLIENTTEST_H

#include <QObject>
#include <QSize>

namespace KWayland
{
namespace Client
{
class Compositor;
class ConnectionThread;
class Registry;
class ShellSurface;
class ShmPool;
class Surface;
}
}

class QTimer;

class WaylandClientTest : public QObject
{
    Q_OBJECT
public:
    explicit WaylandClientTest(QObject *parent = nullptr);
    virtual ~WaylandClientTest();

private:
    void init();
    void render(const QSize &size);
    void render();
    void setupRegistry(KWayland::Client::Registry *registry);
    void toggleTimer();
    KWayland::Client::ConnectionThread *m_connectionThreadObject;
    KWayland::Client::Compositor *m_compositor;
    KWayland::Client::Surface *m_surface;
    KWayland::Client::ShmPool *m_shm;
    KWayland::Client::ShellSurface *m_shellSurface;
    QSize m_currentSize;
    QTimer *m_timer;
};

#endif
