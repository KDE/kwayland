/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
// Qt
#include <QSignalSpy>
#include <QTest>
// KWin
#include "../../src/client/compositor.h"
#include "../../src/client/connection_thread.h"
#include "../../src/client/event_queue.h"
#include "../../src/client/region.h"
#include "../../src/client/registry.h"
#include "../../src/server/compositor_interface.h"
#include "../../src/server/display.h"
#include "../../src/server/region_interface.h"

class TestRegion : public QObject
{
    Q_OBJECT
public:
    explicit TestRegion(QObject *parent = nullptr);
private Q_SLOTS:
    void init();
    void cleanup();

    void testCreate();
    void testCreateWithRegion();
    void testCreateUniquePtr();
    void testAdd();
    void testRemove();
    void testDestroy();
    void testDisconnect();

private:
    KWayland::Server::Display *m_display;
    KWayland::Server::CompositorInterface *m_compositorInterface;
    KWayland::Client::ConnectionThread *m_connection;
    KWayland::Client::Compositor *m_compositor;
    KWayland::Client::EventQueue *m_queue;
    QThread *m_thread;
};

static const QString s_socketName = QStringLiteral("kwayland-test-wayland-region-0");

TestRegion::TestRegion(QObject *parent)
    : QObject(parent)
    , m_display(nullptr)
    , m_compositorInterface(nullptr)
    , m_connection(nullptr)
    , m_compositor(nullptr)
    , m_queue(nullptr)
    , m_thread(nullptr)
{
}

void TestRegion::init()
{
    using namespace KWayland::Server;
    delete m_display;
    m_display = new Display(this);
    m_display->setSocketName(s_socketName);
    m_display->start();
    QVERIFY(m_display->isRunning());

    // setup connection
    m_connection = new KWayland::Client::ConnectionThread;
    QSignalSpy connectedSpy(m_connection, &KWayland::Client::ConnectionThread::connected);
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

    KWayland::Client::Registry registry;
    QSignalSpy compositorSpy(&registry, &KWayland::Client::Registry::compositorAnnounced);
    QVERIFY(compositorSpy.isValid());
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
    const auto name = compositorSpy.first().first().value<quint32>();
    const auto version = compositorSpy.first().last().value<quint32>();
    m_compositor = registry.createCompositor(name, version, this);
}

void TestRegion::cleanup()
{
    if (m_compositor) {
        delete m_compositor;
        m_compositor = nullptr;
    }
    if (m_queue) {
        delete m_queue;
        m_queue = nullptr;
    }
    if (m_thread) {
        m_thread->quit();
        m_thread->wait();
        delete m_thread;
        m_thread = nullptr;
    }
    delete m_connection;
    m_connection = nullptr;

    delete m_display;
    m_display = nullptr;
}

void TestRegion::testCreate()
{
    using namespace KWayland::Client;
    using namespace KWayland::Server;
    QSignalSpy regionCreatedSpy(m_compositorInterface, &KWayland::Server::CompositorInterface::regionCreated);
    QVERIFY(regionCreatedSpy.isValid());

    QScopedPointer<Region> region(m_compositor->createRegion());
    QCOMPARE(region->region(), QRegion());

    QVERIFY(regionCreatedSpy.wait());
    QCOMPARE(regionCreatedSpy.count(), 1);
    auto serverRegion = regionCreatedSpy.first().first().value<KWayland::Server::RegionInterface *>();
    QVERIFY(serverRegion);
    QCOMPARE(serverRegion->region(), QRegion());
    QCOMPARE(serverRegion->global(), m_compositorInterface);
}

void TestRegion::testCreateWithRegion()
{
    using namespace KWayland::Client;
    using namespace KWayland::Server;
    QSignalSpy regionCreatedSpy(m_compositorInterface, &KWayland::Server::CompositorInterface::regionCreated);
    QVERIFY(regionCreatedSpy.isValid());

    QScopedPointer<Region> region(m_compositor->createRegion(QRegion(0, 0, 10, 20), nullptr));
    QCOMPARE(region->region(), QRegion(0, 0, 10, 20));

    QVERIFY(regionCreatedSpy.wait());
    QCOMPARE(regionCreatedSpy.count(), 1);
    auto serverRegion = regionCreatedSpy.first().first().value<KWayland::Server::RegionInterface *>();
    QVERIFY(serverRegion);
    QCOMPARE(serverRegion->region(), QRegion(0, 0, 10, 20));
    QVERIFY(serverRegion->parentResource());
}

void TestRegion::testCreateUniquePtr()
{
    using namespace KWayland::Client;
    using namespace KWayland::Server;
    QSignalSpy regionCreatedSpy(m_compositorInterface, &KWayland::Server::CompositorInterface::regionCreated);
    QVERIFY(regionCreatedSpy.isValid());

    std::unique_ptr<Region> region(m_compositor->createRegion(QRegion(0, 0, 10, 20)));
    QCOMPARE(region->region(), QRegion(0, 0, 10, 20));

    QVERIFY(regionCreatedSpy.wait());
    QCOMPARE(regionCreatedSpy.count(), 1);
    auto serverRegion = regionCreatedSpy.first().first().value<KWayland::Server::RegionInterface *>();
    QVERIFY(serverRegion);
    QCOMPARE(serverRegion->region(), QRegion(0, 0, 10, 20));
}

