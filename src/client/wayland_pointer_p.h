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
#ifndef WAYLAND_POINTER_P_H
#define WAYLAND_POINTER_P_H

struct wl_proxy;

namespace KWayland
{
namespace Client
{

template<typename Pointer, void (*deleter)(Pointer *)>
class WaylandPointer
{
public:
    WaylandPointer() = default;
    WaylandPointer(Pointer *p) : m_pointer(p) {}
    WaylandPointer(const WaylandPointer &other) = delete;
    virtual ~WaylandPointer() {
        release();
    }

    void setup(Pointer *pointer) {
        Q_ASSERT(pointer);
        Q_ASSERT(!m_pointer);
        m_pointer = pointer;
    }

    void release() {
        if (!m_pointer) {
            return;
        }
        deleter(m_pointer);
        m_pointer = nullptr;
    }

    void destroy() {
        if (!m_pointer) {
            return;
        }
        free(m_pointer);
        m_pointer = nullptr;
    }

    bool isValid() const {
        return m_pointer != nullptr;
    }

    operator Pointer*() {
        return m_pointer;
    }

    operator Pointer*() const {
        return m_pointer;
    }

    operator wl_proxy*() {
        return reinterpret_cast<wl_proxy*>(m_pointer);
    }

    Pointer *operator->() {
        return m_pointer;
    }

    operator bool() {
        return isValid();
    }

    operator bool() const {
        return isValid();
    }

private:
    Pointer *m_pointer = nullptr;
};

}
}

#endif
