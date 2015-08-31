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

class OutputManagement;
/**
 * @short Wrapper for the connected, but disabled outputs.
 * @see OutputManagement
 **/
class KWAYLANDCLIENT_EXPORT DisabledOutput : public QObject
{
    Q_OBJECT
public:
    virtual ~DisabledOutput();

    /**
     * @return EDID information of the output as QString
     */
    QString edid() const;

    /**
     * @return The name of the output
     */
    QString name() const;

    /**
     * @return The connector of the output
     */
    QString connector() const;

private:
    friend class OutputManagement;
    explicit DisabledOutput(QObject *parent = nullptr);
    void setConnector(const QString &c);
    void setEdid(const QString &e);
    void setName(const QString &n);

    class Private;
    QScopedPointer<Private> d;
};

}
}


#endif
