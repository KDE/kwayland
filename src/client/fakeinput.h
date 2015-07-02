/********************************************************************
Copyright 2015  Martin Gräßlin <mgraesslin@kde.org>

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
#ifndef KWAYLAND_FAKEINPUT_H
#define KWAYLAND_FAKEINPUT_H

#include <QObject>

#include <KWayland/Client/kwaylandclient_export.h>

struct org_kde_kwin_fake_input;

namespace KWayland
{
namespace Client
{

class EventQueue;
class FakeInputTimeout;
class Seat;

/**
 * @short Wrapper for the org_kde_kwin_fake_input interface.
 *
 * This class provides a convenient wrapper for the org_kde_kwin_fake_input interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the FakeInput interface:
 * @code
 * FakeInput *m = registry->createFakeInput(name, version);
 * @endcode
 *
 * This creates the FakeInput and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * FakeInput *m = new FakeInput;
 * m->setup(registry->bindFakeInput(name, version));
 * @endcode
 *
 * The FakeInput can be used as a drop-in replacement for any org_kde_kwin_fake_input
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 **/
class KWAYLANDCLIENT_EXPORT FakeInput : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a new FakeInput.
     * Note: after constructing the FakeInput it is not yet valid and one needs
     * to call setup. In order to get a ready to use FakeInput prefer using
     * Registry::createFakeInput.
     **/
    explicit FakeInput(QObject *parent = nullptr);
    virtual ~FakeInput();

    /**
     * @returns @c true if managing a org_kde_kwin_fake_input.
     **/
    bool isValid() const;
    /**
     * Setup this FakeInput to manage the @p manager.
     * When using Registry::createFakeInput there is no need to call this
     * method.
     **/
    void setup(org_kde_kwin_fake_input *manager);
    /**
     * Releases the org_kde_kwin_fake_input interface.
     * After the interface has been released the FakeInput instance is no
     * longer valid and can be setup with another org_kde_kwin_fake_input interface.
     **/
    void release();
    /**
     * Destroys the data hold by this FakeInput.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid any more, it's not
     * possible to call release any more as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or setup to a new org_kde_kwin_fake_input interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, manager, &FakeInput::destroyed);
     * @endcode
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for bound proxies.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for bound proxies.
     **/
    EventQueue *eventQueue();

    void authenticate(const QString &applicationName, const QString &reason);
    void requestPointerMove(const QSizeF &delta);
    void requestPointerButtonPress(Qt::MouseButton button);
    void requestPointerButtonPress(quint32 linuxButton);
    void requestPointerButtonRelease(Qt::MouseButton button);
    void requestPointerButtonRelease(quint32 linuxButton);
    void requestPointerButtonClick(Qt::MouseButton button);
    void requestPointerButtonClick(quint32 linuxButton);
    void requestPointerAxis(Qt::Orientation axis, qreal delta);

    operator org_kde_kwin_fake_input*();
    operator org_kde_kwin_fake_input*() const;

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
