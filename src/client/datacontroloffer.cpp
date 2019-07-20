/********************************************************************
Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
Copyright 2019  David Edmundson <davidedmundson@kde.org>

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
#include "datacontroloffer.h"
#include "datacontroldevice.h"
#include "wayland_pointer_p.h"
// Qt
#include <QMimeType>
#include <QMimeDatabase>
// Wayland
#include <wayland-data-control-v1-client-protocol.h>

namespace KWayland
{

namespace Client
{

class Q_DECL_HIDDEN DataControlOffer::Private
{
public:
    Private(zwlr_data_control_offer_v1 *offer, DataControlOffer *q);
    WaylandPointer<zwlr_data_control_offer_v1, zwlr_data_control_offer_v1_destroy> dataOffer;
    QList<QMimeType> mimeTypes;

private:
    void offer(const QString &mimeType);
    static void offerCallback(void *data, zwlr_data_control_offer_v1 *dataOffer, const char *mimeType);
    DataControlOffer *q;

    static const struct zwlr_data_control_offer_v1_listener s_listener;
};

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct zwlr_data_control_offer_v1_listener DataControlOffer::Private::s_listener = {
    offerCallback,
};
#endif

DataControlOffer::Private::Private(zwlr_data_control_offer_v1 *offer, DataControlOffer *q)
    : q(q)
{
    dataOffer.setup(offer);
    zwlr_data_control_offer_v1_add_listener(offer, &s_listener, this);
}

void DataControlOffer::Private::offerCallback(void *data, zwlr_data_control_offer_v1 *dataOffer, const char *mimeType)
{
    auto d = reinterpret_cast<Private*>(data);
    Q_ASSERT(d->dataOffer == dataOffer);
    d->offer(QString::fromUtf8(mimeType));
}

void DataControlOffer::Private::offer(const QString &mimeType)
{
    QMimeDatabase db;
    const auto &m = db.mimeTypeForName(mimeType);
    if (m.isValid()) {
        mimeTypes << m;
        emit q->mimeTypeOffered(m.name());
    }
}

DataControlOffer::DataControlOffer(DataControlDevice *parent, zwlr_data_control_offer_v1 *dataOffer)
    : QObject(parent)
    , d(new Private(dataOffer, this))
{
}

DataControlOffer::~DataControlOffer()
{
    release();
}

void DataControlOffer::release()
{
    d->dataOffer.release();
}

void DataControlOffer::destroy()
{
    d->dataOffer.destroy();
}

bool DataControlOffer::isValid() const
{
    return d->dataOffer.isValid();
}

QList< QMimeType > DataControlOffer::offeredMimeTypes() const
{
    return d->mimeTypes;
}

void DataControlOffer::receive(const QMimeType &mimeType, qint32 fd)
{
    receive(mimeType.name(), fd);
}

void DataControlOffer::receive(const QString &mimeType, qint32 fd)
{
    Q_ASSERT(isValid());
    zwlr_data_control_offer_v1_receive(d->dataOffer, mimeType.toUtf8().constData(), fd);
}

DataControlOffer::operator zwlr_data_control_offer_v1*()
{
    return d->dataOffer;
}

DataControlOffer::operator zwlr_data_control_offer_v1*() const
{
    return d->dataOffer;
}

}
}
