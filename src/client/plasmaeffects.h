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
#ifndef KWAYLAND_PLASMAEFFECTS_H
#define KWAYLAND_PLASMAEFFECTS_H

#include "buffer.h"

#include <QObject>
#include <QPoint>
#include <QSize>

#include <KWayland/Client/kwaylandclient_export.h>

struct wl_buffer;
struct org_kde_plasma_effects;

class QMarginsF;
class QWindow;

namespace KWayland
{
namespace Client
{

class EventQueue;
class Surface;
class Region;
class Output;

class KWAYLANDCLIENT_EXPORT PlasmaEffects : public QObject
{
    Q_OBJECT

public:
    enum Location {
        None = 0,
        Left = 1,
        Top = 2,
        Right = 3,
        Bottom = 4
    };
    Q_ENUMS(Location)

    explicit PlasmaEffects(QObject *parent = nullptr);
    virtual ~PlasmaEffects();

    /**
     * @returns @c true if managing a org_kde_plasma_effects.
     **/
    bool isValid() const;
    /**
     * Setup this PlasmaEffects to manage the @p effects.
     * When using Registry::createPlasmaEffects there is no need to call this
     * method.
     **/
    void setup(org_kde_plasma_effects *effects);
    /**
     * Releases the org_kde_plasma_effects interface.
     * After the interface has been released the PlasmaEffects instance is no
     * longer valid and can be setup with another org_kde_plasma_effects interface.
     **/
    void release();
    /**
     * Destroys the data hold by this PlasmaEffects.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid any more, it's not
     * possible to call release any more as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or setup to a new org_kde_plasma_effects interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, compositor, &PlasmaEffects::destroyed);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating a PlasmaEffects.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating a PlasmaEffects.
     **/
    EventQueue *eventQueue();

    /**
     * Ask the compositor to move the surface from a location to another
     * with a slide animation.
     *
     * The from argument provides a clue about where the slide animation
     * begins, destination coordinates are specified with x and y.
     *
     * @param output on what screen to apply this effect //FIXME: needed?
     * @param surface The surface to apply the effect to
     * @param from The location from which the slide effect will start,
     *             this suggests the direction of the slide animation as well
     * @param x Final destination position of the window //FIXME: we aren't actually animating the window position?
     * @param x Final destination position of the window //FIXME: we aren't actually animating the window position?
     */
    void slide(Output *output, Surface *surface, Location from, int x, int y);

    /**
     * This request sets the region of the surface that will allow to see
     * through with a blur effect.
     * Pass a null region to disable blur behind.
     *
     * @param surface The surface we want apply the blur behind effect to
     * @param region The region of the surface we want apply the blur behind effect to
     */
    void setBlurBehindRegion(Surface *surface, const Region *region = nullptr);

    operator org_kde_plasma_effects*();
    operator org_kde_plasma_effects*() const;

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif

