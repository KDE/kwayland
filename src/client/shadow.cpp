/********************************************************************
Copyright 2015  Martin Gräßlin <mgraesslin@kde.org>

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
#include "shadow.h"
#include "event_queue.h"
#include "surface.h"
#include "wayland_pointer_p.h"

#include <QMarginsF>

#include <wayland-shadow-client-protocol.h>

namespace KWayland
{

namespace Client
{

class ShadowManager::Private
{
public:
    Private() = default;

    WaylandPointer<org_kde_kwin_shadow_manager, org_kde_kwin_shadow_manager_destroy> manager;
    EventQueue *queue = nullptr;
};

ShadowManager::ShadowManager(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

ShadowManager::~ShadowManager()
{
    release();
}

void ShadowManager::release()
{
    d->manager.release();
}

void ShadowManager::destroy()
{
    d->manager.destroy();
}

bool ShadowManager::isValid() const
{
    return d->manager.isValid();
}

void ShadowManager::setup(org_kde_kwin_shadow_manager *manager)
{
    Q_ASSERT(manager);
    Q_ASSERT(!d->manager);
    d->manager.setup(manager);
}

void ShadowManager::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *ShadowManager::eventQueue()
{
    return d->queue;
}

Shadow *ShadowManager::createShadow(Surface *surface, QObject *parent)
{
    Q_ASSERT(isValid());
    Shadow *s = new Shadow(parent);
    auto w = org_kde_kwin_shadow_manager_create(d->manager, *surface);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

void ShadowManager::removeShadow(Surface *surface)
{
    Q_ASSERT(isValid());
    org_kde_kwin_shadow_manager_unset(d->manager, *surface);
}

ShadowManager::operator org_kde_kwin_shadow_manager*()
{
    return d->manager;
}

ShadowManager::operator org_kde_kwin_shadow_manager*() const
{
    return d->manager;
}

class Shadow::Private
{
public:
    WaylandPointer<org_kde_kwin_shadow, org_kde_kwin_shadow_destroy> shadow;
};

Shadow::Shadow(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

Shadow::~Shadow()
{
    release();
}

void Shadow::release()
{
    d->shadow.release();
}

void Shadow::setup(org_kde_kwin_shadow *shadow)
{
    Q_ASSERT(shadow);
    Q_ASSERT(!d->shadow);
    d->shadow.setup(shadow);
}

void Shadow::destroy()
{
    d->shadow.destroy();
}

bool Shadow::isValid() const
{
    return d->shadow.isValid();
}

void Shadow::setOffsets(const QMarginsF &margins)
{
    Q_ASSERT(isValid());
    org_kde_kwin_shadow_set_left_offset(d->shadow, wl_fixed_from_double(margins.left()));
    org_kde_kwin_shadow_set_top_offset(d->shadow, wl_fixed_from_double(margins.top()));
    org_kde_kwin_shadow_set_right_offset(d->shadow, wl_fixed_from_double(margins.right()));
    org_kde_kwin_shadow_set_bottom_offset(d->shadow, wl_fixed_from_double(margins.bottom()));
}

void Shadow::commit()
{
    Q_ASSERT(isValid());
    org_kde_kwin_shadow_commit(d->shadow);
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#define attach( __PART__, __WAYLAND_PART__ ) \
void Shadow::attach##__PART__(wl_buffer *buffer) \
{ \
    Q_ASSERT(isValid()); \
    org_kde_kwin_shadow_attach_##__WAYLAND_PART__(d->shadow, buffer); \
} \
void Shadow::attach##__PART__(Buffer *buffer) \
{ \
    if (!buffer) {\
        return;\
    }\
    attach##__PART__(buffer->buffer()); \
} \
void Shadow::attach##__PART__(Buffer::Ptr buffer) \
{ \
    attach##__PART__(buffer.toStrongRef().data()); \
}

attach(Left, left)
attach(TopLeft, top_left)
attach(Top, top)
attach(TopRight, top_right)
attach(Right, right)
attach(BottomRight, bottom_right)
attach(Bottom, bottom)
attach(BottomLeft, bottom_left)

#undef attach
#endif

Shadow::operator org_kde_kwin_shadow*()
{
    return d->shadow;
}

Shadow::operator org_kde_kwin_shadow*() const
{
    return d->shadow;
}

}
}
