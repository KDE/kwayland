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
#include "kwin_screen_management.h"
#include "wayland_pointer_p.h"
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

    WaylandPointer<org_kde_kwin_screen_management, org_kde_kwin_screen_management_destroy> output;

private:
    static void outputAppearedCallback(void *data, org_kde_kwin_screen_management *output,
                                       const char *edid,
                                       const char *name,
                                       const char *connector);

    static void outputDisappearedCallback(void *data, org_kde_kwin_screen_management *output,
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
    Q_ASSERT(!output);
    output.setup(o);
    org_kde_kwin_screen_management_add_listener(output, &s_outputListener, this);
}

KWinScreenManagement::KWinScreenManagement(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

KWinScreenManagement::~KWinScreenManagement()
{
    d->output.release();
}

org_kde_kwin_screen_management_listener KWinScreenManagement::Private::s_outputListener = {
    outputAppearedCallback,
    outputDisappearedCallback,
    doneCallback
};

void KWinScreenManagement::Private::outputAppearedCallback(void* data, org_kde_kwin_screen_management* output, const char* edid, const char* name, const char* connector)
{
    qDebug() << "outputAppearedCallback!" << name << connector;
    auto o = reinterpret_cast<KWinScreenManagement::Private*>(data);
    Q_ASSERT(o->output == output);

    emit o->q->outputAppeared(QString::fromLocal8Bit(edid), QString::fromLocal8Bit(name), QString::fromLocal8Bit(connector));

}

void KWinScreenManagement::Private::outputDisappearedCallback(void* data, org_kde_kwin_screen_management* output, const char* name, const char* connector)
{
    qDebug() << "outputDisappearedCallback! FIXME" << name << connector;
}

void KWinScreenManagement::Private::doneCallback(void* data, org_kde_kwin_screen_management* output)
{
    auto o = reinterpret_cast<KWinScreenManagement::Private*>(data);
    Q_ASSERT(o->output == output);
    emit o->q->done();
}

void KWinScreenManagement::setup(org_kde_kwin_screen_management *output)
{
    d->setup(output);
}

org_kde_kwin_screen_management *KWinScreenManagement::output()
{
    return d->output;
}

bool KWinScreenManagement::isValid() const
{
    return d->output.isValid();
}

KWinScreenManagement::operator org_kde_kwin_screen_management*() {
    return d->output;
}

KWinScreenManagement::operator org_kde_kwin_screen_management*() const {
    return d->output;
}

}
}
