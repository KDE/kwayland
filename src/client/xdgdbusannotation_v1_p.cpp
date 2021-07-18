/*
    SPDX-FileCopyrightText: 2021 Carson Black <uhhadd@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "xdgdbusannotation_v1_p.h"

namespace KWayland
{

namespace Client
{

void XdgDBusAnnotationManagerV1Private::setup(xdg_dbus_annotation_manager_v1 *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!annotationManager);
    annotationManager.setup(arg);
}

void XdgDBusAnnotationV1Private::setup(xdg_dbus_annotation_v1 *arg)
{
    Q_ASSERT(arg);
    Q_ASSERT(!annotation);
    annotation.setup(arg);
}

};

};
