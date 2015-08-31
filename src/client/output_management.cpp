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
#include "wayland-org_kde_kwin_output_management-client-protocol.h"

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

    QList<DisabledOutput*> disabledOutputs;

private:
    static void disabledOutputAddedCallback(void *data, org_kde_kwin_output_management *output,
                                       const char *edid,
                                       const char *name,
                                       const char *connector);

    static void disabledOutputRemovedCallback(void *data, org_kde_kwin_output_management *output,
                                              const char *name,
                                              const char *connector);

    /*
     *        <arg name="id" type="int" summary="the unique output's device ID"/>
     *        <arg name="width" type="int" summary="current resolution's height in pixel"/>
     *        <arg name="height" type="int" summary="current resolution's width in pixel"/>
     *        <arg name="x" type="int" summary="the outputs EDID string"/>
     *        <arg name="y" type="int" summary="output's name"/>
     *        <arg name="enabled" type="int" summary="is the output currently used to display screen content?"/>
     *        <arg name="primary" type="int" summary="is the output the primary display?"/>
     *        <arg name="rotation" type="int" summary="output rotation in degree"/>
     *
     */
    static void outputDeviceAddedCallback(void *data, org_kde_kwin_output_management *sm,
                                          const int id,
                                          const int width,
                                          const int height,
                                          const int x,
                                          const int y,
                                          const int enabled, /* a bool, really */
                                          const int primary, /* also a bool */
                                          const int rotation);
    /*          <arg name="id" type="int" summary="the ID of the output for this EDID information"/>
     *        <arg name="eisa_id" type="string" summary="EISA ID of the output device"/>
     *        <arg name="monitor_name" type="string" summary="human-readable name of the output device"/>
     *        <arg name="serial_number" type="string" summary="serial number of the output device"/>
     *        <arg name="physical_width" type="int" summary="physical width in millimeter"/>
     *        <arg name="physical_height" type="int" summary="physical height in millimeter"/>
     */
    static void edidCallback(void *data, org_kde_kwin_output_management *sm,
                             const int id,
                             const char *eisa_id,
                             const char *monitor_name,
                             const char *serial_number,
                             const int physical_width,
                             const int physical_height);

    static void modeCallback(void *data, org_kde_kwin_output_management *sm,
                             const int id,
                             const int width,
                             const int height,
                             const int refresh_rate);

    static void outputDeviceRemovedCallback(void *data, org_kde_kwin_output_management *sm,
                                            const int id);

    static void doneCallback(void *data, org_kde_kwin_output_management *output);

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
    qDeleteAll(d->disabledOutputs);
}

void OutputManagement::destroy()
{
    //qDebug() << "SM destroy" << d->output_management.isValid();
    if (!d->output_management) {
        return;
    }
    emit interfaceAboutToBeDestroyed();
    d->output_management.destroy();
}

void OutputManagement::release()
{
    //qDebug() << "SM release";
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
    /* these two just left in for testing right now, remove later */
    disabledOutputAddedCallback,
    disabledOutputRemovedCallback,
    /* the following are for real */
    outputDeviceAddedCallback,
    edidCallback,
    modeCallback,
    outputDeviceRemovedCallback,
    doneCallback
};

void OutputManagement::Private::disabledOutputAddedCallback(void* data, org_kde_kwin_output_management* output, const char* edid, const char* name, const char* connector)
{
    //qDebug() << "disabledOutputAddedCallback!" << name << connector;
    auto o = reinterpret_cast<OutputManagement::Private*>(data);
    Q_ASSERT(o->output_management == output);

    DisabledOutput *op = new DisabledOutput(o->q);
    op->setEdid(edid);
    op->setName(name);
    op->setConnector(connector);

    o->disabledOutputs << op;

    emit o->q->disabledOutputAdded(op);
}

void OutputManagement::Private::disabledOutputRemovedCallback(void* data, org_kde_kwin_output_management* output, const char* name, const char* connector)
{
    //qDebug() << "disabledOutputRemovedCallback!" << name << connector;
    auto o = reinterpret_cast<OutputManagement::Private*>(data);
    Q_ASSERT(o->output_management == output);

    auto it = std::find_if(o->disabledOutputs.begin(), o->disabledOutputs.end(),
                           [name, connector](const DisabledOutput *r) {
                               return (r->name() == name && r->connector() == connector);
                        });
    o->disabledOutputs.erase(it);

    emit o->q->disabledOutputRemoved(*it);
}

void OutputManagement::Private::outputDeviceAddedCallback(void* data, org_kde_kwin_output_management* sm, const int id, const int width, const int height, const int x, const int y, const int enabled, const int primary, const int rotation)
{
    qDebug() << "OutputDeviceAdded!" << id << width << height;

}

void OutputManagement::Private::edidCallback(void* data, org_kde_kwin_output_management* sm, const int id, const char* eisa_id, const char* monitor_name, const char* serial_number, const int physical_width, const int physical_height)
{
    qDebug() << "Edid arrived" << id << monitor_name;
}

void OutputManagement::Private::modeCallback(void* data, org_kde_kwin_output_management* sm, const int id, const int width, const int height, const int refresh_rate)
{
    qDebug() << "modeCallback" << id << width << height << refresh_rate;
}


void OutputManagement::Private::outputDeviceRemovedCallback(void* data, org_kde_kwin_output_management* output, const int id)
{
    qDebug() << "OutputDeviceRemoved!" << id;
}

void OutputManagement::Private::doneCallback(void* data, org_kde_kwin_output_management* output)
{
    auto o = reinterpret_cast<OutputManagement::Private*>(data);
    Q_ASSERT(o->output_management == output);
    emit o->q->done();
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

QList< DisabledOutput* > OutputManagement::disabledOutputs() const
{
    return d->disabledOutputs;
}


}
}
