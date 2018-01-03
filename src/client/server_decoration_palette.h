/****************************************************************************
Copyright 2017  David Edmundson <kde@davidedmundson.co.uk>

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
****************************************************************************/
#ifndef KWAYLAND_CLIENT_SERVER_DECORATION_PALETTE_H
#define KWAYLAND_CLIENT_SERVER_DECORATION_PALETTE_H

#include <QObject>

#include <KWayland/Client/kwaylandclient_export.h>

struct org_kde_kwin_server_decoration_palette_manager;
struct org_kde_kwin_server_decoration_palette;

namespace KWayland
{
namespace Client
{

class EventQueue;
class Surface;
class ServerDecorationPalette;

/**
 * @short Wrapper for the org_kde_kwin_server_decoration_palette_manager interface.
 *
 * This class provides a convenient wrapper for the org_kde_kwin_server_decoration_palette_manager interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the ServerDecorationPaletteManager interface:
 * @code
 * ServerDecorationPaletteManager *c = registry->createServerDecorationPaletteManager(name, version);
 * @endcode
 *
 * This creates the ServerDecorationPaletteManager and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * ServerDecorationPaletteManager *c = new ServerDecorationPaletteManager;
 * c->setup(registry->bindServerDecorationPaletteManager(name, version));
 * @endcode
 *
 * The ServerDecorationPaletteManager can be used as a drop-in replacement for any org_kde_kwin_server_decoration_palette_manager
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 **/
class KWAYLANDCLIENT_EXPORT ServerSideDecorationPaletteManager : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new ServerDecorationPaletteManager.
     * Note: after constructing the ServerDecorationPaletteManager it is not yet valid and one needs
     * to call setup. In order to get a ready to use ServerDecorationPaletteManager prefer using
     * Registry::createServerDecorationPaletteManager.
     **/
    explicit ServerSideDecorationPaletteManager(QObject *parent = nullptr);
    virtual ~ServerSideDecorationPaletteManager();

    /**
     * Setup this ServerDecorationPaletteManager to manage the @p serverDecorationPaletteManager.
     * When using Registry::createServerDecorationPaletteManager there is no need to call this
     * method.
     **/
    void setup(org_kde_kwin_server_decoration_palette_manager *serverDecorationPaletteManager);
    /**
     * @returns @c true if managing a org_kde_kwin_server_decoration_palette_manager.
     **/
    bool isValid() const;
    /**
     * Releases the org_kde_kwin_server_decoration_palette_manager interface.
     * After the interface has been released the ServerDecorationPaletteManager instance is no
     * longer valid and can be setup with another org_kde_kwin_server_decoration_palette_manager interface.
     **/
    void release();
    /**
     * Destroys the data held by this ServerDecorationPaletteManager.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_kwin_server_decoration_palette_manager interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, palettemanager, &ServerDecorationPaletteManager::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating objects with this ServerDecorationPaletteManager.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating objects with this ServerDecorationPaletteManager.
     **/
    EventQueue *eventQueue();

    ServerDecorationPalette *create(Surface *surface, QObject *parent = nullptr);

    operator org_kde_kwin_server_decoration_palette_manager*();
    operator org_kde_kwin_server_decoration_palette_manager*() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the ServerDecorationPaletteManager got created by
     * Registry::createServerDecorationPaletteManager
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

class KWAYLANDCLIENT_EXPORT ServerDecorationPalette : public QObject
{
    Q_OBJECT
public:
    virtual ~ServerDecorationPalette();

    /**
     * Setup this ServerDecorationPalette to manage the @p serversidedecorationpalette.
     * When using ServerDecorationPaletteManager::create there is no need to call this
     * method.
     **/
    void setup(org_kde_kwin_server_decoration_palette *serversidedecorationpalette);
    /**
     * @returns @c true if managing a org_kde_kwin_server_decoration_palette.
     **/
    bool isValid() const;
    /**
     * Releases the org_kde_kwin_server_decoration_palette interface.
     * After the interface has been released the ServerDecorationPalette instance is no
     * longer valid and can be setup with another org_kde_kwin_server_decoration_palette interface.
     **/
    void release();
    /**
     * Destroys the data held by this ServerDecorationPalette.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_kwin_server_decoration_palette interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, palette, &ServerDecorationPalette::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the palette to be used by the server side decorations.
     * Absolute file path, or name of palette in the user's config directory.
     * If set to empty the default palette will be used.
     */
    void setPalette(const QString &palette);

    operator org_kde_kwin_server_decoration_palette*();
    operator org_kde_kwin_server_decoration_palette*() const;

private:
    friend class ServerSideDecorationPaletteManager;
    explicit ServerDecorationPalette(QObject *parent = nullptr);
    class Private;
    QScopedPointer<Private> d;
};


}
}

#endif
