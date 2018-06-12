/********************************************************************
Copyright 2018  Marco Martin <mart@kde.org>

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
// Qt
#include <QtTest/QtTest>
// KWin
#include "../../src/client/compositor.h"
#include "../../src/client/connection_thread.h"
#include "../../src/client/event_queue.h"
#include "../../src/client/region.h"
#include "../../src/client/registry.h"
#include "../../src/client/surface.h"
#include "../../src/client/plasmavirtualdesktop.h"
#include "../../src/server/display.h"
#include "../../src/server/compositor_interface.h"
#include "../../src/server/region_interface.h"
#include "../../src/server/plasmavirtualdesktop_interface.h"
#include "../../src/server/plasmawindowmanagement_interface.h"
#include "../../src/client/plasmawindowmanagement.h"

using namespace KWayland::Client;

class TestVirtualDesktop : public QObject
{
    Q_OBJECT
public:
    explicit TestVirtualDesktop(QObject *parent = nullptr);
private Q_SLOTS:
    void init();
    void cleanup();

    void testCreate();
    void testDestroy();
    void testActivate();

    void testEnterLeaveDesktop();

private:
    KWayland::Server::Display *m_display;
    KWayland::Server::CompositorInterface *m_compositorInterface;
    KWayland::Server::PlasmaVirtualDesktopManagementInterface *m_plasmaVirtualDesktopManagementInterface;
    KWayland::Server::PlasmaWindowManagementInterface *m_windowManagementInterface;
    KWayland::Server::PlasmaWindowInterface *m_windowInterface;

    KWayland::Client::ConnectionThread *m_connection;
    KWayland::Client::Compositor *m_compositor;
    KWayland::Client::PlasmaVirtualDesktopManagement *m_plasmaVirtualDesktopManagement;
    KWayland::Client::EventQueue *m_queue;
    KWayland::Client::PlasmaWindowManagement *m_windowManagement;
    KWayland::Client::PlasmaWindow *m_window;

    QThread *m_thread;
};

static const QString s_socketName = QStringLiteral("kwayland-test-wayland-virtual-desktop-0");

TestVirtualDesktop::TestVirtualDesktop(QObject *parent)
    : QObject(parent)
    , m_display(nullptr)
    , m_compositorInterface(nullptr)
    , m_connection(nullptr)
    , m_compositor(nullptr)
    , m_queue(nullptr)
    , m_thread(nullptr)
{
}

void TestVirtualDesktop::init()
{
    using namespace KWayland::Server;
    delete m_display;
    m_display = new Display(this);
    m_display->setSocketName(s_socketName);
    m_display->start();
    QVERIFY(m_display->isRunning());

    // setup connection
    m_connection = new KWayland::Client::ConnectionThread;
    QSignalSpy connectedSpy(m_connection, &ConnectionThread::connected);
    QVERIFY(connectedSpy.isValid());
    m_connection->setSocketName(s_socketName);

    m_thread = new QThread(this);
    m_connection->moveToThread(m_thread);
    m_thread->start();

    m_connection->initConnection();
    QVERIFY(connectedSpy.wait());

    m_queue = new KWayland::Client::EventQueue(this);
    QVERIFY(!m_queue->isValid());
    m_queue->setup(m_connection);
    QVERIFY(m_queue->isValid());

    Registry registry;
    QSignalSpy compositorSpy(&registry, &Registry::compositorAnnounced);
    QVERIFY(compositorSpy.isValid());

    QSignalSpy plasmaVirtualDesktopManagementSpy(&registry, &Registry::plasmaVirtualDesktopManagementAnnounced);
    QVERIFY(plasmaVirtualDesktopManagementSpy.isValid());

    QSignalSpy windowManagementSpy(&registry, SIGNAL(plasmaWindowManagementAnnounced(quint32,quint32)));
    QVERIFY(windowManagementSpy.isValid());

    QVERIFY(!registry.eventQueue());
    registry.setEventQueue(m_queue);
    QCOMPARE(registry.eventQueue(), m_queue);
    registry.create(m_connection->display());
    QVERIFY(registry.isValid());
    registry.setup();

    m_compositorInterface = m_display->createCompositor(m_display);
    m_compositorInterface->create();
    QVERIFY(m_compositorInterface->isValid());

    QVERIFY(compositorSpy.wait());
    m_compositor = registry.createCompositor(compositorSpy.first().first().value<quint32>(), compositorSpy.first().last().value<quint32>(), this);

    m_plasmaVirtualDesktopManagementInterface = m_display->createPlasmaVirtualDesktopManagement(m_display);
    m_plasmaVirtualDesktopManagementInterface->create();
    QVERIFY(m_plasmaVirtualDesktopManagementInterface->isValid());

    QVERIFY(plasmaVirtualDesktopManagementSpy.wait());
    m_plasmaVirtualDesktopManagement = registry.createPlasmaVirtualDesktopManagement(plasmaVirtualDesktopManagementSpy.first().first().value<quint32>(), plasmaVirtualDesktopManagementSpy.first().last().value<quint32>(), this);

    m_windowManagementInterface = m_display->createPlasmaWindowManagement(m_display);
    m_windowManagementInterface->create();
    QVERIFY(m_windowManagementInterface->isValid());
    m_windowManagementInterface->setPlasmaVirtualDesktopManagementInterface(m_plasmaVirtualDesktopManagementInterface);

    QVERIFY(windowManagementSpy.wait());
    m_windowManagement = registry.createPlasmaWindowManagement(windowManagementSpy.first().first().value<quint32>(), windowManagementSpy.first().last().value<quint32>(), this);

    QSignalSpy windowSpy(m_windowManagement, SIGNAL(windowCreated(KWayland::Client::PlasmaWindow *)));
    QVERIFY(windowSpy.isValid());
    m_windowInterface = m_windowManagementInterface->createWindow(this);
    m_windowInterface->setPid(1337);

    QVERIFY(windowSpy.wait());
    m_window = windowSpy.first().first().value<KWayland::Client::PlasmaWindow *>();
}

void TestVirtualDesktop::cleanup()
{
#define CLEANUP(variable) \
    if (variable) { \
        delete variable; \
        variable = nullptr; \
    }
    CLEANUP(m_compositor)
    CLEANUP(m_plasmaVirtualDesktopManagement)
    CLEANUP(m_windowManagement)
    CLEANUP(m_queue)
    if (m_connection) {
        m_connection->deleteLater();
        m_connection = nullptr;
    }
    if (m_thread) {
        m_thread->quit();
        m_thread->wait();
        delete m_thread;
        m_thread = nullptr;
    }
    CLEANUP(m_compositorInterface)
    CLEANUP(m_plasmaVirtualDesktopManagementInterface)
    CLEANUP(m_windowManagementInterface)
    CLEANUP(m_display)
#undef CLEANUP
}

void TestVirtualDesktop::testCreate()
{
    QSignalSpy desktopAddedSpy(m_plasmaVirtualDesktopManagement, &PlasmaVirtualDesktopManagement::desktopAdded);
    QSignalSpy managementDoneSpy(m_plasmaVirtualDesktopManagement, &PlasmaVirtualDesktopManagement::done);


    //on this createDesktop bind() isn't called already, the desktopadded signals will be sent after bind happened
    KWayland::Server::PlasmaVirtualDesktopInterface *desktop1Int = m_plasmaVirtualDesktopManagementInterface->createDesktop(QStringLiteral("0-1"));
    desktop1Int->setName("Desktop 1");

    desktopAddedSpy.wait();
    QList<QVariant> arguments = desktopAddedSpy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), QStringLiteral("0-1"));
    QCOMPARE(arguments.at(1).toUInt(), 0);
    m_plasmaVirtualDesktopManagementInterface->sendDone();
    managementDoneSpy.wait();


    QCOMPARE(m_plasmaVirtualDesktopManagement->desktops().length(), 1);

    KWayland::Client::PlasmaVirtualDesktop *desktop1 = m_plasmaVirtualDesktopManagement->desktops().first();
    QSignalSpy desktop1DoneSpy(desktop1, &PlasmaVirtualDesktop::done);
    desktop1Int->sendDone();
    desktop1DoneSpy.wait();

    QCOMPARE(desktop1->id(), QStringLiteral("0-1"));
    QCOMPARE(desktop1->name(), QStringLiteral("Desktop 1"));


    //on those createDesktop the bind will already be done
    KWayland::Server::PlasmaVirtualDesktopInterface *desktop2Int = m_plasmaVirtualDesktopManagementInterface->createDesktop(QStringLiteral("0-2"));
    desktop2Int->setName("Desktop 2");
    desktopAddedSpy.wait();
    arguments = desktopAddedSpy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), QStringLiteral("0-2"));
    QCOMPARE(arguments.at(1).toUInt(), 1);
    QCOMPARE(m_plasmaVirtualDesktopManagement->desktops().length(), 2);

    KWayland::Server::PlasmaVirtualDesktopInterface *desktop3Int = m_plasmaVirtualDesktopManagementInterface->createDesktop(QStringLiteral("0-3"));
    desktop3Int->setName("Desktop 3");
    desktopAddedSpy.wait();
    arguments = desktopAddedSpy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), QStringLiteral("0-3"));
    QCOMPARE(m_plasmaVirtualDesktopManagement->desktops().length(), 3);

    m_plasmaVirtualDesktopManagementInterface->sendDone();
    managementDoneSpy.wait();


    //get the clients
    KWayland::Client::PlasmaVirtualDesktop *desktop2 = m_plasmaVirtualDesktopManagement->desktops()[1];
    QSignalSpy desktop2DoneSpy(desktop2, &PlasmaVirtualDesktop::done);
    desktop2Int->sendDone();
    desktop2DoneSpy.wait();

    KWayland::Client::PlasmaVirtualDesktop *desktop3 = m_plasmaVirtualDesktopManagement->desktops()[2];
    QSignalSpy desktop3DoneSpy(desktop3, &PlasmaVirtualDesktop::done);
    desktop3Int->sendDone();
    desktop3DoneSpy.wait();


    QCOMPARE(desktop1->id(), QStringLiteral("0-1"));
    QCOMPARE(desktop1->name(), QStringLiteral("Desktop 1"));

    QCOMPARE(desktop2->id(), QStringLiteral("0-2"));
    QCOMPARE(desktop2->name(), QStringLiteral("Desktop 2"));

    QCOMPARE(desktop3->id(), QStringLiteral("0-3"));
    QCOMPARE(desktop3->name(), QStringLiteral("Desktop 3"));

    //coherence of order between client and server
    QCOMPARE(m_plasmaVirtualDesktopManagementInterface->desktops().length(), 3);
    QCOMPARE(m_plasmaVirtualDesktopManagement->desktops().length(), 3);

    for (int i = 0; i < m_plasmaVirtualDesktopManagement->desktops().length(); ++i) {
        QCOMPARE(m_plasmaVirtualDesktopManagementInterface->desktops().at(i)->id(), m_plasmaVirtualDesktopManagement->desktops().at(i)->id());
    }
}

void TestVirtualDesktop::testDestroy()
{
    //rebuild some desktops
    testCreate();

    KWayland::Server::PlasmaVirtualDesktopInterface *desktop1Int = m_plasmaVirtualDesktopManagementInterface->desktops().first();
    KWayland::Client::PlasmaVirtualDesktop *desktop1 = m_plasmaVirtualDesktopManagement->desktops().first();

    
    QSignalSpy desktop1IntDestroyedSpy(desktop1Int, &QObject::destroyed);
    QSignalSpy desktop1DestroyedSpy(desktop1, &QObject::destroyed);
    QSignalSpy desktop1RemovedSpy(desktop1, &KWayland::Client::PlasmaVirtualDesktop::removed);
    m_plasmaVirtualDesktopManagementInterface->removeDesktop(QStringLiteral("0-1"));

    //test that both server and client desktoip interfaces go away
    desktop1IntDestroyedSpy.wait();
    desktop1RemovedSpy.wait();
    desktop1DestroyedSpy.wait();

    //coherence of order between client and server
    QCOMPARE(m_plasmaVirtualDesktopManagementInterface->desktops().length(), 2);
    QCOMPARE(m_plasmaVirtualDesktopManagement->desktops().length(), 2);

    for (int i = 0; i < m_plasmaVirtualDesktopManagement->desktops().length(); ++i) {
        QCOMPARE(m_plasmaVirtualDesktopManagementInterface->desktops().at(i)->id(), m_plasmaVirtualDesktopManagement->desktops().at(i)->id());
    }

    //Test 0-2 is active
    QVERIFY(m_plasmaVirtualDesktopManagement->desktops().first()->active());
    QVERIFY(m_plasmaVirtualDesktopManagementInterface->desktops().first()->active());

    //Test the desktopRemoved signal of the manager, remove another desktop as the signals can't be tested at the same time
    QSignalSpy desktopManagerRemovedSpy(m_plasmaVirtualDesktopManagement, &KWayland::Client::PlasmaVirtualDesktopManagement::desktopRemoved);
    m_plasmaVirtualDesktopManagementInterface->removeDesktop(QStringLiteral("0-2"));
    desktopManagerRemovedSpy.wait();
    QCOMPARE(desktopManagerRemovedSpy.takeFirst().at(0).toString(), QStringLiteral("0-2"));

    QCOMPARE(m_plasmaVirtualDesktopManagementInterface->desktops().length(), 1);
    QCOMPARE(m_plasmaVirtualDesktopManagement->desktops().length(), 1);

    //Test 0-3 is active
    QVERIFY(m_plasmaVirtualDesktopManagement->desktops().first()->active());
    QVERIFY(m_plasmaVirtualDesktopManagementInterface->desktops().first()->active());
}

void TestVirtualDesktop::testActivate()
{
    //rebuild some desktops
    testCreate();

    KWayland::Server::PlasmaVirtualDesktopInterface *desktop1Int = m_plasmaVirtualDesktopManagementInterface->desktops().first();
    KWayland::Client::PlasmaVirtualDesktop *desktop1 = m_plasmaVirtualDesktopManagement->desktops().first();
    QVERIFY(desktop1->active());
    QVERIFY(desktop1Int->active());

    KWayland::Server::PlasmaVirtualDesktopInterface *desktop2Int = m_plasmaVirtualDesktopManagementInterface->desktops()[1];
    KWayland::Client::PlasmaVirtualDesktop *desktop2 = m_plasmaVirtualDesktopManagement->desktops()[1];
    QVERIFY(!desktop2Int->active());

    QSignalSpy requestActivateSpy(desktop2Int, &KWayland::Server::PlasmaVirtualDesktopInterface::activateRequested);
    QSignalSpy activatedSpy(desktop2, &KWayland::Client::PlasmaVirtualDesktop::activated);

    desktop2->requestActivate();
    requestActivateSpy.wait();

    //activate the desktop that was requested active
    m_plasmaVirtualDesktopManagementInterface->setActiveDesktop(desktop2->id());
    activatedSpy.wait();

    //correct state in the server
    QVERIFY(desktop2Int->active());
    QVERIFY(!desktop1Int->active());
    //correct state in the client
    QVERIFY(desktop2Int->active());
    QVERIFY(!desktop1Int->active());

    //Test the deactivated signal
    QSignalSpy deactivatedSpy(desktop2, &KWayland::Client::PlasmaVirtualDesktop::deactivated);
    m_plasmaVirtualDesktopManagementInterface->setActiveDesktop(desktop1->id());
    deactivatedSpy.wait();
}

void TestVirtualDesktop::testEnterLeaveDesktop()
{
    testCreate();

    QSignalSpy enterRequestedSpy(m_windowInterface, &KWayland::Server::PlasmaWindowInterface::enterPlasmaVirtualDesktopRequested);
    m_window->requestEnterVirtualDesktop(QStringLiteral("0-1"));
    enterRequestedSpy.wait();

    QCOMPARE(enterRequestedSpy.takeFirst().at(0).toString(), QStringLiteral("0-1"));

    QSignalSpy virtualDesktopEnteredSpy(m_window, &KWayland::Client::PlasmaWindow::plasmaVirtualDesktopEntered);

    //agree to the request
    m_windowInterface->addPlasmaVirtualDesktop(QStringLiteral("0-1"));
    QCOMPARE(m_windowInterface->plasmaVirtualDesktops().length(), 1);
    QCOMPARE(m_windowInterface->plasmaVirtualDesktops().first(), QStringLiteral("0-1"));

    //check if the client received the enter
    virtualDesktopEnteredSpy.wait();
    QCOMPARE(virtualDesktopEnteredSpy.takeFirst().at(0).toString(), QStringLiteral("0-1"));
    QCOMPARE(m_window->plasmaVirtualDesktops().length(), 1);
    QCOMPARE(m_window->plasmaVirtualDesktops().first(), QStringLiteral("0-1"));

    //add another desktop, server side
    m_windowInterface->addPlasmaVirtualDesktop(QStringLiteral("0-3"));
    virtualDesktopEnteredSpy.wait();
    QCOMPARE(virtualDesktopEnteredSpy.takeFirst().at(0).toString(), QStringLiteral("0-3"));
    QCOMPARE(m_windowInterface->plasmaVirtualDesktops().length(), 2);
    QCOMPARE(m_window->plasmaVirtualDesktops().length(), 2);
    QCOMPARE(m_window->plasmaVirtualDesktops()[1], QStringLiteral("0-3"));



    //try to add an invalid desktop
    m_windowInterface->addPlasmaVirtualDesktop(QStringLiteral("invalid"));
    QCOMPARE(m_window->plasmaVirtualDesktops().length(), 2);

    //remove a desktop
    QSignalSpy leaveRequestedSpy(m_windowInterface, &KWayland::Server::PlasmaWindowInterface::leavePlasmaVirtualDesktopRequested);
    m_window->requestLeaveVirtualDesktop(QStringLiteral("0-1"));
    leaveRequestedSpy.wait();

    QCOMPARE(leaveRequestedSpy.takeFirst().at(0).toString(), QStringLiteral("0-1"));

    QSignalSpy virtualDesktopLeftSpy(m_window, &KWayland::Client::PlasmaWindow::plasmaVirtualDesktopLeft);

    //agree to the request
    m_windowInterface->removePlasmaVirtualDesktop(QStringLiteral("0-1"));
    QCOMPARE(m_windowInterface->plasmaVirtualDesktops().length(), 1);
    QCOMPARE(m_windowInterface->plasmaVirtualDesktops().first(), QStringLiteral("0-3"));

    //check if the client received the leave
    virtualDesktopLeftSpy.wait();
    QCOMPARE(virtualDesktopLeftSpy.takeFirst().at(0).toString(), QStringLiteral("0-1"));
    QCOMPARE(m_window->plasmaVirtualDesktops().length(), 1);
    QCOMPARE(m_window->plasmaVirtualDesktops().first(), QStringLiteral("0-3"));


    //Destroy desktop 1
    m_plasmaVirtualDesktopManagementInterface->removeDesktop(QStringLiteral("0-3"));
    //the window should receive a left signal from the destroyed desktop
    virtualDesktopLeftSpy.wait();

    QCOMPARE(m_window->plasmaVirtualDesktops().length(), 0);
}

QTEST_GUILESS_MAIN(TestVirtualDesktop)
#include "test_plasma_virtual_desktop.moc"