void TestRegion::testAdd()
{
    using namespace KWayland::Client;
    using namespace KWayland::Server;
    QSignalSpy regionCreatedSpy(m_compositorInterface, &KWayland::Server::CompositorInterface::regionCreated);
    QVERIFY(regionCreatedSpy.isValid());

    QScopedPointer<Region> region(m_compositor->createRegion());
    QVERIFY(regionCreatedSpy.wait());
    auto serverRegion = regionCreatedSpy.first().first().value<KWayland::Server::RegionInterface *>();

    QSignalSpy regionChangedSpy(serverRegion, &KWayland::Server::RegionInterface::regionChanged);
    QVERIFY(regionChangedSpy.isValid());

    // adding a QRect
    region->add(QRect(0, 0, 10, 20));
    QCOMPARE(region->region(), QRegion(0, 0, 10, 20));

    QVERIFY(regionChangedSpy.wait());
    QCOMPARE(regionChangedSpy.count(), 1);
    QCOMPARE(regionChangedSpy.last().first().value<QRegion>(), QRegion(0, 0, 10, 20));
    QCOMPARE(serverRegion->region(), QRegion(0, 0, 10, 20));

    // adding a QRegion
    region->add(QRegion(5, 5, 10, 50));
    QRegion compareRegion(0, 0, 10, 20);
    compareRegion = compareRegion.united(QRect(5, 5, 10, 50));
    QCOMPARE(region->region(), compareRegion);

    QVERIFY(regionChangedSpy.wait());
    QCOMPARE(regionChangedSpy.count(), 2);
    QCOMPARE(regionChangedSpy.last().first().value<QRegion>(), compareRegion);
    QCOMPARE(serverRegion->region(), compareRegion);
}

void TestRegion::testRemove()
{
    using namespace KWayland::Client;
    using namespace KWayland::Server;
    QSignalSpy regionCreatedSpy(m_compositorInterface, &KWayland::Server::CompositorInterface::regionCreated);
    QVERIFY(regionCreatedSpy.isValid());

    std::unique_ptr<Region> region(m_compositor->createRegion(QRegion(0, 0, 100, 200)));
    QVERIFY(regionCreatedSpy.wait());
    auto serverRegion = regionCreatedSpy.first().first().value<KWayland::Server::RegionInterface *>();

    QSignalSpy regionChangedSpy(serverRegion, &KWayland::Server::RegionInterface::regionChanged);
    QVERIFY(regionChangedSpy.isValid());

    // subtract a QRect
    region->subtract(QRect(0, 0, 10, 20));
    QRegion compareRegion(0, 0, 100, 200);
    compareRegion = compareRegion.subtracted(QRect(0, 0, 10, 20));
    QCOMPARE(region->region(), compareRegion);

    QVERIFY(regionChangedSpy.wait());
    QCOMPARE(regionChangedSpy.count(), 1);
    QCOMPARE(regionChangedSpy.last().first().value<QRegion>(), compareRegion);
    QCOMPARE(serverRegion->region(), compareRegion);

    // subtracting a QRegion
    region->subtract(QRegion(5, 5, 10, 50));
    compareRegion = compareRegion.subtracted(QRect(5, 5, 10, 50));
    QCOMPARE(region->region(), compareRegion);

    QVERIFY(regionChangedSpy.wait());
    QCOMPARE(regionChangedSpy.count(), 2);
    QCOMPARE(regionChangedSpy.last().first().value<QRegion>(), compareRegion);
    QCOMPARE(serverRegion->region(), compareRegion);
}

void TestRegion::testDestroy()
{
    using namespace KWayland::Client;
    QScopedPointer<Region> region(m_compositor->createRegion());

    connect(m_connection, &ConnectionThread::connectionDied, region.data(), &Region::destroy);
    connect(m_connection, &ConnectionThread::connectionDied, m_compositor, &Compositor::destroy);
    connect(m_connection, &ConnectionThread::connectionDied, m_queue, &EventQueue::destroy);
    QVERIFY(region->isValid());

    QSignalSpy connectionDiedSpy(m_connection, &KWayland::Client::ConnectionThread::connectionDied);
    QVERIFY(connectionDiedSpy.isValid());
    delete m_display;
    m_display = nullptr;
    QVERIFY(connectionDiedSpy.wait());

    // now the region should be destroyed;
    QVERIFY(!region->isValid());

    // calling destroy again should not fail
    region->destroy();
}

void TestRegion::testDisconnect()
{
    // this test verifies that the server side correctly tears down the resources when the client disconnects
    using namespace KWayland::Client;
    using namespace KWayland::Server;
    QScopedPointer<Region> r(m_compositor->createRegion());
    QVERIFY(!r.isNull());
    QVERIFY(r->isValid());
    QSignalSpy regionCreatedSpy(m_compositorInterface, &CompositorInterface::regionCreated);
    QVERIFY(regionCreatedSpy.isValid());
    QVERIFY(regionCreatedSpy.wait());
    auto serverRegion = regionCreatedSpy.first().first().value<RegionInterface *>();

    // destroy client
    QSignalSpy clientDisconnectedSpy(serverRegion->client(), &ClientConnection::disconnected);
    QVERIFY(clientDisconnectedSpy.isValid());
    QSignalSpy regionDestroyedSpy(serverRegion, &QObject::destroyed);
    QVERIFY(regionDestroyedSpy.isValid());
    if (m_connection) {
        m_connection->deleteLater();
        m_connection = nullptr;
    }
    QVERIFY(clientDisconnectedSpy.wait());
    QCOMPARE(clientDisconnectedSpy.count(), 1);
    QCOMPARE(regionDestroyedSpy.count(), 0);
    QVERIFY(regionDestroyedSpy.wait());
    QCOMPARE(regionDestroyedSpy.count(), 1);

    r->destroy();
    m_compositor->destroy();
    m_queue->destroy();
}

QTEST_GUILESS_MAIN(TestRegion)
#include "test_wayland_region.moc"
