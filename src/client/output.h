/********************************************************************
Copyright 2013  Martin Gräßlin <mgraesslin@kde.org>

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
#ifndef WAYLAND_OUTPUT_H
#define WAYLAND_OUTPUT_H

#include <QObject>

#include <kwaylandclient_export.h>

struct wl_output;
class QPoint;
class QRect;
class QSize;

namespace KWayland
{
namespace Client
{

class KWAYLANDCLIENT_EXPORT Output : public QObject
{
    Q_OBJECT
public:
    enum class SubPixel {
        Unknown,
        None,
        HorizontalRGB,
        HorizontalBGR,
        VerticalRGB,
        VerticalBGR
    };
    enum class Transform {
        Normal,
        Rotated90,
        Rotated180,
        Rotated270,
        Flipped,
        Flipped90,
        Flipped180,
        Flipped270
    };
    explicit Output(QObject *parent = nullptr);
    virtual ~Output();

    void setup(wl_output *output);

    bool isValid() const;
    operator wl_output*();
    operator wl_output*() const;
    wl_output *output();
    QSize physicalSize() const;
    QPoint globalPosition() const;
    QString manufacturer() const;
    QString model() const;
    QSize pixelSize() const;
    QRect geometry() const;
    int refreshRate() const;
    int scale() const;
    SubPixel subPixel() const;
    Transform transform() const;

Q_SIGNALS:
    void changed();

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

Q_DECLARE_METATYPE(KWayland::Client::Output::SubPixel)
Q_DECLARE_METATYPE(KWayland::Client::Output::Transform)

#endif
