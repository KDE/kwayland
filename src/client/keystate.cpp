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
#include "keystate.h"
#include "wayland_pointer_p.h"
#include <QPointer>
#include <QDebug>
#include <wayland-client-protocol.h>
#include <wayland-keystate-client-protocol.h>

namespace KWayland
{
namespace Client
{

class Q_DECL_HIDDEN Keystate::Private
{
public:
    Private() {}

    WaylandPointer<org_kde_kwin_keystate, org_kde_kwin_keystate_destroy> keystate;

    static void org_kde_kwin_keystate_stateChanged(void *data, struct org_kde_kwin_keystate */*keystate*/, uint32_t k, uint32_t s) {
        auto q = static_cast<Keystate*>(data);
        q->stateChanged(Key(k), State(s));
    }

    static const org_kde_kwin_keystate_listener s_listener;
};

const org_kde_kwin_keystate_listener Keystate::Private::s_listener = {
    org_kde_kwin_keystate_stateChanged
};

Keystate::Keystate(QObject *parent)
    : QObject(parent)
    , d(new Private())
{
}

Keystate::~Keystate() = default;

void Keystate::fetchStates()
{
    org_kde_kwin_keystate_fetchStates(d->keystate);
}

void Keystate::setup(org_kde_kwin_keystate* keystate)
{
    d->keystate.setup(keystate);
    org_kde_kwin_keystate_add_listener(keystate, &Keystate::Private::s_listener, this);
}

void Keystate::destroy()
{
    d->keystate.destroy();
}

void Keystate::setEventQueue(KWayland::Client::EventQueue* /*queue*/)
{
}


}
}
