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

#include "outputchangeset.h"

namespace KWayland
{
namespace Server
{

class OutputChangeSet::Private
{
public:
    Private(OutputDeviceInterface *outputdevice, OutputChangeSet *parent);
    ~Private();

    OutputChangeSet *q;
    OutputDeviceInterface *o;

    OutputDeviceInterface::Enablement enabled;
    int modeId;
    OutputDeviceInterface::Transform transform;
    QPoint position;
    int scale;
};


OutputChangeSet::Private::Private(OutputDeviceInterface *outputdevice, OutputChangeSet *parent)
    : q(parent)
    , o(outputdevice)
    , enabled(o->enabled())
    , modeId(o->currentModeId())
    , transform(o->transform())
    , position(o->globalPosition())
    , scale(o->scale())
{
}

OutputChangeSet::Private::~Private() = default;

OutputChangeSet::OutputChangeSet(OutputDeviceInterface *outputdevice, QObject *parent)
    : QObject(parent)
    , d(new Private(outputdevice, this))
{
}

OutputChangeSet::~OutputChangeSet() = default;

OutputChangeSet::Private *OutputChangeSet::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}

bool OutputChangeSet::enabledChanged() const
{
    Q_D();
    return d->enabled != d->o->enabled();
}

OutputDeviceInterface::Enablement OutputChangeSet::enabled() const
{
    Q_D();
    return d->enabled;
}

void OutputChangeSet::setEnabled(OutputDeviceInterface::Enablement enablement)
{
    d->enabled = enablement;
}

bool OutputChangeSet::modeChanged() const
{
    Q_D();
    return d->modeId != d->o->currentModeId();
}

int OutputChangeSet::mode() const
{
    Q_D();
    return d->modeId;
}

void OutputChangeSet::setMode(int modeId)
{
    d->modeId = modeId;
}

bool OutputChangeSet::transformChanged() const
{
    Q_D();
    return d->transform != d->o->transform();
}

OutputDeviceInterface::Transform OutputChangeSet::transform() const
{
    Q_D();
    return d->transform;
}

void OutputChangeSet::setTransform(OutputDeviceInterface::Transform t)
{
    d->transform = t;
}

bool OutputChangeSet::positionChanged() const
{
    Q_D();
    return d->position != d->o->globalPosition();
}

QPoint OutputChangeSet::position() const
{
    Q_D();
    return d->position;
}

void OutputChangeSet::setPosition(QPoint pos)
{
    d->position = pos;
}

bool OutputChangeSet::scaleChanged() const
{
    Q_D();
    return d->scale != d->o->scale();
}

int OutputChangeSet::scale() const
{
    Q_D();
    return d->scale;
}

void OutputChangeSet::setScale(int scale)
{
    d->scale = scale;
}


}
}
