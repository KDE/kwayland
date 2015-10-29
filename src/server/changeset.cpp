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
    Private(ChangeSet *parent);
    ~Private();

    ChangeSet *q;
    bool enabledChanged = false;
    bool modeChanged = false;
    bool transformChanged = false;
    bool positionChanged = false;
    bool scaleChanged = false;

    OutputDeviceInterface::Enablement enabled = OutputDeviceInterface::Enablement::Enabled;
    int modeId = -1;
    OutputDeviceInterface::Transform transform = OutputDeviceInterface::Transform::Normal;
    QPoint position;
    int scale = 1;
};


ChangeSet::Private::Private(ChangeSet *parent)
    : q(parent)
{
}

ChangeSet::Private::~Private()
{
}

ChangeSet::ChangeSet(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
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
    return d->enabledChanged;
}

OutputDeviceInterface::Enablement ChangeSet::enabled() const
{
    Q_D();
    return d->enabled;
}

bool ChangeSet::modeChanged() const
{
    Q_D();
    return d->modeChanged;
}

int ChangeSet::mode() const
{
    Q_D();
    return d->modeId;
}

bool ChangeSet::transformChanged() const
{
    Q_D();
    return d->transformChanged;
}

OutputDeviceInterface::Transform ChangeSet::transform() const
{
    Q_D();
    return d->transform;
}

bool ChangeSet::positionChanged() const
{
    Q_D();
    return d->positionChanged;
}

QPoint ChangeSet::position() const
{
    Q_D();
    return d->position;
}

bool ChangeSet::scaleChanged() const
{
    Q_D();
    return d->scaleChanged;
}

int ChangeSet::scale() const
{
    Q_D();
    return d->scale;
}


}
}
