/********************************************************************
Copyright 2015 Marco Martin <mart@kde.org>
Copyright 2015 Martin Gräßlin <mgraesslin@kde.org>

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

#include "plasmaeffects.h"
#include "event_queue.h"
#include "surface.h"
#include "region.h"
#include "output.h"
#include "wayland_pointer_p.h"

#include <wayland-plasma-effects-client-protocol.h>

namespace KWayland
{

namespace Client
{

class PlasmaEffects::Private
{
public:
    Private() = default;

    WaylandPointer<org_kde_plasma_effects, org_kde_plasma_effects_destroy> effects;
    EventQueue *queue = nullptr;
};

PlasmaEffects::PlasmaEffects(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

PlasmaEffects::~PlasmaEffects()
{
    release();
}

void PlasmaEffects::release()
{
    d->effects.release();
}

void PlasmaEffects::destroy()
{
    d->effects.destroy();
}

bool PlasmaEffects::isValid() const
{
    return d->effects.isValid();
}

void PlasmaEffects::setup(org_kde_plasma_effects *effects)
{
    Q_ASSERT(effects);
    Q_ASSERT(!d->effects);
    d->effects.setup(effects);
}

void PlasmaEffects::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *PlasmaEffects::eventQueue()
{
    return d->queue;
}

void PlasmaEffects::slide(Output *output, Surface *surface, Location from, int x, int y)
{
    Q_ASSERT(isValid());
    Q_ASSERT(surface);

    org_kde_plasma_effects_slide(d->effects, *output, *surface, (org_kde_plasma_effects_location)from, x, y);
}

void PlasmaEffects::setBlurBehindRegion(Surface *surface, const Region *region)
{
    Q_ASSERT(isValid());
    Q_ASSERT(surface);

    if (region) {
        org_kde_plasma_effects_set_blur_behind_region(d->effects, *surface, *region);
    } else {
        org_kde_plasma_effects_set_blur_behind_region(d->effects, *surface, nullptr);
    }
}

void PlasmaEffects::setContrastRegion(Surface *surface, const Region *region, int contrast, int intensity, int saturation)
{
    Q_ASSERT(isValid());
    Q_ASSERT(surface);

    if (region) {
        org_kde_plasma_effects_set_contrast_region(d->effects, *surface, *region, contrast, intensity, saturation);
    } else {
        org_kde_plasma_effects_set_contrast_region(d->effects, *surface, nullptr, contrast, intensity, saturation);
    }
}

PlasmaEffects::operator org_kde_plasma_effects*()
{
    return d->effects;
}

PlasmaEffects::operator org_kde_plasma_effects*() const
{
    return d->effects;
}

}
}
