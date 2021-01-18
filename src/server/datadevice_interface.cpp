/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "datadevice_interface.h"
#include "datadevicemanager_interface.h"
#include "dataoffer_interface_p.h"
#include "datasource_interface.h"
#include "display.h"
#include "resource_p.h"
#include "pointer_interface.h"
#include "seat_interface.h"
#include "surface_interface.h"
// Wayland
#include <wayland-server.h>

namespace KWayland
{
namespace Server
{

class DataDeviceInterface::Private : public Resource::Private
{
public:
    Private(SeatInterface *seat, DataDeviceInterface *q, DataDeviceManagerInterface *manager, wl_resource *parentResource);
    ~Private();

    DataOfferInterface *createDataOffer(DataSourceInterface *source);

    SeatInterface *seat;
    DataSourceInterface *source = nullptr;
    SurfaceInterface *surface = nullptr;
    SurfaceInterface *icon = nullptr;

    DataSourceInterface *selection = nullptr;
    QMetaObject::Connection selectionUnboundConnection;
    QMetaObject::Connection selectionDestroyedConnection;

    struct Drag {
        SurfaceInterface *surface = nullptr;
        QMetaObject::Connection destroyConnection;
        QMetaObject::Connection posConnection;
        QMetaObject::Connection sourceActionConnection;
        QMetaObject::Connection targetActionConnection;
        quint32 serial = 0;
    };
    Drag drag;

    QPointer<SurfaceInterface> proxyRemoteSurface;

private:
    DataDeviceInterface *q_func() {
        return reinterpret_cast<DataDeviceInterface*>(q);
    }
    void startDrag(DataSourceInterface *dataSource, SurfaceInterface *origin, SurfaceInterface *icon, quint32 serial);
    void setSelection(DataSourceInterface *dataSource);
    static void startDragCallback(wl_client *client, wl_resource *resource, wl_resource *source, wl_resource *origin, wl_resource *icon, uint32_t serial);
    static void setSelectionCallback(wl_client *client, wl_resource *resource, wl_resource *source, uint32_t serial);

