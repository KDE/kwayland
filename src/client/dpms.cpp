/********************************************************************
Copyright 2015  Martin Gräßlin <mgraesslin@kde.org>

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
#include "dpms.h"
#include "event_queue.h"
#include "output.h"
#include "wayland_pointer_p.h"

#include <wayland-client-protocol.h>
#include <wayland-dpms-client-protocol.h>

namespace KWayland
{
namespace Client
{

class DpmsManager::Private
{
public:
    WaylandPointer<org_kde_kwin_dpms_manager, org_kde_kwin_dpms_manager_destroy> manager;
    EventQueue *queue = nullptr;
};

DpmsManager::DpmsManager(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
}

DpmsManager::~DpmsManager()
{
    release();
}

void DpmsManager::release()
{
    d->manager.release();
}

void DpmsManager::destroy()
{
    d->manager.destroy();
}

bool DpmsManager::isValid() const
{
    return d->manager.isValid();
}

void DpmsManager::setup(org_kde_kwin_dpms_manager *manager)
{
    Q_ASSERT(manager);
    Q_ASSERT(!d->manager.isValid());
    d->manager.setup(manager);
}

EventQueue *DpmsManager::eventQueue()
{
    return d->queue;
}

void DpmsManager::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

Dpms *DpmsManager::getDpms(Output *output, QObject *parent)
{
    Q_ASSERT(isValid());
    Q_ASSERT(output);
    Dpms *dpms = new Dpms(output, parent);
    auto w = org_kde_kwin_dpms_manager_get(d->manager, *output);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    dpms->setup(w);
    return dpms;
}

DpmsManager::operator org_kde_kwin_dpms_manager*() const
{
    return d->manager;
}

DpmsManager::operator org_kde_kwin_dpms_manager*()
{
    return d->manager;
}


class Dpms::Private
{
public:
    explicit Private(const QPointer<Output> &output, Dpms *q);
    void setup(org_kde_kwin_dpms *d);

    WaylandPointer<org_kde_kwin_dpms, org_kde_kwin_dpms_release> dpms;
    struct Data {
        bool supported = false;
        Mode mode = Mode::On;
        bool supportedChanged = false;
        bool modeChanged = false;
    };
    Data current;
    Data pending;
    QPointer<Output> output;

private:
    static void supportedCallback(void *data, org_kde_kwin_dpms *org_kde_kwin_dpms, uint32_t supported);
    static void modeCallback(void *data, org_kde_kwin_dpms *org_kde_kwin_dpms, uint32_t mode);
    static void doneCallback(void *data, org_kde_kwin_dpms *org_kde_kwin_dpms);
    static const struct org_kde_kwin_dpms_listener s_listener;

    Dpms *q;
};


#ifndef DOXYGEN_SHOULD_SKIP_THIS
const org_kde_kwin_dpms_listener Dpms::Private::s_listener = {
    supportedCallback,
    modeCallback,
    doneCallback
};
#endif

void Dpms::Private::supportedCallback(void *data, org_kde_kwin_dpms *org_kde_kwin_dpms, uint32_t supported)
{
    Q_UNUSED(org_kde_kwin_dpms)
    Private *p = reinterpret_cast<Private*>(data);
    p->pending.supported = supported == 0 ? false : true;
    p->pending.supportedChanged = true;
}

void Dpms::Private::modeCallback(void *data, org_kde_kwin_dpms *org_kde_kwin_dpms, uint32_t mode)
{
    Q_UNUSED(org_kde_kwin_dpms)
    Mode m;
    switch (mode) {
    case ORG_KDE_KWIN_DPMS_MODE_ON:
        m = Mode::On;
        break;
    case ORG_KDE_KWIN_DPMS_MODE_STANDBY:
        m = Mode::Standby;
        break;
    case ORG_KDE_KWIN_DPMS_MODE_SUSPEND:
        m = Mode::Suspend;
        break;
    case ORG_KDE_KWIN_DPMS_MODE_OFF:
        m = Mode::Off;
        break;
    default:
        return;
    }
    Private *p = reinterpret_cast<Private*>(data);
    p->pending.mode = m;
    p->pending.modeChanged = true;
}

void Dpms::Private::doneCallback(void *data, org_kde_kwin_dpms *org_kde_kwin_dpms)
{
    Q_UNUSED(org_kde_kwin_dpms)
    Private *p = reinterpret_cast<Private*>(data);
    const bool supportedChanged = p->pending.supportedChanged && p->pending.supported != p->current.supported;
    const bool modeChanged = p->pending.modeChanged && p->pending.mode != p->current.mode;
    if (supportedChanged) {
        p->current.supported = p->pending.supported;
        emit p->q->supportedChanged();
    }
    if (modeChanged) {
        p->current.mode = p->pending.mode;
        emit p->q->modeChanged();
    }
    p->pending = Data();
}

Dpms::Private::Private(const QPointer<Output> &output, Dpms *q)
    : output(output)
    , q(q)
{
}

void Dpms::Private::setup(org_kde_kwin_dpms *d)
{
    Q_ASSERT(d);
    Q_ASSERT(!dpms.isValid());
    dpms.setup(d);
    org_kde_kwin_dpms_add_listener(dpms, &s_listener, this);
}

Dpms::Dpms(const QPointer<Output> &o, QObject *parent)
    : QObject(parent)
    , d(new Private(o, this))
{
}

Dpms::~Dpms()
{
    release();
}

void Dpms::destroy()
{
    d->dpms.destroy();
}

void Dpms::release()
{
    d->dpms.release();
}

bool Dpms::isValid() const
{
    return d->dpms.isValid();
}

void Dpms::setup(org_kde_kwin_dpms *dpms)
{
    d->setup(dpms);
}

bool Dpms::isSupported() const
{
    return d->current.supported;
}

Dpms::Mode Dpms::mode() const
{
    return d->current.mode;
}

void Dpms::requestMode(Dpms::Mode mode)
{
    uint32_t wlMode;
    switch (mode) {
    case Mode::On:
        wlMode = ORG_KDE_KWIN_DPMS_MODE_ON;
        break;
    case Mode::Standby:
        wlMode = ORG_KDE_KWIN_DPMS_MODE_STANDBY;
        break;
    case Mode::Suspend:
        wlMode = ORG_KDE_KWIN_DPMS_MODE_SUSPEND;
        break;
    case Mode::Off:
        wlMode = ORG_KDE_KWIN_DPMS_MODE_OFF;
        break;
    default:
        Q_UNREACHABLE();
    }
    org_kde_kwin_dpms_set(d->dpms, wlMode);
}

QPointer< Output > Dpms::output() const
{
    return d->output;
}

Dpms::operator org_kde_kwin_dpms*()
{
    return d->dpms;
}

Dpms::operator org_kde_kwin_dpms*() const
{
    return d->dpms;
}

}
}
