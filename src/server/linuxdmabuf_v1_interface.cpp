/********************************************************************
Copyright © 2018 Fredrik Höglund <fredrik@kde.org>

Based on the libweston implementation,
Copyright © 2014, 2015 Collabora, Ltd.

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

#include <KWayland/Server/kwaylandserver_export.h>
#include "linuxdmabuf_v1_interface.h"
#include "wayland-linux-dmabuf-unstable-v1-server-protocol.h"
#include "wayland-server-protocol.h"
#include "global_p.h"

#include "drm_fourcc.h"

#include <QVector>

#include <unistd.h>
#include <assert.h>

#include <array>


namespace KWayland
{

namespace Server
{


class LinuxDmabufParams
{
public:
    LinuxDmabufParams(LinuxDmabufUnstableV1Interface *dmabufInterface, wl_client *client, uint32_t version, uint32_t id);
    ~LinuxDmabufParams();

    template <typename... Args>
    void postError(uint32_t code, const char *msg, Args... args) {
        wl_resource_post_error(m_resource, code, msg, args...);
    }

    void postNoMemory() { wl_resource_post_no_memory(m_resource); }

    wl_resource *resource() const { return m_resource; }

    void add(int fd, uint32_t plane_idx, uint32_t offset, uint32_t stride, uint64_t modifier);
    void create(wl_client *client, uint32_t bufferId, const QSize &size, uint32_t format, uint32_t flags);

    static void destroy(wl_client *client, wl_resource *resource);
    static void add(wl_client *client, wl_resource *resource, int fd, uint32_t plane_idx, uint32_t offset, uint32_t stride, uint32_t modifier_hi, uint32_t modifier_lo);
    static void create(wl_client *client, wl_resource *resource, int width, int height, uint32_t format, uint32_t flags);
    static void createImmed(wl_client *client, wl_resource *resource, uint32_t new_id, int width, int height, uint32_t format, uint32_t flags);

private:
    static const struct zwp_linux_buffer_params_v1_interface s_interface;

    wl_resource *m_resource;
    LinuxDmabufUnstableV1Interface *m_dmabufInterface;
    std::array<LinuxDmabuf::Plane, 4> m_planes;
    size_t m_planeCount = 0;
    bool m_bufferCreated = false;
};


#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zwp_linux_buffer_params_v1_interface LinuxDmabufParams::s_interface = {
    destroy,
    add,
    create,
    createImmed
};
#endif


LinuxDmabufParams::LinuxDmabufParams(LinuxDmabufUnstableV1Interface *dmabufInterface, wl_client *client, uint32_t version, uint32_t id)
    : m_dmabufInterface(dmabufInterface)
{
    m_resource = wl_resource_create(client, &zwp_linux_buffer_params_v1_interface, version, id);
    if (!m_resource) {
        return;
    }

    wl_resource_set_implementation(m_resource, &s_interface, this,
                                   [](wl_resource *resource) {
                                       delete static_cast<LinuxDmabufParams *>(wl_resource_get_user_data(resource));
                                   });

    for (auto &plane : m_planes) {
        plane.fd = -1;
        plane.offset = 0;
        plane.stride = 0;
        plane.modifier = 0;
    }
}


LinuxDmabufParams::~LinuxDmabufParams()
{
    // Close the file descriptors
    for (auto &plane : m_planes) {
        if (plane.fd != -1) {
            ::close(plane.fd);
        }
    }
}


void LinuxDmabufParams::create(wl_client *client, uint32_t bufferId, const QSize &size, uint32_t format, uint32_t flags)
{
    // Validate the parameters
    // -----------------------
    const uint32_t width = size.width();
    const uint32_t height = size.height();

    if (m_bufferCreated) {
        postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_ALREADY_USED,
                  "params was already used to create a wl_buffer");
        return;
    }

    if (m_planeCount == 0) {
        postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INCOMPLETE,
                  "no dmabuf has been added to the params");
        return;
    }

    // Check for holes in the dmabufs set (e.g. [0, 1, 3])
    for (uint32_t i = 0; i < m_planeCount; i++) {
        if (m_planes[i].fd != -1)
            continue;

        postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INCOMPLETE,
                  "no dmabuf has been added for plane %i", i);
        return;
    }

    if (width < 1 || height < 1) {
        postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INVALID_DIMENSIONS,
                  "invalid width %d or height %d", width, height);
        return;
    }

    for (uint32_t i = 0; i < m_planeCount; i++) {
        auto &plane = m_planes[i];

        if (uint64_t(plane.offset) + plane.stride > UINT32_MAX) {
            postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_OUT_OF_BOUNDS,
                      "size overflow for plane %i", i);
            return;
        }

        if (i == 0 && uint64_t(plane.offset) + plane.stride * height > UINT32_MAX) {
            postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_OUT_OF_BOUNDS,
                      "size overflow for plane %i", i);
            return;
        }

        // Don't report an error as it might be caused by the kernel not supporting seeking on dmabuf
        off_t size = ::lseek(plane.fd, 0, SEEK_END);
        if (size == -1)
            continue;

        if (plane.offset >= size) {
            postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_OUT_OF_BOUNDS,
                      "invalid offset %i for plane %i",
                      plane.offset, i);
            return;
        }

        if (plane.offset + plane.stride > size) {
            postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_OUT_OF_BOUNDS,
                      "invalid stride %i for plane %i",
                      plane.stride, i);
            return;
        }

        // Only valid for first plane as other planes might be
        // sub-sampled according to fourcc format
        if (i == 0 && plane.offset + plane.stride * height > size) {
            postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_OUT_OF_BOUNDS,
                      "invalid buffer stride or height for plane %i", i);
            return;
        }
    }


    // Import the buffer
    // -----------------
    QVector<LinuxDmabuf::Plane> planes;
    planes.reserve(m_planeCount);
    for (uint32_t i = 0; i < m_planeCount; i++)
        planes << m_planes[i];

    LinuxDmabuf::Buffer *buffer = m_dmabufInterface->bridge()->importBuffer(planes, format, size, (LinuxDmabuf::Flags) flags);
    if (buffer) {
        // The buffer has ownership of the file descriptors now
        for (auto &plane : m_planes) {
            plane.fd = -1;
        }

        wl_resource *resource = wl_resource_create(client, &wl_buffer_interface, 1, bufferId);
        if (!resource ) {
            postNoMemory();
            delete buffer;
            return;
        }

        wl_resource_set_implementation(resource, m_dmabufInterface->bufferImplementation(), buffer,
                                       [](wl_resource *resource) { // Destructor
                                            delete static_cast<LinuxDmabuf::Buffer *>(wl_resource_get_user_data(resource));
                                       });

        // XXX Do we need this?
        //buffer->setResource(resource);

        // Send a 'created' event when the request is not for an immediate import, i.e. bufferId is zero
        if (bufferId == 0) {
            zwp_linux_buffer_params_v1_send_created(m_resource, resource);
        }

        m_bufferCreated = true;
    } else {
        if (bufferId == 0) {
            zwp_linux_buffer_params_v1_send_failed(m_resource);
        } else {
            // since the behavior is left implementation defined by the
            // protocol in case of create_immed failure due to an unknown cause,
            // we choose to treat it as a fatal error and immediately kill the
            // client instead of creating an invalid handle and waiting for it
            // to be used.
            postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INVALID_WL_BUFFER,
                      "importing the supplied dmabufs failed");
        }
    }
}


void LinuxDmabufParams::add(int fd, uint32_t plane_idx, uint32_t offset, uint32_t stride, uint64_t modifier)
{
    if (m_bufferCreated) {
        postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_ALREADY_USED,
                  "params was already used to create a wl_buffer");
        ::close(fd);
        return;
    }

    if (plane_idx >= m_planes.size()) {
        postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_PLANE_IDX,
                  "plane index %u is too high", plane_idx);
        ::close(fd);
        return;
    }

    auto &plane = m_planes[plane_idx];

    if (plane.fd != -1) {
        postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_PLANE_SET,
                  "a dmabuf has already been added for plane %u",
                   plane_idx);
        ::close(fd);
        return;
    }

    plane.fd = fd;
    plane.offset = offset;
    plane.stride = stride;
    plane.modifier = modifier;

    m_planeCount++;
}



// --------------------------------------------------------------------



void LinuxDmabufParams::destroy(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    wl_resource_destroy(resource);
}


void LinuxDmabufParams::add(wl_client *client, wl_resource *resource,
                            int fd, uint32_t plane_idx,
                            uint32_t offset, uint32_t stride,
                            uint32_t modifier_hi, uint32_t modifier_lo)
{
    Q_UNUSED(client)

    LinuxDmabufParams *params = static_cast<LinuxDmabufParams *>(wl_resource_get_user_data(resource));
    assert(params->m_resource == resource);
    params->add(fd, plane_idx, offset, stride, (uint64_t(modifier_hi) << 32) | modifier_lo);
}


void LinuxDmabufParams::create(wl_client *client, wl_resource *resource,
                               int width, int height, uint32_t format, uint32_t flags)
{
    Q_UNUSED(client)

    LinuxDmabufParams *params = static_cast<LinuxDmabufParams *>(wl_resource_get_user_data(resource));
    assert(params->m_resource == resource);
    params->create(client, 0, QSize(width, height), format, flags);
}


void LinuxDmabufParams::createImmed(wl_client *client, wl_resource *resource,
                                    uint32_t new_id, int width, int height,
                                    uint32_t format, uint32_t flags)
{
    Q_UNUSED(client)

    LinuxDmabufParams *params = static_cast<LinuxDmabufParams *>(wl_resource_get_user_data(resource));
    assert(params->m_resource == resource);
    params->create(client, new_id, QSize(width, height), format, flags);
}



// --------------------------------------------------------------------



class LinuxDmabufUnstableV1Interface::Private : public Global::Private
{
public:
    Private(LinuxDmabufUnstableV1Interface *q, Display *display);
    ~Private();

    static const struct wl_buffer_interface *bufferImplementation() { return &s_bufferImplementation; }
    LinuxDmabufUnstableV1Interface::Bridge *bridge;
    LinuxDmabufUnstableV1Interface * const q;
    static const uint32_t s_version;

    void bind(wl_client *client, uint32_t version, uint32_t id) override final;
    void createParams(wl_client *client, wl_resource *resource, uint32_t id);

    static void unbind(wl_client *client, wl_resource *resource);
    static void createParamsCallback(wl_client *client, wl_resource *resource, uint32_t id);

private:
    static const struct zwp_linux_dmabuf_v1_interface s_implementation;
    static const struct wl_buffer_interface s_bufferImplementation;
};


#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zwp_linux_dmabuf_v1_interface LinuxDmabufUnstableV1Interface::Private::s_implementation = {
    [](wl_client *, wl_resource *resource) { wl_resource_destroy(resource); }, // unbind
    createParamsCallback
};


const struct wl_buffer_interface LinuxDmabufUnstableV1Interface::Private::s_bufferImplementation = {
    [](wl_client *, wl_resource *resource) { wl_resource_destroy(resource); } // destroy
};

const uint32_t LinuxDmabufUnstableV1Interface::Private::s_version = 3;
#endif


LinuxDmabufUnstableV1Interface::Private::Private(LinuxDmabufUnstableV1Interface *q, Display *display)
    : Global::Private(display, &zwp_linux_dmabuf_v1_interface, s_version),
      q(q)
{
}


LinuxDmabufUnstableV1Interface::Private::~Private()
{
}


void LinuxDmabufUnstableV1Interface::Private::bind(wl_client *client, uint32_t version, uint32_t id)
{
    wl_resource *resource = wl_resource_create(client, &zwp_linux_dmabuf_v1_interface, std::min(s_version, version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(resource, &s_implementation, this, nullptr);

    // Send formats & modifiers
    // ------------------------
    const QVector<uint32_t> formats = bridge->supportedFormats();
    for (uint32_t format : formats) {
        QVector<uint64_t> modifiers = bridge->supportedModifiers(format);
        if (modifiers.isEmpty()) {
            modifiers << DRM_FORMAT_MOD_INVALID;
        }

        for (uint64_t modifier : qAsConst(modifiers)) {
            if (version >= ZWP_LINUX_DMABUF_V1_MODIFIER_SINCE_VERSION) {
                const uint32_t modifier_lo = modifier & 0xFFFFFFFF;
                const uint32_t modifier_hi = modifier >> 32;
                zwp_linux_dmabuf_v1_send_modifier(resource, format, modifier_hi, modifier_lo);
            } else if (modifier == DRM_FORMAT_MOD_LINEAR || modifier == DRM_FORMAT_MOD_INVALID) {
                zwp_linux_dmabuf_v1_send_format(resource, format);
            }
        }
    }
}


void LinuxDmabufUnstableV1Interface::Private::createParams(wl_client *client, wl_resource *resource, uint32_t id)
{
    LinuxDmabufParams *params = new LinuxDmabufParams(q, client, wl_resource_get_version(resource), id);
    if (!params->resource()) {
        wl_resource_post_no_memory(resource);
        delete params;
    }
}


void LinuxDmabufUnstableV1Interface::Private::createParamsCallback(wl_client *client, wl_resource *resource, uint32_t id)
{
    LinuxDmabufUnstableV1Interface::Private *global = static_cast<LinuxDmabufUnstableV1Interface::Private *>(wl_resource_get_user_data(resource));
    global->createParams(client, resource, id);
}



// --------------------------------------------------------------------



LinuxDmabufUnstableV1Interface::LinuxDmabufUnstableV1Interface(Display *display, QObject *parent)
    : Global(new Private(this, display), parent)
{
}


LinuxDmabufUnstableV1Interface::~LinuxDmabufUnstableV1Interface()
{
}


void LinuxDmabufUnstableV1Interface::setBridge(LinuxDmabufUnstableV1Interface::Bridge *bridge)
{
    d_func()->bridge = bridge;
}


LinuxDmabufUnstableV1Interface::Bridge *LinuxDmabufUnstableV1Interface::bridge() const
{
    return d_func()->bridge;
}


const struct wl_buffer_interface *LinuxDmabufUnstableV1Interface::bufferImplementation()
{
    return LinuxDmabufUnstableV1Interface::Private::bufferImplementation();
}


LinuxDmabufUnstableV1Interface::Private *LinuxDmabufUnstableV1Interface::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}


} // namespace Server
} // namespace Wayland
