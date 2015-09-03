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
#include "outputconfiguration.h"
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

class OutputConfiguration::Private
{
public:
    Private(OutputConfiguration *q);
    void setup(org_kde_kwin_outputconfiguration *o);
    EventQueue *queue = nullptr;

    WaylandPointer<org_kde_kwin_outputconfiguration, org_kde_kwin_outputconfiguration_destroy> outputconfiguration;

private:

    static void appliedCallback(void *data, org_kde_kwin_outputconfiguration *outputconfiguration);

    OutputConfiguration *q;
    static struct org_kde_kwin_outputconfiguration_listener s_outputListener;
};

OutputConfiguration::Private::Private(OutputConfiguration *q)
    : outputconfiguration(nullptr)
    , q(q)
{
}

void OutputConfiguration::Private::setup(org_kde_kwin_outputconfiguration *o)
{
    Q_ASSERT(o);
    Q_ASSERT(!outputconfiguration);
    outputconfiguration.setup(o);
    org_kde_kwin_outputconfiguration_add_listener(outputconfiguration, &s_outputListener, this);
}

OutputConfiguration::OutputConfiguration(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

OutputConfiguration::~OutputConfiguration()
{
    release();
}

void OutputConfiguration::destroy()
{
    if (!d->outputconfiguration) {
        return;
    }
    emit interfaceAboutToBeDestroyed();
    d->outputconfiguration.destroy();
}

void OutputConfiguration::release()
{
    if (!d->outputconfiguration) {
        return;
    }
    emit interfaceAboutToBeReleased();
    d->outputconfiguration.release();
}

void OutputConfiguration::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *OutputConfiguration::eventQueue()
{
    return d->queue;
}

org_kde_kwin_outputconfiguration_listener OutputConfiguration::Private::s_outputListener = {
    appliedCallback
};

void OutputConfiguration::Private::appliedCallback(void* data, org_kde_kwin_outputconfiguration* outputconfiguration)
{
    auto o = reinterpret_cast<OutputConfiguration::Private*>(data);
    Q_ASSERT(o->outputconfiguration == outputconfiguration);
    emit o->q->applied();
}

void OutputConfiguration::setup(org_kde_kwin_outputconfiguration *output)
{
    d->setup(output);
}

bool OutputConfiguration::isValid() const
{
    return d->outputconfiguration.isValid();
}

OutputConfiguration::operator org_kde_kwin_outputconfiguration*() {
    return d->outputconfiguration;
}

OutputConfiguration::operator org_kde_kwin_outputconfiguration*() const {
    return d->outputconfiguration;
}


}
}
