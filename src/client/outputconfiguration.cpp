/****************************************************************************
 * Copyright 2015  Sebastian Kügler <sebas@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/
#include "outputconfiguration.h"
#include "outputdevice.h"
#include "outputmanagement.h"
#include "event_queue.h"
#include "wayland_pointer_p.h"

#include "wayland-output-management-client-protocol.h"
#include "wayland-org_kde_kwin_outputdevice-client-protocol.h"

namespace KWayland
{
namespace Client
{


class Q_DECL_HIDDEN OutputConfiguration::Private
{
public:
    Private() = default;

    void setup(org_kde_kwin_outputconfiguration *outputconfiguration);

    WaylandPointer<org_kde_kwin_outputconfiguration, org_kde_kwin_outputconfiguration_destroy> outputconfiguration;
    static struct org_kde_kwin_outputconfiguration_listener s_outputconfigurationListener;
    EventQueue *queue = nullptr;

    OutputConfiguration *q;

private:
    static void appliedCallback(void *data, org_kde_kwin_outputconfiguration *config);
    static void failedCallback(void *data, org_kde_kwin_outputconfiguration *config);
};

OutputConfiguration::OutputConfiguration(QObject *parent)
: QObject(parent)
, d(new Private)
{
    d->q = this;
}

OutputConfiguration::~OutputConfiguration()
{
    release();
}

void OutputConfiguration::setup(org_kde_kwin_outputconfiguration *outputconfiguration)
{
    Q_ASSERT(outputconfiguration);
    Q_ASSERT(!d->outputconfiguration);
    d->outputconfiguration.setup(outputconfiguration);
    d->setup(outputconfiguration);
}

void OutputConfiguration::Private::setup(org_kde_kwin_outputconfiguration* outputconfiguration)
{
    org_kde_kwin_outputconfiguration_add_listener(outputconfiguration, &s_outputconfigurationListener, this);
}


void OutputConfiguration::release()
{
    d->outputconfiguration.release();
}

void OutputConfiguration::destroy()
{
    d->outputconfiguration.destroy();
}

void OutputConfiguration::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *OutputConfiguration::eventQueue()
{
    return d->queue;
}

OutputConfiguration::operator org_kde_kwin_outputconfiguration*() {
    return d->outputconfiguration;
}

OutputConfiguration::operator org_kde_kwin_outputconfiguration*() const {
    return d->outputconfiguration;
}

bool OutputConfiguration::isValid() const
{
    return d->outputconfiguration.isValid();
}

// Requests

void OutputConfiguration::setEnabled(OutputDevice *outputdevice, OutputDevice::Enablement enable)
{
    qint32 _enable = ORG_KDE_KWIN_OUTPUTDEVICE_ENABLEMENT_DISABLED;
    if (enable == OutputDevice::Enablement::Enabled) {
        _enable = ORG_KDE_KWIN_OUTPUTDEVICE_ENABLEMENT_ENABLED;
    }
    org_kde_kwin_outputdevice *od = outputdevice->output();
    org_kde_kwin_outputconfiguration_enable(d->outputconfiguration, od, _enable);
}

void OutputConfiguration::setMode(OutputDevice* outputdevice, const int modeId)
{
    org_kde_kwin_outputdevice *od = outputdevice->output();
    org_kde_kwin_outputconfiguration_mode(d->outputconfiguration, od,
                                          modeId);
}

void OutputConfiguration::setTransform(OutputDevice *outputdevice, KWayland::Client::OutputDevice::Transform transform)
{
    auto toTransform = [transform]() {
        switch (transform) {
            using KWayland::Client::OutputDevice;
            case KWayland::Client::OutputDevice::Transform::Normal:
                return WL_OUTPUT_TRANSFORM_NORMAL;
            case KWayland::Client::OutputDevice::Transform::Rotated90:
                return WL_OUTPUT_TRANSFORM_90;
            case KWayland::Client::OutputDevice::Transform::Rotated180:
                return WL_OUTPUT_TRANSFORM_180;
            case KWayland::Client::OutputDevice::Transform::Rotated270:
                return WL_OUTPUT_TRANSFORM_270;
            case KWayland::Client::OutputDevice::Transform::Flipped:
                return WL_OUTPUT_TRANSFORM_FLIPPED;
            case KWayland::Client::OutputDevice::Transform::Flipped90:
                return WL_OUTPUT_TRANSFORM_FLIPPED_90;
            case KWayland::Client::OutputDevice::Transform::Flipped180:
                return WL_OUTPUT_TRANSFORM_FLIPPED_180;
            case KWayland::Client::OutputDevice::Transform::Flipped270:
                return WL_OUTPUT_TRANSFORM_FLIPPED_270;
        }
        abort();
    };
    org_kde_kwin_outputdevice *od = outputdevice->output();
    org_kde_kwin_outputconfiguration_transform(d->outputconfiguration, od, toTransform());
}

void OutputConfiguration::setPosition(OutputDevice *outputdevice, const QPoint &pos)
{
    org_kde_kwin_outputdevice *od = outputdevice->output();
    org_kde_kwin_outputconfiguration_position(d->outputconfiguration, od, pos.x(), pos.y());
}

void OutputConfiguration::setScale(OutputDevice *outputdevice, qint32 scale)
{
    setScaleF(outputdevice, scale);
}

void OutputConfiguration::setScaleF(OutputDevice *outputdevice, qreal scale)
{
    org_kde_kwin_outputdevice *od = outputdevice->output();
    if (wl_proxy_get_version(d->outputconfiguration) < ORG_KDE_KWIN_OUTPUTCONFIGURATION_SCALEF_SINCE_VERSION) {
        org_kde_kwin_outputconfiguration_scale(d->outputconfiguration, od, qRound(scale));
    } else {
        org_kde_kwin_outputconfiguration_scalef(d->outputconfiguration, od, wl_fixed_from_double(scale));
    }
}

void OutputConfiguration::apply()
{
    org_kde_kwin_outputconfiguration_apply(d->outputconfiguration);
}

// Callbacks
org_kde_kwin_outputconfiguration_listener OutputConfiguration::Private::s_outputconfigurationListener = {
    appliedCallback,
    failedCallback
};

void OutputConfiguration::Private::appliedCallback(void* data, org_kde_kwin_outputconfiguration* config)
{
    Q_UNUSED(config);
    auto o = reinterpret_cast<OutputConfiguration::Private*>(data);
    emit o->q->applied();
}

void OutputConfiguration::Private::failedCallback(void* data, org_kde_kwin_outputconfiguration* config)
{
    Q_UNUSED(config);
    auto o = reinterpret_cast<OutputConfiguration::Private*>(data);
    emit o->q->failed();
}


}
}

