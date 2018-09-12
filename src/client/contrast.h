/********************************************************************
Copyright 2015  Martin Gräßlin <mgraesslin@kde.org>
Copyright 2015  Marco Martin <mart@kde.org>

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
#ifndef KWAYLAND_CONTRAST_H
#define KWAYLAND_CONTRAST_H

#include <QObject>
#include <QPoint>
#include <QSize>

#include <KWayland/Client/kwaylandclient_export.h>

struct org_kde_kwin_contrast;
struct org_kde_kwin_contrast_manager;

namespace KWayland
{
namespace Client
{

class EventQueue;
class Contrast;
class Surface;
class Region;

/**
 * TODO
 */
class KWAYLANDCLIENT_EXPORT ContrastManager : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new ContrastManager.
     * Note: after constructing the ContrastManager it is not yet valid and one needs
     * to call setup. In order to get a ready to use ContrastManager prefer using
     * Registry::createContrastManager.
     **/
    explicit ContrastManager(QObject *parent = nullptr);
    virtual ~ContrastManager();

    /**
     * @returns @c true if managing a org_kde_kwin_contrast_manager.
     **/
    bool isValid() const;
    /**
     * Setup this ContrastManager to manage the @p contrastManager.
     * When using Registry::createContrastManager there is no need to call this
     * method.
     **/
    void setup(org_kde_kwin_contrast_manager *contrastManager);
    /**
     * Releases the org_kde_kwin_contrast_manager interface.
     * After the interface has been released the ContrastManager instance is no
     * longer valid and can be setup with another org_kde_kwin_contrast_manager interface.
     **/
    void release();
    /**
     * Destroys the data held by this ContrastManager.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_kwin_contrast_manager interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, contrastManager, &ContrastManager::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating a Contrast.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating a Contrast.
     **/
    EventQueue *eventQueue();

    /**
     * Creates and setup a new Contrast with @p parent.
     * @param parent The parent to pass to the Contrast.
     * @returns The new created Contrast
     **/
    Contrast *createContrast(Surface *surface, QObject *parent = nullptr);
    void removeContrast(Surface *surface);

    operator org_kde_kwin_contrast_manager*();
    operator org_kde_kwin_contrast_manager*() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the ContrastManager got created by
     * Registry::createContrastManager
     *
     * @since 5.5
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

/**
 * @short Wrapper for the org_kde_kwin_contrast interface.
 *
 * This class is a convenient wrapper for the org_kde_kwin_contrast interface.
 * To create a Contrast call ContrastManager::createContrast.
 *
 * The main purpose of this class is to setup the next frame which
 * should be rendered. Therefore it provides methods to add damage
 * and to attach a new Buffer and to finalize the frame by calling
 * commit.
 *
 * @see ContrastManager
 **/
class KWAYLANDCLIENT_EXPORT Contrast : public QObject
{
    Q_OBJECT
public:
    virtual ~Contrast();

    /**
     * Setup this Contrast to manage the @p contrast.
     * When using ContrastManager::createContrast there is no need to call this
     * method.
     **/
    void setup(org_kde_kwin_contrast *contrast);
    /**
     * Releases the org_kde_kwin_contrast interface.
     * After the interface has been released the Contrast instance is no
     * longer valid and can be setup with another org_kde_kwin_contrast interface.
     **/
    void release();
    /**
     * Destroys the data held by this Contrast.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_kwin_contrast interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * Contrast gets destroyed.
     *
     * @see release
     **/
    void destroy();
    /**
     * @returns @c true if managing a org_kde_kwin_contrast.
     **/
    bool isValid() const;

    void commit();

    /**
     * Sets the area of the window that will have a contrasted
     * background.
     * The region will have to be created with
     * Compositor::createRegion(QRegion)
     */
    void setRegion(Region *region);
    void setContrast(qreal contrast);
    void setIntensity(qreal intensity);
    void setSaturation(qreal saturation);

    operator org_kde_kwin_contrast*();
    operator org_kde_kwin_contrast*() const;

private:
    friend class ContrastManager;
    explicit Contrast(QObject *parent = nullptr);
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif

