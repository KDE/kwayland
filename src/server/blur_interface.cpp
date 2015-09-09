/********************************************************************
Copyright 2015  Martin Gräßlin <mgraesslin@kde.org>
Copyright 2015  Marco Martin <mart@kde.org>

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
#include "blur_interface.h"
#include "region_interface.h"
#include "display.h"
#include "global_p.h"
#include "resource_p.h"
#include "surface_interface_p.h"

#include <wayland-server.h>
#include <wayland-blur-server-protocol.h>

namespace KWayland
{
namespace Server
{

static const quint32 s_version = 1;

class BlurManagerInterface::Private : public Global::Private
{
public:
    Private(BlurManagerInterface *q, Display *d);

private:
    void bind(wl_client *client, uint32_t version, uint32_t id) override;
    void createBlur(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *surface);

    static void createCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *surface);
    static void unsetCallback(wl_client *client, wl_resource *resource, wl_resource *surface);
    static void unbind(wl_resource *resource);
    static Private *cast(wl_resource *r) {
        return reinterpret_cast<Private*>(wl_resource_get_user_data(r));
    }

    BlurManagerInterface *q;
    static const struct org_kde_kwin_blur_manager_interface s_interface;
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct org_kde_kwin_blur_manager_interface BlurManagerInterface::Private::s_interface = {
    createCallback,
    unsetCallback
};
#endif

BlurManagerInterface::Private::Private(BlurManagerInterface *q, Display *d)
    : Global::Private(d, &org_kde_kwin_blur_manager_interface, s_version)
    , q(q)
{
}

void BlurManagerInterface::Private::bind(wl_client *client, uint32_t version, uint32_t id)
{
    auto c = display->getConnection(client);
    wl_resource *resource = c->createResource(&org_kde_kwin_blur_manager_interface, qMin(version, s_version), id);
    if (!resource) {
        wl_client_post_no_memory(client);
        return;
    }
    wl_resource_set_implementation(resource, &s_interface, this, unbind);
    // TODO: should we track?
}

void BlurManagerInterface::Private::unbind(wl_resource *resource)
{
    Q_UNUSED(resource)
    // TODO: implement?
}

void BlurManagerInterface::Private::createCallback(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *surface)
{
    cast(resource)->createBlur(client, resource, id, surface);
}

void BlurManagerInterface::Private::createBlur(wl_client *client, wl_resource *resource, uint32_t id, wl_resource *surface)
{
    SurfaceInterface *s = SurfaceInterface::get(surface);
    if (!s) {
        return;
    }

    BlurInterface *blur = new BlurInterface(q, resource);
    blur->create(display->getConnection(client), wl_resource_get_version(resource), id);
    if (!blur->resource()) {
        wl_resource_post_no_memory(resource);
        delete blur;
        return;
    }
    QObject::connect(s, &QObject::destroyed, blur,
        [blur] {
            if (blur->resource()) {
                wl_resource_destroy(blur->resource());
                delete blur;
            }
        }
    );
    s->d_func()->setBlur(QPointer<BlurInterface>(blur));
}

void BlurManagerInterface::Private::unsetCallback(wl_client *client, wl_resource *resource, wl_resource *surface)
{
    Q_UNUSED(client)
    Q_UNUSED(resource)
    SurfaceInterface *s = SurfaceInterface::get(surface);
    if (!s) {
        return;
    }
    s->d_func()->setBlur(QPointer<BlurInterface>());
}

BlurManagerInterface::BlurManagerInterface(Display *display, QObject *parent)
    : Global(new Private(this, display), parent)
{
}

BlurManagerInterface::~BlurManagerInterface() = default;

class BlurInterface::Private : public Resource::Private
{
public:
    Private(BlurInterface *q, BlurManagerInterface *c, wl_resource *parentResource);
    ~Private();

    QRegion pendingRegion;
    QRegion currentRegion;

private:
    void commit();
    //TODO
    BlurInterface *q_func() {
        return reinterpret_cast<BlurInterface *>(q);
    }

    static void commitCallback(wl_client *client, wl_resource *resource);
    static void setRegionCallback(wl_client *client, wl_resource *resource, wl_resource *region);
    static void releaseCallback(wl_client *client, wl_resource *resource);

    static const struct org_kde_kwin_blur_interface s_interface;
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct org_kde_kwin_blur_interface BlurInterface::Private::s_interface = {
    commitCallback,
    setRegionCallback,
    releaseCallback
};
#endif

void BlurInterface::Private::commitCallback(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    cast<Private>(resource)->commit();
}

void BlurInterface::Private::commit()
{
    currentRegion = pendingRegion;
}

void BlurInterface::Private::setRegionCallback(wl_client *client, wl_resource *resource, wl_resource *region)
{
    Q_UNUSED(client)
    Private *p = cast<Private>(resource);
    RegionInterface *r = RegionInterface::get(region);
    if (r) {
        p->pendingRegion = r->region();
    } else {
        p->pendingRegion = QRegion();
    }
}

void BlurInterface::Private::releaseCallback(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client);
    Private *p = reinterpret_cast<Private*>(wl_resource_get_user_data(resource));
    wl_resource_destroy(resource);
    p->q->deleteLater();
}

BlurInterface::Private::Private(BlurInterface *q, BlurManagerInterface *c, wl_resource *parentResource)
    : Resource::Private(q, c, parentResource, &org_kde_kwin_blur_interface, &s_interface)
{
}

BlurInterface::Private::~Private()
{
    if (resource) {
        wl_resource_destroy(resource);
        resource = nullptr;
    }
}

BlurInterface::BlurInterface(BlurManagerInterface *parent, wl_resource *parentResource)
    : Resource(new Private(this, parent, parentResource))
{
}

BlurInterface::~BlurInterface() = default;

QRegion BlurInterface::region()
{
    Q_D();
    return d->currentRegion;
}

BlurInterface::Private *BlurInterface::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}

}
}
