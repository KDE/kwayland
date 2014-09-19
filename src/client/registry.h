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
#ifndef WAYLAND_REGISTRY_H
#define WAYLAND_REGISTRY_H

#include <QHash>
#include <QObject>

#include <KWayland/Client/kwaylandclient_export.h>

struct wl_compositor;
struct wl_display;
struct wl_output;
struct wl_registry;
struct wl_seat;
struct wl_shell;
struct wl_shm;
struct _wl_fullscreen_shell;

namespace KWayland
{
namespace Client
{

class Compositor;
class ConnectionThread;
class FullscreenShell;
class Output;
class Seat;
class Shell;
class ShmPool;

class KWAYLANDCLIENT_EXPORT Registry : public QObject
{
    Q_OBJECT
public:
    enum class Interface {
        Compositor, // wl_compositor
        Shell,      // wl_shell
        Seat,       // wl_seat
        Shm,        // wl_shm
        Output,     // wl_output
        FullscreenShell, // _wl_fullscreen_shell
        Unknown
    };
    explicit Registry(QObject *parent = nullptr);
    virtual ~Registry();

    void release();
    void destroy();
    void create(wl_display *display);
    void create(ConnectionThread *connection);
    void setup();

    bool isValid() const;
    bool hasInterface(Interface interface) const;

    wl_compositor *bindCompositor(uint32_t name, uint32_t version) const;
    wl_shell *bindShell(uint32_t name, uint32_t version) const;
    wl_seat *bindSeat(uint32_t name, uint32_t version) const;
    wl_shm *bindShm(uint32_t name, uint32_t version) const;
    wl_output *bindOutput(uint32_t name, uint32_t version) const;
    _wl_fullscreen_shell *bindFullscreenShell(uint32_t name, uint32_t version) const;

    Compositor *createCompositor(quint32 name, quint32 version, QObject *parent = nullptr);
    Shell *createShell(quint32 name, quint32 version, QObject *parent = nullptr);
    Seat *createSeat(quint32 name, quint32 version, QObject *parent = nullptr);
    ShmPool *createShmPool(quint32 name, quint32 version, QObject *parent = nullptr);
    Output *createOutput(quint32 name, quint32 version, QObject *parent = nullptr);
    FullscreenShell *createFullscreenShell(quint32 name, quint32 version, QObject *parent = nullptr);

    operator wl_registry*();
    operator wl_registry*() const;
    wl_registry *registry();

Q_SIGNALS:
    void compositorAnnounced(quint32 name, quint32 version);
    void shellAnnounced(quint32 name, quint32 version);
    void seatAnnounced(quint32 name, quint32 version);
    void shmAnnounced(quint32 name, quint32 version);
    void outputAnnounced(quint32 name, quint32 version);
    void fullscreenShellAnnounced(quint32 name, quint32 version);
    void compositorRemoved(quint32 name);
    void shellRemoved(quint32 name);
    void seatRemoved(quint32 name);
    void shmRemoved(quint32 name);
    void outputRemoved(quint32 name);
    void fullscreenShellRemoved(quint32 name);
    void interfaceAnnounced(QByteArray interface, quint32 name, quint32 version);
    void interfaceRemoved(quint32 name);

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
