/*
    SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KWAYLAND_CLIENT_XDG_DBUS_ANNOTATION_V1_P_H
#define KWAYLAND_CLIENT_XDG_DBUS_ANNOTATION_V1_P_H

#include <QtGlobal>

#include "wayland_pointer_p.h"
#include "xdgdbusannotation_v1.h"

#include <wayland-xdg-dbus-annotation-v1-client-protocol.h>

namespace KWayland
{
namespace Client
{

class XdgDBusAnnotationManagerV1Private
{
public:
    XdgDBusAnnotationManagerV1Private() = default;

    void setup(xdg_dbus_annotation_manager_v1 *arg);

    WaylandPointer<xdg_dbus_annotation_manager_v1, xdg_dbus_annotation_manager_v1_destroy> annotationManager;
    EventQueue *queue = nullptr;
};

class XdgDBusAnnotationV1Private
{
public:
    void setup(xdg_dbus_annotation_v1 *arg);

    WaylandPointer<xdg_dbus_annotation_v1, xdg_dbus_annotation_v1_destroy> annotation;
};

};

};

#endif