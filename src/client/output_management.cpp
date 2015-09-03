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
#include "output_management.h"
#include "wayland_pointer_p.h"
#include "event_queue.h"
// Qt
#include <QPoint>
#include <QRect>
#include <QSize>
// wayland
#include <wayland-client-protocol.h>
#include "wayland-output-management-client-protocol.h"

#include <QDebug>

namespace KWayland
{

namespace Client
{

class OutputManagement::Private
{
public:
    Private(OutputManagement *q);
    void setup(org_kde_kwin_output_management *o);
    EventQueue *queue = nullptr;

    WaylandPointer<org_kde_kwin_output_management, org_kde_kwin_output_management_destroy> output_management;

private:

    static void configurationCreatedCallback(void *data, org_kde_kwin_output_management *output_management, org_kde_kwin_outputconfiguration *config);

    OutputManagement *q;
    static struct org_kde_kwin_output_management_listener s_outputListener;
};

OutputManagement::Private::Private(OutputManagement *q)
    : output_management(nullptr)
    , q(q)
{
}

void OutputManagement::Private::setup(org_kde_kwin_output_management *o)
{
    Q_ASSERT(o);
    Q_ASSERT(!output_management);
    output_management.setup(o);
    org_kde_kwin_output_management_add_listener(output_management, &s_outputListener, this);
}

OutputManagement::OutputManagement(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

OutputManagement::~OutputManagement()
{
    release();
}

void OutputManagement::destroy()
{
    if (!d->output_management) {
        return;
    }
    emit interfaceAboutToBeDestroyed();
    d->output_management.destroy();
}

void OutputManagement::release()
{
    if (!d->output_management) {
        return;
    }
    emit interfaceAboutToBeReleased();
    d->output_management.release();
}

void OutputManagement::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *OutputManagement::eventQueue()
{
    return d->queue;
}

org_kde_kwin_output_management_listener OutputManagement::Private::s_outputListener = {
    configurationCreatedCallback
};

void OutputManagement::Private::configurationCreatedCallback(void* data, org_kde_kwin_output_management* output, org_kde_kwin_outputconfiguration* outputconfiguration)
{
    auto o = reinterpret_cast<OutputManagement::Private*>(data);
    Q_ASSERT(o->output_management == output);
    //emit o->q->done(); FIXME
}

void OutputManagement::createConfiguration()
{
    org_kde_kwin_output_management_create_configuration(d->output_management);
}


void OutputManagement::setup(org_kde_kwin_output_management *output)
{
    d->setup(output);
}

bool OutputManagement::isValid() const
{
    return d->output_management.isValid();
}

OutputManagement::operator org_kde_kwin_output_management*() {
    return d->output_management;
}

OutputManagement::operator org_kde_kwin_output_management*() const {
    return d->output_management;
}


}
}
