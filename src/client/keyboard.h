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
#ifndef WAYLAND_KEYBOARD_H
#define WAYLAND_KEYBOARD_H

#include <QObject>

#include <KWayland/Client/kwaylandclient_export.h>

struct wl_keyboard;

namespace KWayland
{
namespace Client
{

class KWAYLANDCLIENT_EXPORT Keyboard : public QObject
{
    Q_OBJECT
public:
    enum class KeyState {
        Released,
        Pressed
    };
    explicit Keyboard(QObject *parent = nullptr);
    virtual ~Keyboard();

    bool isValid() const;
    void setup(wl_keyboard *keyboard);
    void release();

    operator wl_keyboard*();
    operator wl_keyboard*() const;

Q_SIGNALS:
    void keymapChanged(int fd, quint32 size);
    void keyChanged(quint32 key, KWayland::Client::Keyboard::KeyState state, quint32 time);
    void modifiersChanged(quint32 depressed, quint32 latched, quint32 locked, quint32 group);

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

Q_DECLARE_METATYPE(KWayland::Client::Keyboard::KeyState)

#endif
