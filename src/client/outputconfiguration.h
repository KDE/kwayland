/****************************************************************************
* Copyright 2015  Sebastian Kügler <sebas@kde.org>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) version 3, or any
* later version accepted by the membership of KDE e.V. (or its
* successor approved by the membership of KDE e.V.), which shall
* act as a proxy defined in Section 6 of version 3 of the license.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/
#ifndef KWAYLAND_CLIENT_OUTPUTCONFIGURATION_H
#define KWAYLAND_CLIENT_OUTPUTCONFIGURATION_H

#include <QObject>
#include <QPoint>

#include "outputdevice.h"
#include <KWayland/Client/kwaylandclient_export.h>

struct org_kde_kwin_outputmanagement;
struct org_kde_kwin_outputconfiguration;

namespace KWayland
{
namespace Client
{

class EventQueue;

class KWAYLANDCLIENT_EXPORT OutputConfiguration : public QObject
{
Q_OBJECT
public:
    virtual ~OutputConfiguration();

    /**
    * Setup this OutputConfiguration to manage the @p outputconfiguration.
    * When using OutputManagement::createOutputConfiguration there is no need to call this
    * method.
    **/
    void setup(org_kde_kwin_outputconfiguration *outputconfiguration);
    /**
    * @returns @c true if managing a org_kde_kwin_outputconfiguration.
    **/
    bool isValid() const;
    /**
    * Releases the org_kde_kwin_outputconfiguration interface.
    * After the interface has been released the OutputConfiguration instance is no
    * longer valid and can be setup with another org_kde_kwin_outputconfiguration interface.
    **/
    void release();
    /**
    * Destroys the data held by this OutputConfiguration.
    * This method is supposed to be used when the connection to the Wayland
    * server goes away. If the connection is not valid any more, it's not
    * possible to call release any more as that calls into the Wayland
    * connection and the call would fail. This method cleans up the data, so
    * that the instance can be deleted or setup to a new org_kde_kwin_outputconfiguration interface
    * once there is a new connection available.
    *
    * It is suggested to connect this method to ConnectionThread::connectionDied:
    * @code
    * connect(connection, &ConnectionThread::connectionDied, outputconfiguration, &OutputConfiguration::destroy);
    * @endcode
    *
    * @see release
    **/
    void destroy();
    /**
     * Sets the @p queue to use for creating a OutputConfiguration.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating a OutputConfiguration
     **/
    EventQueue *eventQueue();

    void setEnabled(OutputDevice *outputdevice, OutputDevice::Enablement enable);

    void setMode(OutputDevice *outputdevice, const QSize &size, int refreshRate = 60000);
    void setTransform(OutputDevice *outputdevice, KWayland::Client::OutputDevice::Transform transform);

    /**
     * Position this output in the global space, relative to other outputs.
     */
    void setPosition(OutputDevice *outputdevice, const QPoint &pos);

    /**
     * Scale rendering of this output.
     */
    void setScale(OutputDevice *outputdevice, qint32 scale);

    void apply();

    operator org_kde_kwin_outputconfiguration*();
    operator org_kde_kwin_outputconfiguration*() const;

Q_SIGNALS:
    void applied();
    void failed();

private:
    friend class OutputManagement;
    explicit OutputConfiguration(QObject *parent = nullptr);
    class Private;
    QScopedPointer<Private> d;
};


}
}


#endif
