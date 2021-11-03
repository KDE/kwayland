/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWAYLAND_SERVER_SERVER_DECORATION_INTERFACE_H
#define KWAYLAND_SERVER_SERVER_DECORATION_INTERFACE_H

#include "global.h"
#include "resource.h"

#include <KWayland/Server/kwaylandserver_export.h>

namespace KWayland
{
namespace Server
{
class Display;
class ServerSideDecorationInterface;
class SurfaceInterface;

/**
 * @brief Manager to create ServerSideDecorationInterface.
 *
 * @since 5.6
 **/
class KWAYLANDSERVER_EXPORT ServerSideDecorationManagerInterface : public Global
{
    Q_OBJECT
public:
    ~ServerSideDecorationManagerInterface() override;

    /**
     * Decoration mode used for SurfaceInterfaces.
     **/
    enum class Mode {
        /**
         * Undecorated: neither client, nor server provide decoration. Example: popups.
         **/
        None,
        /**
         * The decoration is part of the surface.
         **/
        Client,
        /**
         * The surface gets embedded into a decoration frame provided by the Server.
         **/
        Server,
    };

    /**
     * Sets the default @p mode which is pushed to the Clients on creating a ServerSideDecorationInterface.
     * @param mode The new default mode.
     * @see defaultMode
     **/
    void setDefaultMode(Mode mode);
    /**
     * @returns the current default mode.
     * @see setDefaultMode
     **/
    Mode defaultMode() const;

Q_SIGNALS:
    /**
     * Emitted whenever a new ServerSideDecorationInterface is created.
     **/
    void decorationCreated(KWayland::Server::ServerSideDecorationInterface *);

private:
    explicit ServerSideDecorationManagerInterface(Display *display, QObject *parent = nullptr);
    friend class Display;
    class Private;
    Private *d_func() const;
};

/**
 * @brief Representing how a SurfaceInterface should be decorated.
 *
 * Created by ServerSideDecorationManagerInterface and emitted with decorationCreated signal.
 *
 * @since 5.6
 **/
class KWAYLANDSERVER_EXPORT ServerSideDecorationInterface : public Resource
{
    Q_OBJECT
public:
    ~ServerSideDecorationInterface() override;

    /**
     * Sets the @p mode on the SurfaceInterface. A client might refuse the provided @p mode,
     * in that case modeRequested will be emitted.
     * @see mode
     * @see modeRequested
     **/
    void setMode(ServerSideDecorationManagerInterface::Mode mode);
    /**
     * @returns the currently set mode, not the requested mode.
     * @see setMode
     * @see modeRequested
     **/
    ServerSideDecorationManagerInterface::Mode mode() const;

    /**
     * @returns The SurfaceInterface this ServerSideDecorationInterface references.
     **/
    SurfaceInterface *surface() const;

    /**
     * @returns The ServerSideDecorationInterface for the given @p surface, @c nullptr if there is none.
     **/
    static ServerSideDecorationInterface *get(SurfaceInterface *surface);

Q_SIGNALS:
    /**
     * The client requested the provided mode.
     * The server needs to acknowledge the requested mode by setting it through setMode.
     * @see setMode
     * @see mode
     **/
    void modeRequested(KWayland::Server::ServerSideDecorationManagerInterface::Mode);

private:
    explicit ServerSideDecorationInterface(ServerSideDecorationManagerInterface *parent, SurfaceInterface *surface, wl_resource *parentResource);
    friend class ServerSideDecorationManagerInterface;

    class Private;
    Private *d_func() const;
};

}
}

Q_DECLARE_METATYPE(KWayland::Server::ServerSideDecorationInterface *)
Q_DECLARE_METATYPE(KWayland::Server::ServerSideDecorationManagerInterface::Mode)

#endif
