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
#include "../../src/client/registry.h"
#include "../../src/client/shm_pool.h"
#include "../../src/client/surface.h"
#include "../../src/server/buffer_interface.h"
#include "../../src/server/compositor_interface.h"
#include "../../src/server/display.h"
#include "../../src/server/surface_interface.h"

class TestCompositor : public QObject
{
    Q_OBJECT
public:
    explicit TestCompositor(QObject *parent = nullptr);
private Q_SLOTS:
    void init();
    void cleanup();

    void testDestroy();
    void testCast();

private:
    KWayland::Server::Display *m_display;
    KWayland::Server::CompositorInterface *m_compositorInterface;
    KWayland::Client::ConnectionThread *m_connection;
    KWayland::Client::Compositor *m_compositor;
    QThread *m_thread;
};

static const QString s_socketName = QStringLiteral("kwayland-test-wayland-compositor-0");

TestCompositor::TestCompositor(QObject *parent)
    : QObject(parent)
    , m_display(nullptr)
    , m_compositorInterface(nullptr)
    , m_connection(nullptr)
    , m_compositor(nullptr)
    , m_thread(nullptr)
{
}

void TestCompositor::init()
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

    KWayland::Client::Registry registry;
    QSignalSpy compositorSpy(&registry, &KWayland::Client::Registry::compositorAnnounced);
    registry.create(m_connection->display());
    QVERIFY(registry.isValid());
    registry.setup();

    // here we need a shm pool
    m_compositorInterface = m_display->createCompositor(m_display);
    QVERIFY(m_compositorInterface);
    m_compositorInterface->create();
    QVERIFY(m_compositorInterface->isValid());

    QVERIFY(compositorSpy.wait());
    const auto name = compositorSpy.first().first().value<quint32>();
    const auto version = compositorSpy.first().last().value<quint32>();
    m_compositor = registry.createCompositor(name, version, this);
}

void TestCompositor::cleanup()
{
    if (m_compositor) {
        delete m_compositor;
        m_compositor = nullptr;
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

void TestCompositor::testDestroy()
{
    using namespace KWayland::Client;
    connect(m_connection, &ConnectionThread::connectionDied, m_compositor, &Compositor::destroy);
    QVERIFY(m_compositor->isValid());

    QSignalSpy connectionDiedSpy(m_connection, &KWayland::Client::ConnectionThread::connectionDied);
    QVERIFY(connectionDiedSpy.isValid());
    delete m_display;
    m_display = nullptr;
    QVERIFY(connectionDiedSpy.wait());

    // now the pool should be destroyed;
    QVERIFY(!m_compositor->isValid());

    // calling destroy again should not fail
    m_compositor->destroy();
}

void TestCompositor::testCast()
{
    using namespace KWayland::Client;
    Registry registry;
    QSignalSpy compositorSpy(&registry, &KWayland::Client::Registry::compositorAnnounced);
    registry.create(m_connection->display());
    QVERIFY(registry.isValid());
    registry.setup();

    QVERIFY(compositorSpy.wait());

    Compositor c;
    auto wlComp = registry.bindCompositor(compositorSpy.first().first().value<quint32>(), compositorSpy.first().last().value<quint32>());
    c.setup(wlComp);
    QCOMPARE((wl_compositor *)c, wlComp);

    const Compositor &c2(c);
    QCOMPARE((wl_compositor *)c2, wlComp);
}

QTEST_GUILESS_MAIN(TestCompositor)
#include "test_compositor.moc"
