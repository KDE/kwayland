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
#ifndef WAYLAND_SERVER_GLOBAL_H
#define WAYLAND_SERVER_GLOBAL_H

#include <QObject>

#include <KWayland/Server/kwaylandserver_export.h>

struct wl_global;

namespace KWayland
{
namespace Server
{
class Display;

/**
 * @brief Base class for all Globals.
 *
 * Any class representing a Global should be derived from this base class.
 * This class provides common functionality for all globals. A global is an
 * object listed as an interface on the registry on client side.
 *
 * Normally a Global gets factored by the Display. For each Global-derived class there
 * is a dedicated factory method. After creating an instance through the factory method
 * it is not yet announced on the registry. One needs to call ::create on it. This allows
 * to setup the Global before it gets announced, ensuring that the client's state is correct
 * from the start.
 *
 * As an example shown for @link OutputInterface @endlink:
 * @code
 * Display *display; // The existing display
 * auto o = display->createOutput();
 * o->setManufacturer(QStringLiteral("The KDE Community"));
 * // setup further data on the OutputInterface
 * o->create(); // announces OutputInterface
 * @endcode
 *
 * @see Display
 *
 **/
class KWAYLANDSERVER_EXPORT Global : public QObject
{
    Q_OBJECT
public:
    virtual ~Global();
    /**
     * Creates the global by creating a native wl_global and by that announcing it
     * to the clients.
     **/
    void create();
    /**
     * Destroys the low level wl_global. Afterwards the Global is no longer shown to clients.
     **/
    void destroy();
    /**
     * @returns whether the Global got created
     **/
    bool isValid() const;

    /**
     * @returns the Display the Global got created on.
     */
    Display *display();

    /**
     * Cast operator to the native wl_global this Global represents.
     **/
    operator wl_global*();
    /**
     * Cast operator to the native wl_global this Global represents.
     **/
    operator wl_global*() const;

protected:
    class Private;
    explicit Global(Private *d, QObject *parent = nullptr);
    QScopedPointer<Private> d;
};

}
}

#endif
