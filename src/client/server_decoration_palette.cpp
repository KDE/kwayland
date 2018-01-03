/****************************************************************************
Copyright 2017  David Edmundson <kde@davidedmundson.co.uk>

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
****************************************************************************/
#include "server_decoration_palette.h"
#include "event_queue.h"
#include "surface.h"
#include "wayland_pointer_p.h"

#include <wayland-server-decoration-palette-client-protocol.h>

#include <QDebug>

namespace KWayland
{
namespace Client
{

class ServerSideDecorationPaletteManager::Private
{
public:
    Private() = default;

    void setup(org_kde_kwin_server_decoration_palette_manager *arg);

    WaylandPointer<org_kde_kwin_server_decoration_palette_manager, org_kde_kwin_server_decoration_palette_manager_destroy> serverdecomanager;
    EventQueue *queue = nullptr;
};

ServerSideDecorationPaletteManager::ServerSideDecorationPaletteManager(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

void ServerSideDecorationPaletteManager::Private::setup(org_kde_kwin_server_decoration_palette_manager *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!serverdecomanager);
    serverdecomanager.setup(arg);
}

ServerSideDecorationPaletteManager::~ServerSideDecorationPaletteManager()
{
    release();
}

void ServerSideDecorationPaletteManager::setup(org_kde_kwin_server_decoration_palette_manager *serverdecomanager)
{
    d->setup(serverdecomanager);
}

void ServerSideDecorationPaletteManager::release()
{
    d->serverdecomanager.release();
}

void ServerSideDecorationPaletteManager::destroy()
{
    d->serverdecomanager.destroy();
}

ServerSideDecorationPaletteManager::operator org_kde_kwin_server_decoration_palette_manager*() {
    return d->serverdecomanager;
}

ServerSideDecorationPaletteManager::operator org_kde_kwin_server_decoration_palette_manager*() const {
    return d->serverdecomanager;
}

bool ServerSideDecorationPaletteManager::isValid() const
{
    return d->serverdecomanager.isValid();
}

void ServerSideDecorationPaletteManager::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *ServerSideDecorationPaletteManager::eventQueue()
{
    return d->queue;
}

ServerDecorationPalette *ServerSideDecorationPaletteManager::create(Surface *surface, QObject *parent)
{
    Q_ASSERT(isValid());
    auto p = new ServerDecorationPalette(parent);
    auto w = org_kde_kwin_server_decoration_palette_manager_create(d->serverdecomanager, *surface);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    p->setup(w);

    return p;
}

class ServerDecorationPalette::Private
{
public:
    Private(ServerDecorationPalette *q);

    void setup(org_kde_kwin_server_decoration_palette *arg);

    WaylandPointer<org_kde_kwin_server_decoration_palette, org_kde_kwin_server_decoration_palette_release> decoration_palette;

private:
    ServerDecorationPalette *q;
};

ServerDecorationPalette::Private::Private(ServerDecorationPalette *q)
    : q(q)
{
}

ServerDecorationPalette::ServerDecorationPalette(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

void ServerDecorationPalette::Private::setup(org_kde_kwin_server_decoration_palette *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!decoration_palette);
    decoration_palette.setup(arg);
}

ServerDecorationPalette::~ServerDecorationPalette()
{
    release();
}

void ServerDecorationPalette::setup(org_kde_kwin_server_decoration_palette *decoration_palette)
{
    d->setup(decoration_palette);
}

void ServerDecorationPalette::release()
{
    d->decoration_palette.release();
}

void ServerDecorationPalette::destroy()
{
    d->decoration_palette.destroy();
}

ServerDecorationPalette::operator org_kde_kwin_server_decoration_palette*() {
    return d->decoration_palette;
}

ServerDecorationPalette::operator org_kde_kwin_server_decoration_palette*() const {
    return d->decoration_palette;
}

bool ServerDecorationPalette::isValid() const
{
    return d->decoration_palette.isValid();
}

void ServerDecorationPalette::setPalette(const QString &palette)
{
    Q_ASSERT(isValid());
    org_kde_kwin_server_decoration_palette_set_palette(*this, palette.toUtf8());
}
}
}
