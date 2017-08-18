/****************************************************************************
Copyright 2017  Marco Martin <notmart@gmail.com>

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
****************************************************************************/

#include "xdgforeign_interface.h"
#include "xdgforeign_v1_interface_p.h"
#include "display.h"
#include "global_p.h"
#include "resource_p.h"
#include "surface_interface_p.h"

#include "wayland-xdg-foreign-unstable-v1-server-protocol.h"

#include <QUuid>
#include <QDebug>

namespace KWayland
{
namespace Server
{

XdgForeignUnstableInterface::Private::Private(Display *display, XdgForeignUnstableInterface *q)
    : q(q)
{
    exporter = new XdgExporterUnstableV1Interface(display, q);
    importer = new XdgImporterUnstableV1Interface(display, q);

    connect(importer, &XdgImporterUnstableV1Interface::transientChanged,
        q, &XdgForeignUnstableInterface::transientChanged);
}

XdgForeignUnstableInterface::XdgForeignUnstableInterface(Display *display, QObject *parent)
    : QObject(parent),
      d(new Private(display, this))
{
}

XdgForeignUnstableInterface::~XdgForeignUnstableInterface()
{
    delete d->exporter;
    delete d->importer;
    delete d;
}

void XdgForeignUnstableInterface::create()
{
    d->exporter->create();
    d->importer->create();
}

bool XdgForeignUnstableInterface::isValid()
{
    return d->exporter->isValid() && d->importer->isValid();
}

SurfaceInterface *XdgForeignUnstableInterface::transientFor(SurfaceInterface *surface)
{
    return d->importer->transientFor(surface);
}

}
}

