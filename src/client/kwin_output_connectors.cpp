/********************************************************************
Copyright 2013  Martin Gräßlin <mgraesslin@kde.org>

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
#include "kwin_output_connectors.h"
#include "wayland_pointer_p.h"
// Qt
#include <QPoint>
#include <QRect>
#include <QSize>
// wayland
#include <wayland-client-protocol.h>
#include "wayland-org_kde_kwin_output_connectors-client-protocol.h"

#include <QDebug>

namespace KWayland
{

namespace Client
{

class KWinOutputConnectors::Private
{
public:
    Private(KWinOutputConnectors *q);
    void setup(org_kde_kwin_output_connectors *o);

    WaylandPointer<org_kde_kwin_output_connectors, org_kde_kwin_output_connectors_destroy> output;

    void getDisabledOutputs();

private:
    static void outputAppearedCallback(void *data, org_kde_kwin_output_connectors *output,
                                       const char *edid,
                                       const char *name,
                                       const char *connector);

    static void outputDisappearedCallback(void *data, org_kde_kwin_output_connectors *output,
                                          const char *name,
                                          const char *connector);

    static void syncCallback(void *data, org_kde_kwin_output_connectors *output);

    KWinOutputConnectors *q;
    static struct org_kde_kwin_output_connectors_listener s_outputListener;
};

KWinOutputConnectors::Private::Private(KWinOutputConnectors *q)
    : q(q)
{
}

void KWinOutputConnectors::Private::setup(org_kde_kwin_output_connectors *o)
{
    Q_ASSERT(o);
    Q_ASSERT(!output);
    output.setup(o);
    org_kde_kwin_output_connectors_add_listener(output, &s_outputListener, this);
}

KWinOutputConnectors::KWinOutputConnectors(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

KWinOutputConnectors::~KWinOutputConnectors()
{
    d->output.release();
}

org_kde_kwin_output_connectors_listener KWinOutputConnectors::Private::s_outputListener = {
    outputAppearedCallback,
    outputDisappearedCallback,
    syncCallback
};

void KWinOutputConnectors::Private::outputAppearedCallback(void* data, org_kde_kwin_output_connectors* output, const char* edid, const char* name, const char* connector)
{
    qDebug() << "outputAppearedCallback!" << name << connector;
    auto o = reinterpret_cast<KWinOutputConnectors::Private*>(data);
    Q_ASSERT(o->output == output);

    emit o->q->outputAppeared(QString::fromLocal8Bit(edid), QString::fromLocal8Bit(name), QString::fromLocal8Bit(connector));

}

void KWinOutputConnectors::Private::outputDisappearedCallback(void* data, org_kde_kwin_output_connectors* output, const char* name, const char* connector)
{
    qDebug() << "outputDisappearedCallback! FIXME" << name << connector;
}

void KWinOutputConnectors::Private::syncCallback(void* data, org_kde_kwin_output_connectors* output)
{
    qDebug() << "Sync! FIXME";
}

void KWinOutputConnectors::setup(org_kde_kwin_output_connectors *output)
{
    d->setup(output);
}

void KWinOutputConnectors::getDisabledOutputs()
{
    return d->getDisabledOutputs();
}

void KWinOutputConnectors::Private::getDisabledOutputs()
{
    qDebug() << "client: get disabled outputs";
    org_kde_kwin_output_connectors_get_disabled_outputs(output);
}

org_kde_kwin_output_connectors *KWinOutputConnectors::output()
{
    return d->output;
}

bool KWinOutputConnectors::isValid() const
{
    return d->output.isValid();
}

KWinOutputConnectors::operator org_kde_kwin_output_connectors*() {
    return d->output;
}

KWinOutputConnectors::operator org_kde_kwin_output_connectors*() const {
    return d->output;
}

}
}
