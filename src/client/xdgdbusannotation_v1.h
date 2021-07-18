/*
    SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWAYLAND_CLIENT_XDG_DBUS_ANNOTATION_V1_H
#define KWAYLAND_CLIENT_XDG_DBUS_ANNOTATION_V1_H

#include <QObject>

#include <KWayland/Client/kwaylandclient_export.h>

struct xdg_dbus_annotation_manager_v1;
struct xdg_dbus_annotation_v1;

namespace KWayland
{
namespace Client
{

class EventQueue;
class XdgShellSurface;
class XdgDBusAnnotationV1;

class XdgDBusAnnotationManagerV1Private;
class XdgDBusAnnotationV1Private;

class KWAYLANDCLIENT_EXPORT XdgDBusAnnotationManagerV1 : public QObject
{
    Q_OBJECT
public:

    explicit XdgDBusAnnotationManagerV1(QObject *parent = nullptr);
    virtual ~XdgDBusAnnotationManagerV1();

    void setup(xdg_dbus_annotation_manager_v1 *appmenumanager);
    bool isValid() const;
    void release();
    void destroy();

    void setEventQueue(EventQueue *queue);
    EventQueue *eventQueue();

    XdgDBusAnnotationV1 *createForToplevel(XdgShellSurface *surface, const QString& name, QObject *parent = nullptr);
    XdgDBusAnnotationV1 *createForClient(const QString& name, QObject *parent = nullptr);

    operator xdg_dbus_annotation_manager_v1 *();
    operator xdg_dbus_annotation_manager_v1 *() const;

Q_SIGNALS:
    void removed();

private:
    QScopedPointer<XdgDBusAnnotationManagerV1Private> d;
};

class KWAYLANDCLIENT_EXPORT XdgDBusAnnotationV1 : public QObject
{
    Q_OBJECT
public:
    virtual ~XdgDBusAnnotationV1();

    void setup(xdg_dbus_annotation_v1 *appmenu);
    bool isValid() const;
    void release();
    void destroy();

    void setAddress(const QString &serviceName, const QString &objectPath);

    operator xdg_dbus_annotation_v1*();
    operator xdg_dbus_annotation_v1*() const;

private:
    friend class XdgDBusAnnotationManagerV1;
    explicit XdgDBusAnnotationV1(QObject *parent = nullptr);

    QScopedPointer<XdgDBusAnnotationV1Private> d;
};

}
}

#endif
