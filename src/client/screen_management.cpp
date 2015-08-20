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
#include "screen_management.h"
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

class ScreenManagement::Private
{
public:
    Private(ScreenManagement *q);
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

    ScreenManagement *q;
    static struct org_kde_kwin_screen_management_listener s_outputListener;
};

ScreenManagement::Private::Private(ScreenManagement *q)
    : q(q)
{
}

void ScreenManagement::Private::setup(org_kde_kwin_screen_management *o)
{
    Q_ASSERT(o);
    Q_ASSERT(!screen_management);
    screen_management.setup(o);
    org_kde_kwin_screen_management_add_listener(screen_management, &s_outputListener, this);
}

ScreenManagement::ScreenManagement(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

ScreenManagement::~ScreenManagement()
{
    d->screen_management.release();
    qDeleteAll(d->disabledOutputs);
}

void ScreenManagement::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *ScreenManagement::eventQueue()
{
    return d->queue;
}

org_kde_kwin_screen_management_listener ScreenManagement::Private::s_outputListener = {
    disabledOutputAddedCallback,
    disabledOutputRemovedCallback,
    doneCallback
};

void ScreenManagement::Private::disabledOutputAddedCallback(void* data, org_kde_kwin_screen_management* output, const char* edid, const char* name, const char* connector)
{
    qDebug() << "disabledOutputAddedCallback!" << name << connector;
    auto o = reinterpret_cast<ScreenManagement::Private*>(data);
    Q_ASSERT(o->screen_management == output);

    DisabledOutput *op = new DisabledOutput(o->q);
    op->setEdid(edid);
    op->setName(name);
    op->setConnector(connector);

    o->disabledOutputs << op;

    emit o->q->disabledOutputAdded(QString::fromLocal8Bit(edid), QString::fromLocal8Bit(name), QString::fromLocal8Bit(connector));

}

void ScreenManagement::Private::disabledOutputRemovedCallback(void* data, org_kde_kwin_screen_management* output, const char* name, const char* connector)
{
    qDebug() << "disabledOutputRemovedCallback! FIXME" << name << connector;
    auto o = reinterpret_cast<ScreenManagement::Private*>(data);
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

void ScreenManagement::Private::doneCallback(void* data, org_kde_kwin_screen_management* output)
{
    auto o = reinterpret_cast<ScreenManagement::Private*>(data);
    Q_ASSERT(o->screen_management == output);
    emit o->q->done();
}

void ScreenManagement::setup(org_kde_kwin_screen_management *output)
{
    d->setup(output);
}

org_kde_kwin_screen_management *ScreenManagement::screen_management()
{
    return d->screen_management;
}

bool ScreenManagement::isValid() const
{
    return d->screen_management.isValid();
}

ScreenManagement::operator org_kde_kwin_screen_management*() {
    return d->screen_management;
}

ScreenManagement::operator org_kde_kwin_screen_management*() const {
    return d->screen_management;
}

QList< DisabledOutput* > ScreenManagement::disabledOutputs() const
{
    return d->disabledOutputs;
}


}
}
