/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "dataoffer_interface_p.h"
#include "datadevice_interface.h"
#include "datasource_interface.h"
// Qt
#include <QStringList>
// Wayland
#include <wayland-server.h>
// system
#include <unistd.h>

namespace KWayland
{
namespace Server
{

#ifndef K_DOXYGEN
const struct wl_data_offer_interface DataOfferInterface::Private::s_interface = {
    acceptCallback,
    receiveCallback,
    resourceDestroyedCallback,
    finishCallback,
    setActionsCallback
};
#endif

DataOfferInterface::Private::Private(DataSourceInterface *source, DataDeviceInterface *parentInterface, DataOfferInterface *q, wl_resource *parentResource)
    : Resource::Private(q, nullptr, parentResource, &wl_data_offer_interface, &s_interface)
    , source(source)
    , dataDevice(parentInterface)
{
    // TODO: connect to new selections
}

DataOfferInterface::Private::~Private() = default;

void DataOfferInterface::Private::acceptCallback(wl_client *client, wl_resource *resource, uint32_t serial, const char *mimeType)
{
    Q_UNUSED(client)
    Q_UNUSED(serial)
    auto p = cast<Private>(resource);
    if (!p->source) {
        return;
    }
    p->source->accept(mimeType ? QString::fromUtf8(mimeType) : QString());
}

void DataOfferInterface::Private::receiveCallback(wl_client *client, wl_resource *resource, const char *mimeType, int32_t fd)
{
    Q_UNUSED(client)
    cast<Private>(resource)->receive(QString::fromUtf8(mimeType), fd);
}

void DataOfferInterface::Private::receive(const QString &mimeType, qint32 fd)
{
    if (!source) {
        close(fd);
        return;
    }
    source->requestData(mimeType, fd);
}

void DataOfferInterface::Private::finishCallback(wl_client *client, wl_resource *resource)
{
    Q_UNUSED(client)
    auto p = cast<Private>(resource);
    if (!p->source) {
        return;
    }
    p->source->dndFinished();
    // TODO: It is a client error to perform other requests than wl_data_offer.destroy after this one
}

void DataOfferInterface::Private::setActionsCallback(wl_client *client, wl_resource *resource, uint32_t dnd_actions, uint32_t preferred_action)
{
    // TODO: check it's drag and drop, otherwise send error
    Q_UNUSED(client)
    DataDeviceManagerInterface::DnDActions supportedActions;
    if (dnd_actions & WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY) {
        supportedActions |= DataDeviceManagerInterface::DnDAction::Copy;
    }
    if (dnd_actions & WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE) {
        supportedActions |= DataDeviceManagerInterface::DnDAction::Move;
    }
    if (dnd_actions & WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK) {
        supportedActions |= DataDeviceManagerInterface::DnDAction::Ask;
    }
    // verify that the no other actions are sent
    if (dnd_actions & ~(WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY | WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE | WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK)) {
        wl_resource_post_error(resource, WL_DATA_OFFER_ERROR_INVALID_ACTION_MASK, "Invalid action mask");
        return;
    }
    if (preferred_action != WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY &&
        preferred_action != WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE &&
        preferred_action != WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK &&
        preferred_action != WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE) {
        wl_resource_post_error(resource, WL_DATA_OFFER_ERROR_INVALID_ACTION, "Invalid preferred action");
        return;
    }

    DataDeviceManagerInterface::DnDAction preferredAction = DataDeviceManagerInterface::DnDAction::None;
    if (preferred_action == WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY) {
        preferredAction = DataDeviceManagerInterface::DnDAction::Copy;
    } else if (preferred_action == WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE) {
        preferredAction = DataDeviceManagerInterface::DnDAction::Move;
    } else if (preferred_action == WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK) {
        preferredAction = DataDeviceManagerInterface::DnDAction::Ask;
    }

    auto p = cast<Private>(resource);
    p->supportedDnDActions = supportedActions;
    p->preferredDnDAction = preferredAction;
    Q_EMIT p->q_func()->dragAndDropActionsChanged();
}

void DataOfferInterface::Private::sendSourceActions()
{
    if (!source) {
        return;
    }
    if (wl_resource_get_version(resource) < WL_DATA_OFFER_SOURCE_ACTIONS_SINCE_VERSION) {
        return;
    }
    uint32_t wlActions = WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
    const auto actions = source->supportedDragAndDropActions();
    if (actions.testFlag(DataDeviceManagerInterface::DnDAction::Copy)) {
        wlActions |= WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY;
    }
    if (actions.testFlag(DataDeviceManagerInterface::DnDAction::Move)) {
        wlActions |= WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE;
    }
    if (actions.testFlag(DataDeviceManagerInterface::DnDAction::Ask)) {
        wlActions |= WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK;
    }
    wl_data_offer_send_source_actions(resource, wlActions);
}

DataOfferInterface::DataOfferInterface(DataSourceInterface *source, DataDeviceInterface *parentInterface, wl_resource *parentResource)
    : Resource(new Private(source, parentInterface, this, parentResource))
{
    Q_ASSERT(source);
    connect(source, &DataSourceInterface::mimeTypeOffered, this,
        [this](const QString &mimeType) {
            Q_D();
            if (!d->resource) {
                return;
            }
            wl_data_offer_send_offer(d->resource, mimeType.toUtf8().constData());
        }
    );
    QObject::connect(source, &QObject::destroyed, this,
        [this] {
            Q_D();
            d->source = nullptr;
        }
    );
}

DataOfferInterface::~DataOfferInterface() = default;

void DataOfferInterface::sendAllOffers()
{
    Q_D();
    for (const QString &mimeType : d->source->mimeTypes()) {
        wl_data_offer_send_offer(d->resource, mimeType.toUtf8().constData());
    }
}

DataOfferInterface::Private *DataOfferInterface::d_func() const
{
    return reinterpret_cast<DataOfferInterface::Private*>(d.data());
}

DataDeviceManagerInterface::DnDActions DataOfferInterface::supportedDragAndDropActions() const
{
    Q_D();
    return d->supportedDnDActions;
}

DataDeviceManagerInterface::DnDAction DataOfferInterface::preferredDragAndDropAction() const
{
    Q_D();
    return d->preferredDnDAction;
}

void DataOfferInterface::dndAction(DataDeviceManagerInterface::DnDAction action)
{
    Q_D();
    if (wl_resource_get_version(d->resource) < WL_DATA_OFFER_ACTION_SINCE_VERSION) {
        return;
    }
    uint32_t wlAction = WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;
    if (action == DataDeviceManagerInterface::DnDAction::Copy) {
        wlAction = WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY;
    } else if (action == DataDeviceManagerInterface::DnDAction::Move ) {
        wlAction = WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE;
    } else if (action == DataDeviceManagerInterface::DnDAction::Ask) {
        wlAction = WL_DATA_DEVICE_MANAGER_DND_ACTION_ASK;
    }
    wl_data_offer_send_action(d->resource, wlAction);
}

}
}