    static const struct wl_data_device_interface s_interface;
};

#ifndef K_DOXYGEN
const struct wl_data_device_interface DataDeviceInterface::Private::s_interface = {
    startDragCallback,
    setSelectionCallback,
    resourceDestroyedCallback
};
#endif

DataDeviceInterface::Private::Private(SeatInterface *seat, DataDeviceInterface *q, DataDeviceManagerInterface *manager, wl_resource *parentResource)
    : Resource::Private(q, manager, parentResource, &wl_data_device_interface, &s_interface)
    , seat(seat)
{
}

DataDeviceInterface::Private::~Private() = default;

void DataDeviceInterface::Private::startDragCallback(wl_client *client, wl_resource *resource, wl_resource *source, wl_resource *origin, wl_resource *icon, uint32_t serial)
{
    Q_UNUSED(client)
    Q_UNUSED(serial)
    // TODO: verify serial
    cast<Private>(resource)->startDrag(DataSourceInterface::get(source), SurfaceInterface::get(origin), SurfaceInterface::get(icon), serial);
}

void DataDeviceInterface::Private::startDrag(DataSourceInterface *dataSource, SurfaceInterface *origin, SurfaceInterface *i, quint32 serial)
{
    SurfaceInterface *focusSurface = origin;
    if (proxyRemoteSurface) {
        // origin is a proxy surface
        focusSurface = proxyRemoteSurface.data();
    }
    const bool pointerGrab = seat->hasImplicitPointerGrab(serial) && seat->focusedPointerSurface() == focusSurface;
    if (!pointerGrab) {
        // Client doesn't have pointer grab.
        const bool touchGrab = seat->hasImplicitTouchGrab(serial) && seat->focusedTouchSurface() == focusSurface;
        if (!touchGrab) {
            // Client neither has pointer nor touch grab. No drag start allowed.
            return;
        }
    }
    // TODO: source is allowed to be null, handled client internally!
    Q_Q(DataDeviceInterface);
    source = dataSource;
    if (dataSource) {
        QObject::connect(dataSource, &Resource::aboutToBeUnbound, q, [this] { source = nullptr; });
    }
    surface = origin;
    icon = i;
    drag.serial = serial;
    Q_EMIT q->dragStarted();
}

void DataDeviceInterface::Private::setSelectionCallback(wl_client *client, wl_resource *resource, wl_resource *source, uint32_t serial)
{
    Q_UNUSED(client)
    Q_UNUSED(serial)
    // TODO: verify serial
    cast<Private>(resource)->setSelection(DataSourceInterface::get(source));
}

void DataDeviceInterface::Private::setSelection(DataSourceInterface *dataSource)
{
    if (dataSource && dataSource->supportedDragAndDropActions() && wl_resource_get_version(dataSource->resource()) >= WL_DATA_SOURCE_ACTION_SINCE_VERSION) {
        wl_resource_post_error(dataSource->resource(), WL_DATA_SOURCE_ERROR_INVALID_SOURCE, "Data source is for drag and drop");
        return;
    }
    if (selection == dataSource) {
        return;
    }
    Q_Q(DataDeviceInterface);
    QObject::disconnect(selectionUnboundConnection);
    QObject::disconnect(selectionDestroyedConnection);
    if (selection) {
        selection->cancel();
    }
    selection = dataSource;
    if (selection) {
        auto clearSelection = [this] {
            setSelection(nullptr);
        };
        selectionUnboundConnection = QObject::connect(selection, &Resource::unbound, q, clearSelection);
        selectionDestroyedConnection = QObject::connect(selection, &QObject::destroyed, q, clearSelection);
        Q_EMIT q->selectionChanged(selection);
    } else {
        selectionUnboundConnection = QMetaObject::Connection();
        selectionDestroyedConnection = QMetaObject::Connection();
        Q_EMIT q->selectionCleared();
    }
}

DataOfferInterface *DataDeviceInterface::Private::createDataOffer(DataSourceInterface *source)
{
    if (!resource) {
        return nullptr;
    }
    if (!source) {
        // a data offer can only exist together with a source
        return nullptr;
    }
    Q_Q(DataDeviceInterface);
    DataOfferInterface *offer = new DataOfferInterface(source, q, resource);
    auto c = q->global()->display()->getConnection(wl_resource_get_client(resource));
    offer->create(c, wl_resource_get_version(resource), 0);
    if (!offer->resource()) {
        // TODO: send error?
        delete offer;
        return nullptr;
    }
    wl_data_device_send_data_offer(resource, offer->resource());
    offer->sendAllOffers();
    return offer;
}

DataDeviceInterface::DataDeviceInterface(SeatInterface *seat, DataDeviceManagerInterface *parent, wl_resource *parentResource)
    : Resource(new Private(seat, this, parent, parentResource))
{
}

DataDeviceInterface::~DataDeviceInterface() = default;

SeatInterface *DataDeviceInterface::seat() const
{
    Q_D();
    return d->seat;
}

DataSourceInterface *DataDeviceInterface::dragSource() const
{
    Q_D();
    return d->source;
}

SurfaceInterface *DataDeviceInterface::icon() const
{
    Q_D();
    return d->icon;
}

SurfaceInterface *DataDeviceInterface::origin() const
{
    Q_D();
    return d->proxyRemoteSurface ? d->proxyRemoteSurface.data() : d->surface;
}

DataSourceInterface *DataDeviceInterface::selection() const
{
    Q_D();
    return d->selection;
}

void DataDeviceInterface::sendSelection(DataDeviceInterface *other)
{
    Q_D();
    auto otherSelection = other->selection();
    if (!otherSelection) {
        sendClearSelection();
        return;
    }
    auto r = d->createDataOffer(otherSelection);
    if (!r) {
        return;
    }
    if (!d->resource) {
        return;
    }
    wl_data_device_send_selection(d->resource, r->resource());
}

void DataDeviceInterface::sendClearSelection()
{
    Q_D();
    if (!d->resource) {
        return;
    }
    wl_data_device_send_selection(d->resource, nullptr);
}

void DataDeviceInterface::drop()
{
    Q_D();
    if (!d->resource) {
        return;
    }
    wl_data_device_send_drop(d->resource);
    if (d->drag.posConnection) {
        disconnect(d->drag.posConnection);
        d->drag.posConnection = QMetaObject::Connection();
    }
    disconnect(d->drag.destroyConnection);
    d->drag.destroyConnection = QMetaObject::Connection();
    d->drag.surface = nullptr;
    client()->flush();
}

void DataDeviceInterface::updateDragTarget(SurfaceInterface *surface, quint32 serial)
{
    Q_D();
    if (d->drag.surface) {
        if (d->resource && d->drag.surface->resource()) {
            wl_data_device_send_leave(d->resource);
        }
        if (d->drag.posConnection) {
            disconnect(d->drag.posConnection);
            d->drag.posConnection = QMetaObject::Connection();
        }
        disconnect(d->drag.destroyConnection);
        d->drag.destroyConnection = QMetaObject::Connection();
        d->drag.surface = nullptr;
        if (d->drag.sourceActionConnection) {
            disconnect(d->drag.sourceActionConnection);
            d->drag.sourceActionConnection = QMetaObject::Connection();
        }
        if (d->drag.targetActionConnection) {
            disconnect(d->drag.targetActionConnection);
            d->drag.targetActionConnection = QMetaObject::Connection();
        }
        // don't update serial, we need it
    }
    if (!surface) {
        if (auto s = d->seat->dragSource()->dragSource()) {
            s->dndAction(DataDeviceManagerInterface::DnDAction::None);
        }
        return;
    }
    if (d->proxyRemoteSurface && d->proxyRemoteSurface == surface) {
        // A proxy can not have the remote surface as target.
        // TODO: do this for all client's surfaces?
        return;
    }
    auto *source = d->seat->dragSource()->dragSource();
    DataOfferInterface *offer = d->createDataOffer(source);
    d->drag.surface = surface;
    if (d->seat->isDragPointer()) {
        d->drag.posConnection = connect(d->seat, &SeatInterface::pointerPosChanged, this,
            [this] {
                Q_D();
                const QPointF pos = d->seat->dragSurfaceTransformation().map(d->seat->pointerPos());
                wl_data_device_send_motion(d->resource, d->seat->timestamp(),
                                        wl_fixed_from_double(pos.x()), wl_fixed_from_double(pos.y()));
                client()->flush();
            }
        );
    } else if (d->seat->isDragTouch()) {
        d->drag.posConnection = connect(d->seat, &SeatInterface::touchMoved, this,
            [this](qint32 id, quint32 serial, const QPointF &globalPosition) {
                Q_D();
                Q_UNUSED(id);
                if (serial != d->drag.serial) {
                    // different touch down has been moved
                    return;
                }
                const QPointF pos = d->seat->dragSurfaceTransformation().map(globalPosition);
                wl_data_device_send_motion(d->resource, d->seat->timestamp(),
                                        wl_fixed_from_double(pos.x()), wl_fixed_from_double(pos.y()));
                client()->flush();
            }
        );
    }
    d->drag.destroyConnection = connect(d->drag.surface, &QObject::destroyed, this,
        [this] {
            Q_D();
            if (d->resource) {
                wl_data_device_send_leave(d->resource);
            }
            if (d->drag.posConnection) {
                disconnect(d->drag.posConnection);
            }
            d->drag = Private::Drag();
        }
    );

    // TODO: handle touch position
    const QPointF pos = d->seat->dragSurfaceTransformation().map(d->seat->pointerPos());
    wl_data_device_send_enter(d->resource, serial, surface->resource(),
                              wl_fixed_from_double(pos.x()), wl_fixed_from_double(pos.y()), offer ? offer->resource() : nullptr);
    if (offer) {
        offer->d_func()->sendSourceActions();
        auto matchOffers = [source, offer] {
            DataDeviceManagerInterface::DnDAction action{DataDeviceManagerInterface::DnDAction::None};
            if (source->supportedDragAndDropActions().testFlag(offer->preferredDragAndDropAction())) {
                action = offer->preferredDragAndDropAction();
            } else {
                if (source->supportedDragAndDropActions().testFlag(DataDeviceManagerInterface::DnDAction::Copy) &&
                    offer->supportedDragAndDropActions().testFlag(DataDeviceManagerInterface::DnDAction::Copy)) {
                    action = DataDeviceManagerInterface::DnDAction::Copy;
                } else if (source->supportedDragAndDropActions().testFlag(DataDeviceManagerInterface::DnDAction::Move) &&
                    offer->supportedDragAndDropActions().testFlag(DataDeviceManagerInterface::DnDAction::Move)) {
                    action = DataDeviceManagerInterface::DnDAction::Move;
                } else if (source->supportedDragAndDropActions().testFlag(DataDeviceManagerInterface::DnDAction::Ask) &&
                    offer->supportedDragAndDropActions().testFlag(DataDeviceManagerInterface::DnDAction::Ask)) {
                    action = DataDeviceManagerInterface::DnDAction::Ask;
                }
            }
            offer->dndAction(action);
            source->dndAction(action);
        };
        d->drag.targetActionConnection = connect(offer, &DataOfferInterface::dragAndDropActionsChanged, offer, matchOffers);
        d->drag.sourceActionConnection = connect(source, &DataSourceInterface::supportedDragAndDropActionsChanged, source, matchOffers);
    }
    d->client->flush();
}

quint32 DataDeviceInterface::dragImplicitGrabSerial() const
{
    Q_D();
    return d->drag.serial;
}

void DataDeviceInterface::updateProxy(SurfaceInterface *remote)
{
    Q_D();
    // TODO: connect destroy signal?
    d->proxyRemoteSurface = remote;
}

DataDeviceInterface::Private *DataDeviceInterface::d_func() const
{
    return reinterpret_cast<DataDeviceInterface::Private*>(d.data());
}

}
}
