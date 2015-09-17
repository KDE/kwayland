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

    static const quint32 s_version = 1;
    OutputManagementInterface *outputManagement;

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
}

void OutputConfigurationInterface::Private::enableCallback(wl_client *client, wl_resource *resource, wl_resource * outputdevice, int32_t enable)
{
    auto _enable = (enable == ORG_KDE_KWIN_OUTPUTDEVICE_ENABLEMENT_ENABLED) ?
                                    OutputDeviceInterface::Enablement::Enabled :
                                    OutputDeviceInterface::Enablement::Disabled;
    OutputDeviceInterface *o = OutputDeviceInterface::get(outputdevice);
    if (o->enabled() != _enable) {
        o->pendingChanges()->enabledChanged = true;
        o->pendingChanges()->enabled = _enable;
        Q_EMIT o->pendingChangesChanged();
    } else if (o->pendingChanges()->enabledChanged) {
        o->pendingChanges()->enabledChanged = false;
        Q_EMIT o->pendingChangesChanged();
    }
}

void OutputConfigurationInterface::Private::modeCallback(wl_client *client, wl_resource *resource, wl_resource * outputdevice, int32_t mode_id)
{
    bool modeValid = false;
    OutputDeviceInterface *output = OutputDeviceInterface::get(outputdevice);

    foreach (auto m, output->modes()) {
        if (m.id == mode_id) {
            modeValid = true;
        }
    }
    if (!modeValid) {
        qWarning() << "Set invalid mode id:" << mode_id;
        return;
    }
    OutputDeviceInterface *o = OutputDeviceInterface::get(outputdevice);
    if (o->currentModeId() != mode_id) {
        o->pendingChanges()->modeChanged = true;
        o->pendingChanges()->mode = mode_id;
        Q_EMIT o->pendingChangesChanged();
    } else if (o->pendingChanges()->modeChanged) {
        o->pendingChanges()->modeChanged = false;
        Q_EMIT o->pendingChangesChanged();
    }
}

void OutputConfigurationInterface::Private::transformCallback(wl_client *client, wl_resource *resource, wl_resource * outputdevice, int32_t transform)
{
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
    if (o->transform() != _transform) {
        o->pendingChanges()->transform = _transform;
        o->pendingChanges()->transformChanged = true;
        Q_EMIT o->pendingChangesChanged();
    } else if (o->pendingChanges()->transformChanged) {
        o->pendingChanges()->transformChanged = false;
        Q_EMIT o->pendingChangesChanged();
    }
}

void OutputConfigurationInterface::Private::positionCallback(wl_client *client, wl_resource *resource, wl_resource * outputdevice, int32_t x, int32_t y)
{
    auto _pos = QPoint(x, y);
    qDebug() << "new position:" << _pos;
    OutputDeviceInterface *o = OutputDeviceInterface::get(outputdevice);
    if (o->globalPosition() != _pos) {
        o->pendingChanges()->positionChanged = true;
        o->pendingChanges()->position = _pos;
        Q_EMIT o->pendingChangesChanged();
    } else if (o->pendingChanges()->positionChanged) {
        o->pendingChanges()->positionChanged = false;
        Q_EMIT o->pendingChangesChanged();
    }
}

void OutputConfigurationInterface::Private::scaleCallback(wl_client *client, wl_resource *resource, wl_resource * outputdevice, int32_t scale)
{
    if (scale <= 0) {
        qWarning() << "Requested to scale output device to" << scale << ", but I can't do that.";
        return;
    }
    OutputDeviceInterface *o = OutputDeviceInterface::get(outputdevice);
    if (o->scale() != scale) {
        o->pendingChanges()->scaleChanged = true;
        o->pendingChanges()->scale = scale;
        Q_EMIT o->pendingChangesChanged();
    } else if (o->pendingChanges()->scaleChanged) {
        o->pendingChanges()->scaleChanged = false;
        Q_EMIT o->pendingChangesChanged();
    }
}

void OutputConfigurationInterface::Private::applyCallback(wl_client *client, wl_resource *resource)
{
    qDebug() << "Asked to apply";
    auto s = cast<Private>(resource);
    Q_ASSERT(s);
    auto q = reinterpret_cast<OutputConfigurationInterface *>(s->q);
    Q_ASSERT(q);
    Q_EMIT q->applyRequested();
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
    qDebug() << "set applied";
    Q_D();
    Q_ASSERT(d->outputManagement);
    auto outputs = d->outputManagement->display()->outputDevices();
    Q_FOREACH (auto o, outputs) {
        qDebug() << "output: " << o->uuid() << o->model();
        o->applyPendingChanges();
    }
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
    qDebug() << "set FAILED, resetting outputs";
    Q_ASSERT(d->outputManagement);
    auto outputs = d->outputManagement->display()->outputDevices();
    Q_FOREACH (auto o, outputs) {
        qDebug() << "output: " << o->uuid() << o->model();
        o->clearPendingChanges();
    }
    d->sendFailed();
}

void OutputConfigurationInterface::Private::sendFailed()
{
    foreach (auto r, s_allResources) {
        org_kde_kwin_outputconfiguration_send_failed(r->resource);
    }
}


}
}
