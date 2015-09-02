/********************************************************************
Copyright 2015  Martin Gräßlin <mgraesslin@kde.org>
Copyright 2015  Marco Martin <mart@kde.org>

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
#include "contrast.h"
#include "event_queue.h"
#include "region.h"
#include "surface.h"
#include "wayland_pointer_p.h"

#include <wayland-contrast-client-protocol.h>

namespace KWayland
{

namespace Client
{

class ContrastManager::Private
{
public:
    Private() = default;

    WaylandPointer<org_kde_kwin_contrast_manager, org_kde_kwin_contrast_manager_destroy> manager;
    EventQueue *queue = nullptr;
};

ContrastManager::ContrastManager(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

ContrastManager::~ContrastManager()
{
    release();
}

void ContrastManager::release()
{
    d->manager.release();
}

void ContrastManager::destroy()
{
    d->manager.destroy();
}

bool ContrastManager::isValid() const
{
    return d->manager.isValid();
}

void ContrastManager::setup(org_kde_kwin_contrast_manager *manager)
{
    Q_ASSERT(manager);
    Q_ASSERT(!d->manager);
    d->manager.setup(manager);
}

void ContrastManager::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *ContrastManager::eventQueue()
{
    return d->queue;
}

Contrast *ContrastManager::createContrast(Surface *surface, QObject *parent)
{
    Q_ASSERT(isValid());
    Contrast *s = new Contrast(parent);
    auto w = org_kde_kwin_contrast_manager_create(d->manager, *surface);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    s->setup(w);
    return s;
}

void ContrastManager::removeContrast(Surface *surface)
{
    Q_ASSERT(isValid());
    org_kde_kwin_contrast_manager_unset(d->manager, *surface);
}

ContrastManager::operator org_kde_kwin_contrast_manager*()
{
    return d->manager;
}

ContrastManager::operator org_kde_kwin_contrast_manager*() const
{
    return d->manager;
}

class Contrast::Private
{
public:
    WaylandPointer<org_kde_kwin_contrast, org_kde_kwin_contrast_destroy> contrast;
};

Contrast::Contrast(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

Contrast::~Contrast()
{
    release();
}

void Contrast::release()
{
    d->contrast.release();
}

void Contrast::setup(org_kde_kwin_contrast *contrast)
{
    Q_ASSERT(contrast);
    Q_ASSERT(!d->contrast);
    d->contrast.setup(contrast);
}

void Contrast::destroy()
{
    d->contrast.destroy();
}

bool Contrast::isValid() const
{
    return d->contrast.isValid();
}

void Contrast::commit()
{
    Q_ASSERT(isValid());
    org_kde_kwin_contrast_commit(d->contrast);
}

void Contrast::setRegion(Region *region)
{
    org_kde_kwin_contrast_set_region(d->contrast, *region);
}

void Contrast::setContrast(qreal contrast)
{
    org_kde_kwin_contrast_set_contrast(d->contrast, wl_fixed_from_double(contrast));
}

void Contrast::setIntensity(qreal intensity)
{
    org_kde_kwin_contrast_set_intensity(d->contrast, wl_fixed_from_double(intensity));
}

void Contrast::setSaturation(qreal saturation)
{
    org_kde_kwin_contrast_set_saturation(d->contrast, wl_fixed_from_double(saturation));
}

Contrast::operator org_kde_kwin_contrast*()
{
    return d->contrast;
}

Contrast::operator org_kde_kwin_contrast*() const
{
    return d->contrast;
}

}
}
