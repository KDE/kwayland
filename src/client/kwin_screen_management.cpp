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
#include "kwin_screen_management.h"
#include "wayland_pointer_p.h"
#include "event_queue.h"
// Qt
#include <QPoint>
#include <QRect>
#include <QSize>
// wayland
#include <wayland-client-protocol.h>
#include "wayland-org_kde_kwin_screen_management-client-protocol.h"

#include <QDebug>

namespace KWayland
{

namespace Client
{

class KWinScreenManagement::Private
{
public:
    Private(KWinScreenManagement *q);
    void setup(org_kde_kwin_screen_management *o);
    EventQueue *queue = nullptr;

    WaylandPointer<org_kde_kwin_screen_management, org_kde_kwin_screen_management_destroy> screen_management;

    QList<DisabledOutput*> disabledOutputs;

private:
    static void disabledOutputAddedCallback(void *data, org_kde_kwin_screen_management *output,
                                       const char *edid,
                                       const char *name,
                                       const char *connector);

    static void disabledOutputRemovedCallback(void *data, org_kde_kwin_screen_management *output,
                                          const char *name,
                                          const char *connector);

    static void doneCallback(void *data, org_kde_kwin_screen_management *output);

    KWinScreenManagement *q;
    static struct org_kde_kwin_screen_management_listener s_outputListener;
};

KWinScreenManagement::Private::Private(KWinScreenManagement *q)
    : q(q)
{
}

void KWinScreenManagement::Private::setup(org_kde_kwin_screen_management *o)
{
    Q_ASSERT(o);
    Q_ASSERT(!screen_management);
    screen_management.setup(o);
    org_kde_kwin_screen_management_add_listener(screen_management, &s_outputListener, this);
}

KWinScreenManagement::KWinScreenManagement(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

KWinScreenManagement::~KWinScreenManagement()
{
    d->screen_management.release();
    qDeleteAll(d->disabledOutputs);
}

void KWinScreenManagement::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *KWinScreenManagement::eventQueue()
{
    return d->queue;
}

org_kde_kwin_screen_management_listener KWinScreenManagement::Private::s_outputListener = {
    disabledOutputAddedCallback,
    disabledOutputRemovedCallback,
    doneCallback
};

void KWinScreenManagement::Private::disabledOutputAddedCallback(void* data, org_kde_kwin_screen_management* output, const char* edid, const char* name, const char* connector)
{
    qDebug() << "disabledOutputAddedCallback!" << name << connector;
    auto o = reinterpret_cast<KWinScreenManagement::Private*>(data);
    Q_ASSERT(o->screen_management == output);

    DisabledOutput *op = new DisabledOutput(o->q);
    op->setEdid(edid);
    op->setName(name);
    op->setConnector(connector);

    o->disabledOutputs << op;

    emit o->q->disabledOutputAdded(QString::fromLocal8Bit(edid), QString::fromLocal8Bit(name), QString::fromLocal8Bit(connector));

}

void KWinScreenManagement::Private::disabledOutputRemovedCallback(void* data, org_kde_kwin_screen_management* output, const char* name, const char* connector)
{
    qDebug() << "disabledOutputRemovedCallback! FIXME" << name << connector;
    auto o = reinterpret_cast<KWinScreenManagement::Private*>(data);
    Q_ASSERT(o->screen_management == output);

    DisabledOutput *op = new DisabledOutput();
    op->setName(name);
    op->setConnector(connector);

    auto it = std::find_if(o->disabledOutputs.begin(), o->disabledOutputs.end(),
                           [op](const DisabledOutput *r) {
                               return (r->name() == op->name() && r->connector() == op->connector());
                        });
    o->disabledOutputs.erase(it);

    emit o->q->disabledOutputRemoved(QString::fromLocal8Bit(name), QString::fromLocal8Bit(connector));
}

void KWinScreenManagement::Private::doneCallback(void* data, org_kde_kwin_screen_management* output)
{
    auto o = reinterpret_cast<KWinScreenManagement::Private*>(data);
    Q_ASSERT(o->screen_management == output);
    emit o->q->done();
}

void KWinScreenManagement::setup(org_kde_kwin_screen_management *output)
{
    d->setup(output);
}

org_kde_kwin_screen_management *KWinScreenManagement::screen_management()
{
    return d->screen_management;
}

bool KWinScreenManagement::isValid() const
{
    return d->screen_management.isValid();
}

KWinScreenManagement::operator org_kde_kwin_screen_management*() {
    return d->screen_management;
}

KWinScreenManagement::operator org_kde_kwin_screen_management*() const {
    return d->screen_management;
}

QList< DisabledOutput* > KWinScreenManagement::disabledOutputs() const
{
    return d->disabledOutputs;
}


}
}
