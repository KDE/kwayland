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

#include <QDebug>
#include "wayland-output-management-client-protocol.h"
#include "wayland-org_kde_kwin_outputdevice-client-protocol.h"

namespace KWayland
{
namespace Client
{


class OutputConfiguration::Private
{
public:
    Private() = default;

    WaylandPointer<org_kde_kwin_outputconfiguration, org_kde_kwin_outputconfiguration_destroy> outputconfiguration;
    EventQueue *queue = nullptr;
};

OutputConfiguration::OutputConfiguration(QObject *parent)
: QObject(parent)
, d(new Private)
{
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

void OutputConfiguration::setEnabled(OutputDevice *outputdevice, qint32 enable)
{
    qDebug() << " => " << outputdevice << enable;
    //org_kde_plasma_surface_set_position(d->surface, point.x(), point.y());
    org_kde_kwin_outputdevice *od = outputdevice->output();
    org_kde_kwin_outputconfiguration_enable(d->outputconfiguration, od, enable);

}

void OutputConfiguration::setMode(OutputDevice *outputdevice, qint32 modeId)
{
}

void OutputConfiguration::setTransform(OutputDevice *outputdevice, qint32 transform)
{
}

void OutputConfiguration::setPosition(OutputDevice *outputdevice, qint32 x, qint32 y)
{
}

void OutputConfiguration::setScale(OutputDevice *outputdevice, qint32 scale)
{
}

void OutputConfiguration::apply()
{
}


}
}

