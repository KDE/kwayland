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
#ifndef WAYLAND_POINTER_H
#define WAYLAND_POINTER_H

#include <QObject>

#include <kwaylandclient_export.h>

struct wl_pointer;

namespace KWayland
{
namespace Client
{

class Surface;

class KWAYLANDCLIENT_EXPORT Pointer : public QObject
{
    Q_OBJECT
public:
    enum class ButtonState {
        Released,
        Pressed
    };
    enum class Axis {
        Vertical,
        Horizontal
    };
    explicit Pointer(QObject *parent = nullptr);
    virtual ~Pointer();

    bool isValid() const;
    void setup(wl_pointer *pointer);
    void release();

    Surface *enteredSurface() const;
    Surface *enteredSurface();

    operator wl_pointer*();
    operator wl_pointer*() const;

Q_SIGNALS:
    void entered(quint32 serial, const QPointF &relativeToSurface);
    void left(quint32 serial);
    void motion(const QPointF &relativeToSurface, quint32 time);
    void buttonStateChanged(quint32 serial, quint32 time, quint32 button, KWayland::Client::Pointer::ButtonState state);
    void axisChanged(quint32 time, KWayland::Client::Pointer::Axis axis, qreal delta);

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

Q_DECLARE_METATYPE(KWayland::Client::Pointer::ButtonState)
Q_DECLARE_METATYPE(KWayland::Client::Pointer::Axis)

#endif
