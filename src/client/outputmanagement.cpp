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
#include "outputmanagement.h"
#include "outputconfiguration.h"
#include "event_queue.h"
#include "wayland_pointer_p.h"
#include "wayland-output-management-client-protocol.h"


namespace KWayland
{
namespace Client
{

class OutputManagement::Private
{
public:
    Private() = default;

    WaylandPointer<org_kde_kwin_outputmanagement, org_kde_kwin_outputmanagement_destroy> outputmanagement;
    EventQueue *queue = nullptr;
};

OutputManagement::OutputManagement(QObject *parent)
: QObject(parent)
, d(new Private)
{
}

OutputManagement::~OutputManagement()
{
    d->outputmanagement.release();
}

void OutputManagement::setup(org_kde_kwin_outputmanagement *outputmanagement)
{
    Q_ASSERT(outputmanagement);
    Q_ASSERT(!d->outputmanagement);
    d->outputmanagement.setup(outputmanagement);
}

void OutputManagement::release()
{
    d->outputmanagement.release();
}

void OutputManagement::destroy()
{
    d->outputmanagement.destroy();
}

void OutputManagement::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *OutputManagement::eventQueue()
{
    return d->queue;
}

OutputManagement::operator org_kde_kwin_outputmanagement*() {
    return d->outputmanagement;
}

OutputManagement::operator org_kde_kwin_outputmanagement*() const {
    return d->outputmanagement;
}

bool OutputManagement::isValid() const
{
    return d->outputmanagement.isValid();
}

OutputConfiguration *OutputManagement::createConfiguration(QObject *parent)
{
    Q_UNUSED(parent);
    OutputConfiguration *config = new OutputConfiguration(this);
    auto w = org_kde_kwin_outputmanagement_create_configuration(d->outputmanagement);

    if (d->queue) {
        d->queue->addProxy(w);
    }

    config->setup(w);
    return config;
}


}
}
