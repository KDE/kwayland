/********************************************************************
Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>

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
#include "datacontrolsource.h"
#include "wayland_pointer_p.h"
// Qt
#include <QMimeType>
// Wayland
#include <wayland-data-control-v1-client-protocol.h>

namespace KWayland
{
namespace Client
{

class Q_DECL_HIDDEN DataControlSource::Private
{
public:
    explicit Private(DataControlSource *q);
    void setup(zwlr_data_control_source_v1 *s);

    WaylandPointer<zwlr_data_control_source_v1, zwlr_data_control_source_v1_destroy> source;

private:
    static void sendCallback(void *data, zwlr_data_control_source_v1 *dataSource, const char *mimeType, int32_t fd);
    static void cancelledCallback(void *data, zwlr_data_control_source_v1 *dataSource);

    static const struct zwlr_data_control_source_v1_listener s_listener;

    DataControlSource *q;
};

const zwlr_data_control_source_v1_listener DataControlSource::Private::s_listener = {
    sendCallback,
    cancelledCallback
};

DataControlSource::Private::Private(DataControlSource *q)
    : q(q)
{
}

void DataControlSource::Private::sendCallback(void *data, zwlr_data_control_source_v1 *dataSource, const char *mimeType, int32_t fd)
{
    auto d = reinterpret_cast<DataControlSource::Private*>(data);
    Q_ASSERT(d->source == dataSource);
    emit d->q->sendDataRequested(QString::fromUtf8(mimeType), fd);
}

void DataControlSource::Private::cancelledCallback(void *data, zwlr_data_control_source_v1 *dataSource)
{
    auto d = reinterpret_cast<DataControlSource::Private*>(data);
    Q_ASSERT(d->source == dataSource);
    emit d->q->cancelled();
}

void DataControlSource::Private::setup(zwlr_data_control_source_v1 *s)
{
    Q_ASSERT(!source.isValid());
    Q_ASSERT(s);
    source.setup(s);
    zwlr_data_control_source_v1_add_listener(s, &s_listener, this);
}

DataControlSource::DataControlSource(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

DataControlSource::~DataControlSource()
{
    release();
}

void DataControlSource::release()
{
    d->source.release();
}

void DataControlSource::destroy()
{
    d->source.destroy();
}

bool DataControlSource::isValid() const
{
    return d->source.isValid();
}

void DataControlSource::setup(zwlr_data_control_source_v1 *dataSource)
{
    d->setup(dataSource);
}

void DataControlSource::offer(const QString &mimeType)
{
    zwlr_data_control_source_v1_offer(d->source, mimeType.toUtf8().constData());
}

void DataControlSource::offer(const QMimeType &mimeType)
{
    if (!mimeType.isValid()) {
        return;
    }
    offer(mimeType.name());
}

DataControlSource::operator zwlr_data_control_source_v1*() const
{
    return d->source;
}

DataControlSource::operator zwlr_data_control_source_v1*()
{
    return d->source;
}

}
}
