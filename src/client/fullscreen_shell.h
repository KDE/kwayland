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

#include <wayland-client-fullscreen-shell.h>
#include <wayland-client-protocol.h>

#include <kwaylandclient_export.h>

namespace KWayland
{
namespace Client
{

class Surface;
class Output;

class KWAYLANDCLIENT_EXPORT FullscreenShell : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool capabilityArbitraryModes READ hasCapabilityArbitraryModes NOTIFY capabilityArbitraryModesChanged)
    Q_PROPERTY(bool capabilityCursorPlane READ hasCapabilityCursorPlane NOTIFY capabilityCursorPlaneChanged)
public:
    explicit FullscreenShell(QObject *parent = nullptr);
    virtual ~FullscreenShell();

    bool isValid() const {
        return m_shell != nullptr;
    }
    void release();
    void destroy();
    bool hasCapabilityArbitraryModes() const {
        return m_capabilityArbitraryModes;
    }
    bool hasCapabilityCursorPlane() const {
        return m_capabilityCursorPlane;
    }
    void setup(_wl_fullscreen_shell *shell);
    void present(wl_surface *surface, wl_output *output);
    void present(Surface *surface, Output *output);

    static void capabilitiesAnnounce(void *data, struct _wl_fullscreen_shell *shell, uint32_t capability);

Q_SIGNALS:
    void capabilityArbitraryModesChanged(bool);
    void capabilityCursorPlaneChanged(bool);

private:
    void handleCapabilities(uint32_t capability);
    _wl_fullscreen_shell *m_shell;
    bool m_capabilityArbitraryModes;
    bool m_capabilityCursorPlane;
    static _wl_fullscreen_shell_listener s_fullscreenShellListener;
};

}
}

#endif
