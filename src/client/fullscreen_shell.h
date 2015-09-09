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
#ifndef WAYLAND_FULLSCREEN_SHELL_H
#define WAYLAND_FULLSCREEN_SHELL_H

#include <QObject>

#include <KWayland/Client/kwaylandclient_export.h>

struct _wl_fullscreen_shell;
struct wl_output;
struct wl_surface;

namespace KWayland
{
namespace Client
{

class EventQueue;
class Surface;
class Output;

/**
 * @short Wrapper for the _wl_fullscreen_shell interface.
 *
 * This class provides a convenient wrapper for the _wl_fullscreen_shell interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the FullscreenShell interface:
 * @code
 * FullscreenShell *f = registry->createFullscreenShell(name, version);
 * @endcode
 *
 * This creates the FullscreenShell and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * FullscreenShell *f = new FullscreenShell;
 * f->setup(registry->bindFullscreenShell(name, version));
 * @endcode
 *
 * The FullscreenShell can be used as a drop-in replacement for any _wl_fullscreen_shell
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 **/
class KWAYLANDCLIENT_EXPORT FullscreenShell : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool capabilityArbitraryModes READ hasCapabilityArbitraryModes NOTIFY capabilityArbitraryModesChanged)
    Q_PROPERTY(bool capabilityCursorPlane READ hasCapabilityCursorPlane NOTIFY capabilityCursorPlaneChanged)
public:
    explicit FullscreenShell(QObject *parent = nullptr);
    virtual ~FullscreenShell();

    bool isValid() const;
    void release();
    void destroy();
    bool hasCapabilityArbitraryModes() const;
    bool hasCapabilityCursorPlane() const;
    void setup(_wl_fullscreen_shell *shell);
    void present(wl_surface *surface, wl_output *output);
    void present(Surface *surface, Output *output);

    /**
     * Sets the @p queue to use for bound proxies.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for bound proxies.
     **/
    EventQueue *eventQueue() const;

Q_SIGNALS:
    void capabilityArbitraryModesChanged(bool);
    void capabilityCursorPlaneChanged(bool);

    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the Compositor got created by
     * Registry::createFullscreenShell
     *
     * @since 5.5
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
