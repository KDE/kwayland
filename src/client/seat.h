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
#ifndef WAYLAND_SEAT_H
#define WAYLAND_SEAT_H

#include <QObject>

#include <KWayland/Client/kwaylandclient_export.h>

struct wl_seat;
struct wl_touch;

namespace KWayland
{
namespace Client
{

class Keyboard;
class Pointer;

class KWAYLANDCLIENT_EXPORT Seat : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool keyboard READ hasKeyboard NOTIFY hasKeyboardChanged)
    Q_PROPERTY(bool pointer READ hasPointer NOTIFY hasPointerChanged)
    Q_PROPERTY(bool touch READ hasTouch NOTIFY hasTouchChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
public:
    explicit Seat(QObject *parent = nullptr);
    virtual ~Seat();

    bool isValid() const;
    void setup(wl_seat *seat);
    void release();
    void destroy();

    bool hasKeyboard() const;
    bool hasPointer() const;
    bool hasTouch() const;
    QString name() const;
    operator wl_seat*();
    operator wl_seat*() const;

    Keyboard *createKeyboard(QObject *parent = nullptr);
    Pointer *createPointer(QObject *parent = nullptr);
    wl_touch *createTouch();

Q_SIGNALS:
    void hasKeyboardChanged(bool);
    void hasPointerChanged(bool);
    void hasTouchChanged(bool);
    void nameChanged(const QString &name);

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
