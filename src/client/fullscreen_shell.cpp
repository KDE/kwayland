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
#include "fullscreen_shell.h"
#include "surface.h"
#include "output.h"
#include "wayland_pointer_p.h"

#include <QDebug>
// wayland
#include <wayland-fullscreen-shell-client-protocol.h>
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{

class Q_DECL_HIDDEN FullscreenShell::Private
{
public:
    Private(FullscreenShell *q);
    void setup(_wl_fullscreen_shell *shell);

    WaylandPointer<_wl_fullscreen_shell, _wl_fullscreen_shell_release> shell;
    EventQueue *queue = nullptr;
    bool capabilityArbitraryModes = false;
    bool capabilityCursorPlane = false;

private:
    void handleCapabilities(uint32_t capability);
    static void capabilitiesAnnounce(void *data, struct _wl_fullscreen_shell *shell, uint32_t capability);
    static _wl_fullscreen_shell_listener s_fullscreenShellListener;
    FullscreenShell *q;
};

_wl_fullscreen_shell_listener FullscreenShell::Private::s_fullscreenShellListener = {
    FullscreenShell::Private::capabilitiesAnnounce
};

FullscreenShell::Private::Private(FullscreenShell *q)
    : q(q)
{
}

void FullscreenShell::Private::setup(_wl_fullscreen_shell *s)
{
    Q_ASSERT(!shell);
    Q_ASSERT(s);
    shell.setup(s);
    _wl_fullscreen_shell_add_listener(shell, &s_fullscreenShellListener, this);
}

void FullscreenShell::Private::capabilitiesAnnounce(void *data, _wl_fullscreen_shell *shell, uint32_t capability)
{
    auto s = reinterpret_cast<FullscreenShell::Private*>(data);
    Q_ASSERT(shell == s->shell);
    s->handleCapabilities(capability);
}

void FullscreenShell::Private::handleCapabilities(uint32_t capability)
{
    if (capability & _WL_FULLSCREEN_SHELL_CAPABILITY_ARBITRARY_MODES) {
        capabilityArbitraryModes = true;
        emit q->capabilityArbitraryModesChanged(capabilityArbitraryModes);
    }
    if (capability & _WL_FULLSCREEN_SHELL_CAPABILITY_CURSOR_PLANE) {
        capabilityCursorPlane = true;
        emit q->capabilityCursorPlaneChanged(capabilityCursorPlane);
    }
}

FullscreenShell::FullscreenShell(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

FullscreenShell::~FullscreenShell()
{
    release();
}

void FullscreenShell::release()
{
    d->shell.release();
}

void FullscreenShell::destroy()
{
    d->shell.destroy();
}

void FullscreenShell::setup(_wl_fullscreen_shell *shell)
{
    d->setup(shell);
}

EventQueue *FullscreenShell::eventQueue() const
{
    return d->queue;
}

void FullscreenShell::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

void FullscreenShell::present(wl_surface *surface, wl_output *output)
{
    Q_ASSERT(d->shell);
    _wl_fullscreen_shell_present_surface(d->shell, surface, _WL_FULLSCREEN_SHELL_PRESENT_METHOD_DEFAULT, output);
}

void FullscreenShell::present(Surface *surface, Output *output)
{
    Q_ASSERT(surface);
    Q_ASSERT(output);
    present(*surface, *output);
}

bool FullscreenShell::isValid() const
{
    return d->shell.isValid();
}

bool FullscreenShell::hasCapabilityArbitraryModes() const
{
    return d->capabilityArbitraryModes;
}

bool FullscreenShell::hasCapabilityCursorPlane() const
{
    return d->capabilityCursorPlane;
}

}
}
