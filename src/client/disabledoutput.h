/********************************************************************
Copyright 2013  Martin Gräßlin <mgraesslin@kde.org>
Copyright 2015  Sebastian Kügler <sebas@kde.org>

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
#ifndef WAYLAND_DISABLEDOUTPUT_H
#define WAYLAND_DISABLEDOUTPUT_H

#include <QObject>
#include <QPointer>
#include <QSize>

#include <KWayland/Client/kwaylandclient_export.h>

struct wl_output;
class QPoint;
class QRect;

namespace KWayland
{
namespace Client
{

/**
 * @short Wrapper for the wl_output interface.
 *
 * This class provides a convenient wrapper for the wl_output interface.
 * Its main purpose is to hold the information about one DisabledOutput.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create an DisabledOutput interface:
 * @code
 * DisabledOutput *c = registry->createDisabledOutput(name, version);
 * @endcode
 *
 * This creates the DisabledOutput and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * DisabledOutput *c = new DisabledOutput;
 * c->setup(registry->bindDisabledOutput(name, version));
 * @endcode
 *
 * The DisabledOutput can be used as a drop-in replacement for any wl_output
 * pointer as it provides matching cast operators.
 *
 * Please note that all properties of DisabledOutput are not valid until the
 * changed signal has been emitted. The wayland server is pushing the
 * information in an async way to the DisabledOutput instance. By emitting changed
 * the DisabledOutput indicates that all relevant information is available.
 *
 * @see Registry
 **/
class KWAYLANDCLIENT_EXPORT DisabledOutput : public QObject
{
    Q_OBJECT
public:
    explicit DisabledOutput(QObject *parent = nullptr);
    virtual ~DisabledOutput();

    QString edid() const;
    void setEdid(const QString &e);

    QString name() const;
    void setName(const QString &n);

    QString connector() const;
    void setConnector(const QString &c);

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}


#endif
