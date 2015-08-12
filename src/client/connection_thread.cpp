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
#include "connection_thread.h"
#include "logging_p.h"
// Qt
#include <QAbstractEventDispatcher>
#include <QGuiApplication>
#include <QDebug>
#include <QDir>
#include <QFileSystemWatcher>
#include <QSocketNotifier>
#include <qpa/qplatformnativeinterface.h>
// Wayland
#include <wayland-client-protocol.h>

namespace KWayland
{

namespace Client
{

class ConnectionThread::Private
{
public:
    Private(ConnectionThread *q);
    ~Private();
    void doInitConnection();
    void setupSocketNotifier();
    void setupSocketFileWatcher();

    wl_display *display = nullptr;
    int fd = -1;
    QString socketName;
    QDir runtimeDir;
    QScopedPointer<QSocketNotifier> socketNotifier;
    QScopedPointer<QFileSystemWatcher> socketWatcher;
    bool serverDied = false;
    bool foreign = false;
private:
    ConnectionThread *q;
};

ConnectionThread::Private::Private(ConnectionThread *q)
    : socketName(QString::fromUtf8(qgetenv("WAYLAND_DISPLAY")))
    , runtimeDir(QString::fromUtf8(qgetenv("XDG_RUNTIME_DIR")))
    , q(q)
{
    if (socketName.isEmpty()) {
        socketName = QStringLiteral("wayland-0");
    }
}

ConnectionThread::Private::~Private()
{
    if (display && !foreign) {
        wl_display_flush(display);
        wl_display_disconnect(display);
    }
}

void ConnectionThread::Private::doInitConnection()
{
    if (fd != -1) {
        display = wl_display_connect_to_fd(fd);
    } else {
        display = wl_display_connect(socketName.toUtf8().constData());
    }
    if (!display) {
        qCWarning(KWAYLAND_CLIENT) << "Failed connecting to Wayland display";
        emit q->failed();
        return;
    }
    if (fd != -1) {
        qCDebug(KWAYLAND_CLIENT) << "Connected to Wayland server over file descriptor:" << fd;
    } else {
        qCDebug(KWAYLAND_CLIENT) << "Connected to Wayland server at:" << socketName;
    }

    // setup socket notifier
    setupSocketNotifier();
    setupSocketFileWatcher();
    emit q->connected();
}

void ConnectionThread::Private::setupSocketNotifier()
{
    const int fd = wl_display_get_fd(display);
    socketNotifier.reset(new QSocketNotifier(fd, QSocketNotifier::Read));
    QObject::connect(socketNotifier.data(), &QSocketNotifier::activated, q,
        [this]() {
            if (!display) {
                return;
            }
            wl_display_dispatch(display);
            emit q->eventsRead();
        }
    );
}

void ConnectionThread::Private::setupSocketFileWatcher()
{
    if (!runtimeDir.exists() || fd != -1) {
        return;
    }
    socketWatcher.reset(new QFileSystemWatcher);
    socketWatcher->addPath(runtimeDir.absoluteFilePath(socketName));
    QObject::connect(socketWatcher.data(), &QFileSystemWatcher::fileChanged, q,
        [this] (const QString &file) {
            if (QFile::exists(file) || serverDied) {
                return;
            }
            qCWarning(KWAYLAND_CLIENT) << "Connection to server went away";
            serverDied = true;
            if (display) {
                free(display);
                display = nullptr;
            }
            socketNotifier.reset();

            // need a new filesystem watcher
            socketWatcher.reset(new QFileSystemWatcher);
            socketWatcher->addPath(runtimeDir.absolutePath());
            QObject::connect(socketWatcher.data(), &QFileSystemWatcher::directoryChanged, q,
                [this]() {
                    if (!serverDied) {
                        return;
                    }
                    if (runtimeDir.exists(socketName)) {
                        qCDebug(KWAYLAND_CLIENT) << "Socket reappeared";
                        socketWatcher.reset();
                        serverDied = false;
                        q->initConnection();
                    }
                }
            );
            emit q->connectionDied();
        }
    );
}

ConnectionThread::ConnectionThread(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
    connect(QCoreApplication::eventDispatcher(), &QAbstractEventDispatcher::aboutToBlock, this,
        [this] {
            if (d->display) {
                wl_display_flush(d->display);
            }
        },
        Qt::DirectConnection);
}

ConnectionThread::~ConnectionThread() = default;

ConnectionThread *ConnectionThread::fromApplication(QObject *parent)
{
    if (!QGuiApplication::platformName().contains(QStringLiteral("wayland"), Qt::CaseInsensitive)) {
        return nullptr;
    }
    QPlatformNativeInterface *native = qApp->platformNativeInterface();
    if (!native) {
        return nullptr;
    }
    wl_display *display = reinterpret_cast<wl_display*>(native->nativeResourceForIntegration(QByteArrayLiteral("wl_display")));
    if (!display) {
        return nullptr;
    }
    ConnectionThread *ct = new ConnectionThread(parent);
    ct->d->foreign = true;
    ct->d->display = display;
    return ct;
}

void ConnectionThread::initConnection()
{
    QMetaObject::invokeMethod(this, "doInitConnection", Qt::QueuedConnection);
}

void ConnectionThread::doInitConnection()
{
    d->doInitConnection();
}

void ConnectionThread::setSocketName(const QString &socketName)
{
    if (d->display) {
        // already initialized
        return;
    }
    d->socketName = socketName;
}

void ConnectionThread::setSocketFd(int fd)
{
    if (d->display) {
        // already initialized
        return;
    }
    d->fd = fd;
}

wl_display *ConnectionThread::display()
{
    return d->display;
}

QString ConnectionThread::socketName() const
{
    return d->socketName;
}

void ConnectionThread::flush()
{
    if (!d->display) {
        return;
    }
    wl_display_flush(d->display);
}

void ConnectionThread::roundtrip()
{
    if (!d->display) {
        return;
    }
    wl_display_roundtrip(d->display);
}

}
}
