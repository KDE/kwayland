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
#ifndef WAYLAND_CONNECTION_THREAD_H
#define WAYLAND_CONNECTION_THREAD_H

#include <QObject>
#include <QDir>

#include <kwaylandclient_export.h>

struct wl_display;
class QFileSystemWatcher;
class QSocketNotifier;

namespace KWayland
{
namespace Client
{

class KWAYLANDCLIENT_EXPORT ConnectionThread : public QObject
{
    Q_OBJECT
public:
    explicit ConnectionThread(QObject *parent = nullptr);
    virtual ~ConnectionThread();

    wl_display *display();
    /**
     * @returns the name of the socket it connects to.
     **/
    const QString &socketName() const;
    /**
     * Sets the @p socketName to connect to.
     * Only applies if called before calling initConnection.
     * The default socket name is derived from environment variable WAYLAND_DISPLAY
     * and if not set is hard coded to "wayland-0".
     **/
    void setSocketName(const QString &socketName);

public Q_SLOTS:
    void initConnection();

Q_SIGNALS:
    /**
     * Emitted once a connection to a Wayland server is established.
     * Normally emitted after invoking initConnection(), but might also be
     * emitted after re-connecting to another server.
     **/
    void connected();
    /**
     * Emitted if connecting to a Wayland server failed.
     **/
    void failed();
    /**
     * Emitted whenever new events are ready to be read.
     **/
    void eventsRead();
    /**
     * Emitted if the Wayland server connection dies.
     * If the socket reappears, it is tried to reconnect.
     **/
    void connectionDied();

private Q_SLOTS:
    void doInitConnection();

private:
    void setupSocketNotifier();
    void setupSocketFileWatcher();
    wl_display *m_display;
    QString m_socketName;
    QDir m_runtimeDir;
    QSocketNotifier *m_socketNotifier;
    QFileSystemWatcher *m_socketWatcher;
    bool m_serverDied;
};

inline
const QString &ConnectionThread::socketName() const
{
    return m_socketName;
}

inline
wl_display *ConnectionThread::display()
{
    return m_display;
}

}
}


#endif
