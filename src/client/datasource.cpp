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
#include "datasource.h"
#include "wayland_pointer_p.h"
// Qt
#include <QMimeType>
// Wayland
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{

class DataSource::Private
{
public:
    explicit Private(DataSource *q);
    void setup(wl_data_source *s);

    WaylandPointer<wl_data_source, wl_data_source_destroy> source;

private:
    static void targetCallback(void *data, wl_data_source *dataSource, const char *mimeType);
    static void sendCallback(void *data, wl_data_source *dataSource, const char *mimeType, int32_t fd);
    static void cancelledCallback(void *data, wl_data_source *dataSource);

    static const struct wl_data_source_listener s_listener;

    DataSource *q;
};

const wl_data_source_listener DataSource::Private::s_listener = {
    targetCallback,
    sendCallback,
    cancelledCallback
};

DataSource::Private::Private(DataSource *q)
    : q(q)
{
}

void DataSource::Private::targetCallback(void *data, wl_data_source *dataSource, const char *mimeType)
{
    auto d = reinterpret_cast<DataSource::Private*>(data);
    Q_ASSERT(d->source == dataSource);
    emit d->q->targetAccepts(QString::fromUtf8(mimeType));
}

void DataSource::Private::sendCallback(void *data, wl_data_source *dataSource, const char *mimeType, int32_t fd)
{
    auto d = reinterpret_cast<DataSource::Private*>(data);
    Q_ASSERT(d->source == dataSource);
    emit d->q->sendDataRequested(QString::fromUtf8(mimeType), fd);
}

void DataSource::Private::cancelledCallback(void *data, wl_data_source *dataSource)
{
    auto d = reinterpret_cast<DataSource::Private*>(data);
    Q_ASSERT(d->source == dataSource);
    emit d->q->cancelled();
}

void DataSource::Private::setup(wl_data_source *s)
{
    Q_ASSERT(!source.isValid());
    Q_ASSERT(s);
    source.setup(s);
    wl_data_source_add_listener(s, &s_listener, this);
}

DataSource::DataSource(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

DataSource::~DataSource()
{
    release();
}

void DataSource::release()
{
    d->source.release();
}

void DataSource::destroy()
{
    d->source.destroy();
}

bool DataSource::isValid() const
{
    return d->source.isValid();
}

void DataSource::setup(wl_data_source *dataSource)
{
    d->setup(dataSource);
}

void DataSource::offer(const QString &mimeType)
{
    wl_data_source_offer(d->source, mimeType.toUtf8().constData());
}

void DataSource::offer(const QMimeType &mimeType)
{
    if (!mimeType.isValid()) {
        return;
    }
    offer(mimeType.name());
}

DataSource::operator wl_data_source*() const
{
    return d->source;
}

DataSource::operator wl_data_source*()
{
    return d->source;
}

}
}
