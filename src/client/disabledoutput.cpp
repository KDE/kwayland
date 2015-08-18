/********************************************************************
Copyright 2013  Martin Gräßlin <mgraesslin@kde.org>
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
#include "disabledoutput.h"
#include "wayland_pointer_p.h"
// Qt
#include <QPoint>
#include <QRect>
#include <QSize>
// wayland
#include <wayland-client-protocol.h>

namespace KWayland
{

namespace Client
{

class DisabledOutput::Private
{
public:
    Private(DisabledOutput *q);

    QByteArray edid;
    QString name;
    QString connector;

    DisabledOutput *q;
};

DisabledOutput::Private::Private(DisabledOutput *q)
    : q(q)
{
}

DisabledOutput::DisabledOutput(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

DisabledOutput::~DisabledOutput()
{
}


QByteArray DisabledOutput::edid() const
{
    return d->edid;
}

void DisabledOutput::setEdid(const QByteArray& e)
{
    d->edid = e;
}

QString DisabledOutput::name() const
{
    return d->name;
}

void DisabledOutput::setName(const QString& n)
{
    d->name = n;
}

QString DisabledOutput::connector() const
{
    return d->connector;
}

void DisabledOutput::setConnector(const QString& c)
{
    d->connector = c;
}



}
}
