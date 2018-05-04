/********************************************************************
Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
Copyright 2018  Msrco Martin <mart@kde.org>

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

private:
    KWayland::Server::Display *m_display;
    KWayland::Server::CompositorInterface *m_compositorInterface;
    KWayland::Server::PlasmaVirtualDesktopManagementInterface *m_plasmaVirtualDesktopManagementInterface;
    KWayland::Client::ConnectionThread *m_connection;
    KWayland::Client::Compositor *m_compositor;
    KWayland::Client::PlasmaVirtualDesktopManagement *m_plasmaVirtualDesktopManagement;
    KWayland::Client::EventQueue *m_queue;
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
    CLEANUP(m_display)
#undef CLEANUP
}

void TestVirtualDesktop::testCreate()
{
    QSignalSpy desktopAddedSpy(m_plasmaVirtualDesktopManagement, &PlasmaVirtualDesktopManagement::desktopAdded);
    QSignalSpy managementDoneSpy(m_plasmaVirtualDesktopManagement, &PlasmaVirtualDesktopManagement::done);


    QSignalSpy layoutSpy(m_plasmaVirtualDesktopManagement, &PlasmaVirtualDesktopManagement::layout);
    m_plasmaVirtualDesktopManagementInterface->setLayout(2, 2);
    layoutSpy.wait();
    QCOMPARE(m_plasmaVirtualDesktopManagement->rows(), 2);
    QCOMPARE(m_plasmaVirtualDesktopManagement->columns(), 2);

    //on this createDesktop bind() isn't called already, the desktopadded signals will be sent after bind happened
    KWayland::Server::PlasmaVirtualDesktopInterface *desktop1Int = m_plasmaVirtualDesktopManagementInterface->createDesktop(QStringLiteral("0-1"));
    desktop1Int->setName("Desktop 1");
    desktop1Int->setLayoutPosition(0, 0);

    desktopAddedSpy.wait();
    m_plasmaVirtualDesktopManagementInterface->sendDone();
    managementDoneSpy.wait();


    QCOMPARE(m_plasmaVirtualDesktopManagement->desktops().length(), 1);

    KWayland::Client::PlasmaVirtualDesktop *desktop1 = m_plasmaVirtualDesktopManagement->desktops().first();
    QSignalSpy desktop1DoneSpy(desktop1, &PlasmaVirtualDesktop::done);
    desktop1Int->sendDone();
    desktop1DoneSpy.wait();

    QCOMPARE(desktop1->id(), QStringLiteral("0-1"));
    QCOMPARE(desktop1->name(), QStringLiteral("Desktop 1"));
    QCOMPARE(desktop1->row(), 0);
    QCOMPARE(desktop1->column(), 0);


    //on those createDesktop the bind will already be done
    KWayland::Server::PlasmaVirtualDesktopInterface *desktop2Int = m_plasmaVirtualDesktopManagementInterface->createDesktop(QStringLiteral("0-2"));
    desktop2Int->setName("Desktop 2");
    desktop2Int->setLayoutPosition(0, 1);
    desktopAddedSpy.wait();
    QCOMPARE(m_plasmaVirtualDesktopManagement->desktops().length(), 2);

    KWayland::Server::PlasmaVirtualDesktopInterface *desktop3Int = m_plasmaVirtualDesktopManagementInterface->createDesktop(QStringLiteral("0-3"));
    desktop3Int->setName("Desktop 3");
    desktop3Int->setLayoutPosition(1, 0);
    desktopAddedSpy.wait();
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
    QCOMPARE(desktop1->row(), 0);
    QCOMPARE(desktop1->column(), 0);

    QCOMPARE(desktop2->id(), QStringLiteral("0-2"));
    QCOMPARE(desktop2->name(), QStringLiteral("Desktop 2"));
    QCOMPARE(desktop2->row(), 0);
    QCOMPARE(desktop2->column(), 1);

    QCOMPARE(desktop3->id(), QStringLiteral("0-3"));
    QCOMPARE(desktop3->name(), QStringLiteral("Desktop 3"));
    QCOMPARE(desktop3->row(), 1);
    QCOMPARE(desktop3->column(), 0);
}

void TestVirtualDesktop::testDestroy()
{
    
}

void TestVirtualDesktop::testActivate()
{
    //rebuild some desktops
    testCreate();

    KWayland::Server::PlasmaVirtualDesktopInterface *desktop1Int = m_plasmaVirtualDesktopManagementInterface->desktops().first();
    KWayland::Client::PlasmaVirtualDesktop *desktop1 = m_plasmaVirtualDesktopManagement->desktops().first();
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
    //TODO: test the deactivated signal as well?

    //correct state in the server
    QVERIFY(desktop2Int->active());
    QVERIFY(!desktop1Int->active());
    //correct state in the client
    QVERIFY(desktop2Int->active());
    QVERIFY(!desktop1Int->active());
}

QTEST_GUILESS_MAIN(TestVirtualDesktop)
#include "test_plasma_virtual_desktop.moc"
