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
#ifndef WAYLAND_COMPOSITOR_H
#define WAYLAND_COMPOSITOR_H

#include <QObject>

#include <kwaylandclient_export.h>

struct wl_compositor;

namespace KWayland
{
namespace Client
{

class Surface;

class KWAYLANDCLIENT_EXPORT Compositor : public QObject
{
    Q_OBJECT
public:
    explicit Compositor(QObject *parent = nullptr);
    virtual ~Compositor();

    bool isValid() const;
    void setup(wl_compositor *compositor);
    void release();
    void destroy();

    Surface *createSurface(QObject *parent = nullptr);

    operator wl_compositor*();
    operator wl_compositor*() const;

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
