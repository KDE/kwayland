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
#include "dataoffer.h"
#include "datadevice.h"
#include "wayland_pointer_p.h"
// Qt
#include <QMimeType>
#include <QMimeDatabase>
// Wayland
#include <wayland-client-protocol.h>

namespace KWayland
{

namespace Client
{

class DataOffer::Private
{
public:
    Private(wl_data_offer *offer, DataOffer *q);
    WaylandPointer<wl_data_offer, wl_data_offer_destroy> dataOffer;
    QList<QMimeType> mimeTypes;

private:
    void offer(const QString &mimeType);
    static void offerCallback(void *data, wl_data_offer *dataOffer, const char *mimeType);
    DataOffer *q;

    static const struct wl_data_offer_listener s_listener;
};

const struct wl_data_offer_listener DataOffer::Private::s_listener = {
    offerCallback
};

DataOffer::Private::Private(wl_data_offer *offer, DataOffer *q)
    : q(q)
{
    dataOffer.setup(offer);
    wl_data_offer_add_listener(offer, &s_listener, this);
}

void DataOffer::Private::offerCallback(void *data, wl_data_offer *dataOffer, const char *mimeType)
{
    auto d = reinterpret_cast<Private*>(data);
    Q_ASSERT(d->dataOffer == dataOffer);
    d->offer(QString::fromUtf8(mimeType));
}

void DataOffer::Private::offer(const QString &mimeType)
{
    QMimeDatabase db;
    const auto &m = db.mimeTypeForName(mimeType);
    if (m.isValid()) {
        mimeTypes << m;
        emit q->mimeTypeOffered(m.name());
    }
}

DataOffer::DataOffer(DataDevice *parent, wl_data_offer *dataOffer)
    : QObject(parent)
    , d(new Private(dataOffer, this))
{
}

DataOffer::~DataOffer()
{
    release();
}

void DataOffer::release()
{
    d->dataOffer.release();
}

void DataOffer::destroy()
{
    d->dataOffer.destroy();
}

bool DataOffer::isValid() const
{
    return d->dataOffer.isValid();
}

QList< QMimeType > DataOffer::offeredMimeTypes() const
{
    return d->mimeTypes;
}

void DataOffer::receive(const QMimeType &mimeType, qint32 fd)
{
    receive(mimeType.name(), fd);
}

void DataOffer::receive(const QString &mimeType, qint32 fd)
{
    Q_ASSERT(isValid());
    wl_data_offer_receive(d->dataOffer, mimeType.toUtf8().constData(), fd);
}

DataOffer::operator wl_data_offer*()
{
    return d->dataOffer;
}

DataOffer::operator wl_data_offer*() const
{
    return d->dataOffer;
}

}
}
