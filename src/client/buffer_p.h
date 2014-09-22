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
#ifndef WAYLAND_BUFFER_P_H
#define WAYLAND_BUFFER_P_H
#include "buffer.h"
// wayland
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{

class Buffer::Private
{
public:
    Private(Buffer *q, ShmPool *parent, wl_buffer *nativeBuffer, const QSize &size, int32_t stride, size_t offset, Format format);
    ~Private();
    void destroy();

    ShmPool *shm;
    wl_buffer *nativeBuffer;
    bool released;
    QSize size;
    int32_t stride;
    size_t offset;
    bool used;
    Format format;
private:
    Buffer *q;
    static const struct wl_buffer_listener s_listener;
    static void releasedCallback(void *data, wl_buffer *wl_buffer);
};

}
}

#endif
