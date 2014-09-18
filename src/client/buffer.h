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
#ifndef WAYLAND_BUFFER_H
#define WAYLAND_BUFFER_H

#include <QSize>
#include <QScopedPointer>

#include <kwaylandclient_export.h>

struct wl_buffer;

namespace KWayland
{
namespace Client
{

class ShmPool;

class KWAYLANDCLIENT_EXPORT Buffer
{
public:
    ~Buffer();
    void copy(const void *src);
    void setReleased(bool released);
    void setUsed(bool used);

    wl_buffer *buffer() const;
    QSize size() const;
    int32_t stride() const;
    bool isReleased() const;
    bool isUsed() const;
    uchar *address();

    operator wl_buffer*();
    operator wl_buffer*() const;

private:
    friend class ShmPool;
    explicit Buffer(ShmPool *parent, wl_buffer *buffer, const QSize &size, int32_t stride, size_t offset);
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
