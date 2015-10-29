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

#include "changeset.h"

namespace KWayland
{
namespace Server
{

class ChangeSet::Private
{
public:
    Private(OutputDeviceInterface *outputdevice, ChangeSet *parent);
    ~Private();

    ChangeSet *q;
    OutputDeviceInterface *o;

    OutputDeviceInterface::Enablement enabled;
    int modeId;
    OutputDeviceInterface::Transform transform;
    QPoint position;
    int scale;
};


ChangeSet::Private::Private(OutputDeviceInterface *outputdevice, ChangeSet *parent)
    : q(parent)
    , o(outputdevice)
    , enabled(o->enabled())
    , modeId(o->currentModeId())
    , transform(o->transform())
    , position(o->globalPosition())
    , scale(o->scale())
{
}

ChangeSet::Private::~Private()
{
}

ChangeSet::ChangeSet(OutputDeviceInterface *outputdevice, QObject *parent)
    : QObject(parent)
    , d(new Private(outputdevice, this))
{
}

ChangeSet::~ChangeSet()
{
}

ChangeSet::Private *ChangeSet::d_func() const
{
    return reinterpret_cast<Private*>(d.data());
}

bool ChangeSet::enabledChanged() const
{
    Q_D();
    return d->enabled == d->o->enabled();
}

OutputDeviceInterface::Enablement ChangeSet::enabled() const
{
    Q_D();
    return d->enabled;
}

bool ChangeSet::modeChanged() const
{
    Q_D();
    return d->modeId != d->o->currentModeId();
}

int ChangeSet::mode() const
{
    Q_D();
    return d->modeId;
}

bool ChangeSet::transformChanged() const
{
    Q_D();
    return d->transform != d->o->transform();
}

OutputDeviceInterface::Transform ChangeSet::transform() const
{
    Q_D();
    return d->transform;
}

bool ChangeSet::positionChanged() const
{
    Q_D();
    return d->position != d->o->globalPosition();
}

QPoint ChangeSet::position() const
{
    Q_D();
    return d->position;
}

bool ChangeSet::scaleChanged() const
{
    Q_D();
    return d->scale != d->o->scale();
}

int ChangeSet::scale() const
{
    Q_D();
    return d->scale;
}


}
}
