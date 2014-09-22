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
#ifndef WAYLAND_SURFACE_H
#define WAYLAND_SURFACE_H

#include "buffer.h"

#include <QObject>
#include <QPoint>
#include <QSize>

#include <KWayland/Client/kwaylandclient_export.h>

struct wl_buffer;
struct wl_surface;

namespace KWayland
{
namespace Client
{

class KWAYLANDCLIENT_EXPORT Surface : public QObject
{
    Q_OBJECT
public:
    explicit Surface(QObject *parent = nullptr);
    virtual ~Surface();

    void setup(wl_surface *surface);
    void release();
    void destroy();
    bool isValid() const;
    void setupFrameCallback();
    enum class CommitFlag {
        None,
        FrameCallback
    };
    void commit(CommitFlag flag = CommitFlag::FrameCallback);
    void damage(const QRect &rect);
    void damage(const QRegion &region);
    void attachBuffer(wl_buffer *buffer, const QPoint &offset = QPoint());
    void attachBuffer(Buffer *buffer, const QPoint &offset = QPoint());
    void attachBuffer(Buffer::Ptr buffer, const QPoint &offset = QPoint());
    void setSize(const QSize &size);
    QSize size() const;

    operator wl_surface*();
    operator wl_surface*() const;

    static const QList<Surface*> &all();
    static Surface *get(wl_surface *native);

Q_SIGNALS:
    void frameRendered();
    void sizeChanged(const QSize&);

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
