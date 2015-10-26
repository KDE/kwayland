/********************************************************************
Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
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
#include "outputconfiguration_interface.h"
#include "outputdevice_interface.h"
#include "resource_p.h"
#include "display.h"

#include <wayland-server.h>
#include "wayland-output-management-server-protocol.h"
#include "wayland-org_kde_kwin_outputdevice-server-protocol.h"

#include <QDebug>
#include <QSize>

namespace KWayland
{
namespace Server
{


class OutputConfigurationInterface::Private : public Resource::Private
{
public:
    Private(OutputConfigurationInterface *q, OutputManagementInterface *c, wl_resource *parentResource);
    ~Private();

    void sendApplied();
    void sendFailed();
    void applyPendingChanges(OutputDeviceInterface *outputdevice);
    void clearPendingChanges();

    bool hasPendingChanges(OutputDeviceInterface *outputdevice) const;
    Changes* pendingChanges(OutputDeviceInterface *outputdevice);

    OutputManagementInterface *outputManagement;
    QHash<OutputDeviceInterface*, Changes*> changes;

    static const quint32 s_version = 1;

private:
    static void enableCallback(wl_client *client, wl_resource *resource,
                               wl_resource * outputdevice, int32_t enable);
    static void modeCallback(wl_client *client, wl_resource *resource,
                             wl_resource * outputdevice, int32_t mode_id);
    static void transformCallback(wl_client *client, wl_resource *resource,
                                  wl_resource * outputdevice, int32_t transform);
    static void positionCallback(wl_client *client, wl_resource *resource,
                                 wl_resource * outputdevice, int32_t x, int32_t y);
    static void scaleCallback(wl_client *client, wl_resource *resource,
                              wl_resource * outputdevice, int32_t scale);
    static void applyCallback(wl_client *client, wl_resource *resource);

    OutputConfigurationInterface *q_func() {
        return reinterpret_cast<OutputConfigurationInterface *>(q);
    }

    static const struct org_kde_kwin_outputconfiguration_interface s_interface;
};

const struct org_kde_kwin_outputconfiguration_interface OutputConfigurationInterface::Private::s_interface = {
    enableCallback,
    modeCallback,
    transformCallback,
    positionCallback,
    scaleCallback,
    applyCallback
};

OutputConfigurationInterface::OutputConfigurationInterface(OutputManagementInterface* parent, wl_resource* parentResource): Resource(new Private(this, parent, parentResource))
{
    Q_D();
    d->outputManagement = parent;
}

OutputConfigurationInterface::~OutputConfigurationInterface()
{
    Q_D();
    d->clearPendingChanges();
}

void OutputConfigurationInterface::Private::enableCallback(wl_client *client, wl_resource *resource, wl_resource * outputdevice, int32_t enable)
{
    Q_UNUSED(client);
    auto s = cast<Private>(resource);
    Q_ASSERT(s);
    auto _enable = (enable == ORG_KDE_KWIN_OUTPUTDEVICE_ENABLEMENT_ENABLED) ?
                                    OutputDeviceInterface::Enablement::Enabled :
                                    OutputDeviceInterface::Enablement::Disabled;
    OutputDeviceInterface *o = OutputDeviceInterface::get(outputdevice);
    auto pendingChanges = s->pendingChanges(o);
    if (o->enabled() != _enable) {
        pendingChanges->enabledChanged = true;
        pendingChanges->enabled = _enable;
    } else if (pendingChanges->enabledChanged) {
        pendingChanges->enabledChanged = false;
    }
}

void OutputConfigurationInterface::Private::modeCallback(wl_client *client, wl_resource *resource, wl_resource * outputdevice, int32_t mode_id)
{
    Q_UNUSED(client);
    bool modeValid = false;
    OutputDeviceInterface *o = OutputDeviceInterface::get(outputdevice);

    foreach (auto m, o->modes()) {
        if (m.id == mode_id) {
            modeValid = true;
            break;
        }
    }
    if (!modeValid) {
        qWarning() << "Set invalid mode id:" << mode_id;
        return;
    }
    auto s = cast<Private>(resource);
    Q_ASSERT(s);
    auto pendingChanges = s->pendingChanges(o);
    if (o->currentModeId() != mode_id) {
        pendingChanges->modeChanged = true;
        pendingChanges->mode = mode_id;
    } else if (pendingChanges->modeChanged) {
        pendingChanges->modeChanged = false;
    }
}

void OutputConfigurationInterface::Private::transformCallback(wl_client *client, wl_resource *resource, wl_resource * outputdevice, int32_t transform)
{
    Q_UNUSED(client);
    auto toTransform = [transform]() {
        switch (transform) {
            case WL_OUTPUT_TRANSFORM_90:
                return OutputDeviceInterface::Transform::Rotated90;
            case WL_OUTPUT_TRANSFORM_180:
                return OutputDeviceInterface::Transform::Rotated180;
            case WL_OUTPUT_TRANSFORM_270:
                return OutputDeviceInterface::Transform::Rotated270;
            case WL_OUTPUT_TRANSFORM_FLIPPED:
                return OutputDeviceInterface::Transform::Flipped;
            case WL_OUTPUT_TRANSFORM_FLIPPED_90:
                return OutputDeviceInterface::Transform::Flipped90;
            case WL_OUTPUT_TRANSFORM_FLIPPED_180:
                return OutputDeviceInterface::Transform::Flipped180;
            case WL_OUTPUT_TRANSFORM_FLIPPED_270:
                return OutputDeviceInterface::Transform::Flipped270;
            case WL_OUTPUT_TRANSFORM_NORMAL:
            default:
                return OutputDeviceInterface::Transform::Normal;
        }
    };
    auto _transform = toTransform();
    OutputDeviceInterface *o = OutputDeviceInterface::get(outputdevice);
    auto s = cast<Private>(resource);
    Q_ASSERT(s);
    auto pendingChanges = s->pendingChanges(o);
    if (o->transform() != _transform) {
        pendingChanges->transform = _transform;
        pendingChanges->transformChanged = true;
    } else if (pendingChanges->transformChanged) {
        pendingChanges->transformChanged = false;
    }
}

void OutputConfigurationInterface::Private::positionCallback(wl_client *client, wl_resource *resource, wl_resource * outputdevice, int32_t x, int32_t y)
{
    Q_UNUSED(client);
    auto _pos = QPoint(x, y);
    OutputDeviceInterface *o = OutputDeviceInterface::get(outputdevice);
    auto s = cast<Private>(resource);
    Q_ASSERT(s);
    auto pendingChanges = s->pendingChanges(o);
    if (o->globalPosition() != _pos) {
        pendingChanges->positionChanged = true;
        pendingChanges->position = _pos;
    } else if (pendingChanges->positionChanged) {
        pendingChanges->positionChanged = false;
    }
}

void OutputConfigurationInterface::Private::scaleCallback(wl_client *client, wl_resource *resource, wl_resource * outputdevice, int32_t scale)
{
    Q_UNUSED(client);
    if (scale <= 0) {
        qWarning() << "Requested to scale output device to" << scale << ", but I can't do that.";
        return;
    }
    OutputDeviceInterface *o = OutputDeviceInterface::get(outputdevice);
    auto s = cast<Private>(resource);
    Q_ASSERT(s);
    auto pendingChanges = s->pendingChanges(o);
    if (o->scale() != scale) {
        pendingChanges->scaleChanged = true;
        pendingChanges->scale = scale;
    } else if (pendingChanges->scaleChanged) {
        pendingChanges->scaleChanged = false;
    }
}

void OutputConfigurationInterface::Private::applyCallback(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client);
    auto s = cast<Private>(resource);
    Q_ASSERT(s);
    auto q = reinterpret_cast<OutputConfigurationInterface *>(s->q);
    Q_ASSERT(q);
    Q_EMIT q->applyRequested();
    Q_FOREACH (auto o, s->changes.keys()) {
        s->applyPendingChanges(o);
    }
}

OutputConfigurationInterface::Private::Private(OutputConfigurationInterface *q, OutputManagementInterface *c, wl_resource *parentResource)
: Resource::Private(q, c, parentResource, &org_kde_kwin_outputconfiguration_interface, &s_interface)
{
}

OutputConfigurationInterface::Private::~Private()
{
    if (resource) {
        wl_resource_destroy(resource);
        resource = nullptr;
    }
}

OutputConfigurationInterface::Private *OutputConfigurationInterface::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}

void OutputConfigurationInterface::setApplied()
{
    Q_D();
    Q_ASSERT(d->outputManagement);
    auto outputs = d->outputManagement->display()->outputDevices();
    // ...
    d->sendApplied();
}

void OutputConfigurationInterface::Private::sendApplied()
{
    foreach (auto r, s_allResources) {
        org_kde_kwin_outputconfiguration_send_applied(r->resource);
    }
}

void OutputConfigurationInterface::setFailed()
{
    Q_D();
    Q_ASSERT(d->outputManagement);
    auto outputs = d->outputManagement->display()->outputDevices();
    // ...
    d->sendFailed();
}

void OutputConfigurationInterface::Private::sendFailed()
{
    foreach (auto r, s_allResources) {
        org_kde_kwin_outputconfiguration_send_failed(r->resource);
    }
}

OutputConfigurationInterface::Changes* OutputConfigurationInterface::Private::pendingChanges(OutputDeviceInterface *outputdevice)
{
    if (!changes.keys().contains(outputdevice)) {
        changes[outputdevice] = new Changes;
    }
    return changes[outputdevice];
}

void OutputConfigurationInterface::Private::applyPendingChanges(OutputDeviceInterface *outputdevice)
{
    auto c = changes[outputdevice];
    if (c->enabledChanged) {
        outputdevice->setEnabled(c->enabled);
    }
    if (c->modeChanged) {
        outputdevice->setCurrentMode(c->mode);
    }
    if (c->transformChanged) {
        outputdevice->setTransform(c->transform);
    }
    if (c->positionChanged) {
        outputdevice->setGlobalPosition(c->position);
    }
    if (c->scaleChanged) {
        outputdevice->setScale(c->scale);
    }
    clearPendingChanges();
}

bool OutputConfigurationInterface::Private::hasPendingChanges(OutputDeviceInterface *outputdevice) const
{
    if (!changes.keys().contains(outputdevice)) {
        return false;
    }
    auto c = changes[outputdevice];
    return c->enabledChanged ||
    c->modeChanged ||
    c->transformChanged ||
    c->positionChanged ||
    c->scaleChanged;
}

void OutputConfigurationInterface::Private::clearPendingChanges()
{
    qDeleteAll(changes.begin(), changes.end());
    changes.clear();
}


}
}
