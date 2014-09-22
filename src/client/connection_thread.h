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

#include <KWayland/Client/kwaylandclient_export.h>

struct wl_display;

namespace KWayland
{
namespace Client
{

/**
 * @short Creates and manages the connection to a Wayland server.
 *
 * The purpose of this class is to create the connection to a Wayland server
 * and to manage it. As the name suggests it's intended to move instances of
 * this class into a dedicated thread. This also means that this class doesn't
 * inherit QThread. In order to use it in a threaded way one needs to create a
 * QThread and move the object there:
 *
 * @code
 * ConnectionThread *connection = new ConnectionThread;
 * QThread *thread = new QThread;
 * connection->moveToThread(thread);
 * thread->start();
 * @endcode
 *
 * To finalize the initialization of the connection one needs to call @link ::initConnection.
 * This starts an asynchronous connection initialization. In case the initialization
 * succeeds the signal @link ::connected will be emitted, otherwise @link ::failed will
 * be emitted:
 *
 * @code
 * connect(connection, &ConnectionThread::connected, [connection] {
 *     qDebug() << "Successfully connected to Wayland server at socket:" << connection->socketName();
 * });
 * connect(connection, &ConnectionThread::failed, [connection] {
 *     qDebug() << "Failed to connect to Wayland server at socket:" << connection->socketName();
 * });
 * connection->initConnection();
 * @endcode
 *
 * This class is also responsible for dispatching events. Whenever new data is available on
 * the Wayland socket, it will be dispatched and the signal @link ::eventsRead is emitted.
 * This allows further event queues in other threads to also dispatch their events.
 *
 * Furthermore this class flushes the Wayland connection whenever the QAbstractEventDispatcher
 * is about to block.
 *
 * To disconnect the connection to the Wayland server one should delete the instance of this
 * class and quit the dedicated thread:
 *
 * @code
 * connection->deleteLater();
 * thread->quit();
 * thread->wait();
 * @endcode
 *
 **/
class KWAYLANDCLIENT_EXPORT ConnectionThread : public QObject
{
    Q_OBJECT
public:
    explicit ConnectionThread(QObject *parent = nullptr);
    virtual ~ConnectionThread();

    /**
     * The display this ConnectionThread is connected to.
     * As long as there is no connection this method returns @c null.
     * @see initConnection
     **/
    wl_display *display();
    /**
     * @returns the name of the socket it connects to.
     **/
    QString socketName() const;
    /**
     * Sets the @p socketName to connect to.
     * Only applies if called before calling initConnection.
     * The default socket name is derived from environment variable WAYLAND_DISPLAY
     * and if not set is hard coded to "wayland-0".
     **/
    void setSocketName(const QString &socketName);

public Q_SLOTS:
    /**
     * Initializes the connection in an asynchronous way.
     * In case the connection gets established the signal @link ::connected will be
     * emitted, on failure the signal @link ::failed will be emitted.
     *
     * @see connected
     * @see failed
     **/
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
    /**
     * @internal
     **/
    void doInitConnection();

private:
    class Private;
    QScopedPointer<Private> d;
};
}
}


#endif
