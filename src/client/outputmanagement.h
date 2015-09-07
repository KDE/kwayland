/********************************************************************
Copyright 2013  Martin Gräßlin <mgraesslin@kde.org>
Copyright 2015  Sebastian Kügler <sebas@kde.org>

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
#ifndef WAYLAND_KWIN_OUTPUTMANAGEMENT_H
#define WAYLAND_KWIN_OUTPUTMANAGEMENT_H

#include <QObject>
#include <QPointer>
#include <QSize>

#include <KWayland/Client/kwaylandclient_export.h>

struct org_kde_kwin_outputmanagement;
class QPoint;
class QRect;

namespace KWayland
{
namespace Client
{
class OutputConfiguration;
class EventQueue;

/**
 * @short Wrapper for the org_kde_kwin_outputmanagement interface.
 *
 * This class provides a convenient wrapper for the org_kde_kwin_outputmanagement interface.
 * Its main purpose is to provide information about connected, but disabled screens, i.e.
 * outputs that are not visible in the wl_output interface, but could be enabled by the
 * compositor.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create an KWinOutputManagement interface:
 * @code
 * KWayland::Client::OutputManagement *c = registry->createOutputManagement(name, version);
 * @endcode
 *
 * This creates the KWinOutputManagement and sets it up directly. As an alternative this
 *
 * The OutputManagement can be used as a drop-in replacement for any
 * org_kde_kwin_outputmanagement * pointer as it provides matching cast operators.
 *
 * Please note that all properties of OutputManagement are not valid until the
 * done signal has been emitted. The wayland server is pushing the
 * information in an async way to the OutputManagement instance. By emitting done,
 * the OutputManagement indicates that all relevant information is available.
 *
 * @see Registry
 **/
class KWAYLANDCLIENT_EXPORT OutputManagement : public QObject
{
    Q_OBJECT
public:
    explicit OutputManagement(QObject *parent = nullptr);
    virtual ~OutputManagement();

    /**
     * Setup this OutputManagement.
     * When using Registry::createOutputManagement there is no need to call this
     * method.
     **/
    void setup(org_kde_kwin_outputmanagement *outputmanagement);

    void release();
    void destroy();

    /**
     * Sets the @p queue to use for creating a Surface.
     **/
    void setEventQueue(EventQueue *queue);

    /**
     * @returns The event queue to use for creating a Surface.
     **/
    EventQueue *eventQueue();

    void createConfiguration();

    /**
     * @returns @c true if managing a org_kde_kwin_outputmanagement.
     **/
    bool isValid() const;
    operator org_kde_kwin_outputmanagement*();
    operator org_kde_kwin_outputmanagement*() const;

Q_SIGNALS:
    /**
     * This signal is emitted right before the interface is released.
     **/
    void interfaceAboutToBeReleased();
    /**
     * This signal is emitted right before the data is destroyed.
     **/
    void interfaceAboutToBeDestroyed();

    /**
     * An output has been connected, but is not enabled yet.
     * @param output A pointer to the DisabledOutput. This pointers lifetime is
     * managed by the OutputManagement class. Do not delete it yourself.
     */
    void configurationCreated(const KWayland::Client::OutputConfiguration*);

    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the OutputManagement got created by
     * Registry::createOutputManagement
     *
     * @since 5.5
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}
//Q_DECLARE_METATYPE(KWayland::Client::OutputConfiguration)

#endif
