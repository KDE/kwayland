/*
    SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "event_queue.h"
#include "xdgdbusannotation_v1_p.h"
#include "xdgshell.h"

namespace KWayland
{

namespace Client
{

XdgDBusAnnotationManagerV1::XdgDBusAnnotationManagerV1(QObject *parent)
    : QObject(parent)
    , d(new XdgDBusAnnotationManagerV1Private)
{

}
XdgDBusAnnotationManagerV1::~XdgDBusAnnotationManagerV1()
{

}

void XdgDBusAnnotationManagerV1::setup(xdg_dbus_annotation_manager_v1 *annotationManager)
{
    d->setup(annotationManager);
}

bool XdgDBusAnnotationManagerV1::isValid() const
{
    return d->annotationManager.isValid();
}

void XdgDBusAnnotationManagerV1::release()
{
    d->annotationManager.release();
}

void XdgDBusAnnotationManagerV1::destroy()
{
    d->annotationManager.destroy();
}

void XdgDBusAnnotationManagerV1::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *XdgDBusAnnotationManagerV1::eventQueue()
{
    return d->queue;
}

XdgDBusAnnotationV1 *XdgDBusAnnotationManagerV1::createForToplevel(XdgShellSurface *surface, const QString& name, QObject *parent)
{
    Q_ASSERT(isValid());
    auto p = new XdgDBusAnnotationV1(parent);
    auto data = name.toLatin1();
    auto w = xdg_dbus_annotation_manager_v1_create(d->annotationManager, data.constData(), *surface);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    p->setup(w);
    return p;
}

XdgDBusAnnotationV1 *XdgDBusAnnotationManagerV1::createForClient(const QString& name, QObject *parent)
{
    Q_ASSERT(isValid());
    auto p = new XdgDBusAnnotationV1(parent);
    auto data = name.toLatin1();
    auto w = xdg_dbus_annotation_manager_v1_create(d->annotationManager, data.constData(), nullptr);
    if (d->queue) {
        d->queue->addProxy(w);
    }
    p->setup(w);
    return p;
}

XdgDBusAnnotationManagerV1::operator xdg_dbus_annotation_manager_v1 *()
{
    return d->annotationManager;
}

XdgDBusAnnotationManagerV1::operator xdg_dbus_annotation_manager_v1 *() const
{
    return d->annotationManager;
}

XdgDBusAnnotationV1::~XdgDBusAnnotationV1()
{

}

void XdgDBusAnnotationV1::setup(xdg_dbus_annotation_v1 *appmenu)
{
    d->annotation.setup(appmenu);
}

bool XdgDBusAnnotationV1::isValid() const
{
    return d->annotation.isValid();
}

void XdgDBusAnnotationV1::release()
{
    d->annotation.release();
}

void XdgDBusAnnotationV1::destroy()
{
    d->annotation.destroy();
}

void XdgDBusAnnotationV1::setAddress(const QString &serviceName, const QString &objectPath)
{
    auto nameData = serviceName.toLatin1();
    auto objectData = objectPath.toLatin1();
    xdg_dbus_annotation_v1_set_address(d->annotation, nameData.constData(), objectData.constData());
}

XdgDBusAnnotationV1::operator xdg_dbus_annotation_v1*()
{
    return d->annotation;
}

XdgDBusAnnotationV1::operator xdg_dbus_annotation_v1*() const
{
    return d->annotation;
}

XdgDBusAnnotationV1::XdgDBusAnnotationV1(QObject *parent)
    : QObject(parent)
    , d(new XdgDBusAnnotationV1Private)
{

}

};

};
