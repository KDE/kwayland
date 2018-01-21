/********************************************************************
Copyright 2017  David Edmundson <davidedmundson@kde.org>
Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>

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
#include "../../src/client/server_decoration_palette.h"
#include "../../src/server/display.h"
#include "../../src/server/compositor_interface.h"
#include "../../src/server/region_interface.h"
#include "../../src/server/server_decoration_palette_interface.h"

using namespace KWayland::Client;

class TestServerSideDecorationPalette : public QObject
{
    Q_OBJECT
public:
    explicit TestServerSideDecorationPalette(QObject *parent = nullptr);
private Q_SLOTS:
    void init();
    void cleanup();

    void testCreateAndSet();

private:
    KWayland::Server::Display *m_display;
    KWayland::Server::CompositorInterface *m_compositorInterface;
    KWayland::Server::ServerSideDecorationPaletteManagerInterface *m_paletteManagerInterface;
    KWayland::Client::ConnectionThread *m_connection;
    KWayland::Client::Compositor *m_compositor;
    KWayland::Client::ServerSideDecorationPaletteManager *m_paletteManager;
    KWayland::Client::EventQueue *m_queue;
    QThread *m_thread;
};

static const QString s_socketName = QStringLiteral("kwayland-test-wayland-decopalette-0");

TestServerSideDecorationPalette::TestServerSideDecorationPalette(QObject *parent)
    : QObject(parent)
    , m_display(nullptr)
    , m_compositorInterface(nullptr)
    , m_connection(nullptr)
    , m_compositor(nullptr)
    , m_queue(nullptr)
    , m_thread(nullptr)
{
}

void TestServerSideDecorationPalette::init()
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

    QSignalSpy registrySpy(&registry, &Registry::serverSideDecorationPaletteManagerAnnounced);
    QVERIFY(registrySpy.isValid());

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

    m_paletteManagerInterface = m_display->createServerSideDecorationPaletteManager(m_display);
    m_paletteManagerInterface->create();
    QVERIFY(m_paletteManagerInterface->isValid());

    QVERIFY(registrySpy.wait());
    m_paletteManager = registry.createServerSideDecorationPaletteManager(registrySpy.first().first().value<quint32>(), registrySpy.first().last().value<quint32>(), this);
}

void TestServerSideDecorationPalette::cleanup()
{
#define CLEANUP(variable) \
    if (variable) { \
        delete variable; \
        variable = nullptr; \
    }
    CLEANUP(m_compositor)
    CLEANUP(m_paletteManager)
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
    CLEANUP(m_paletteManagerInterface)
    CLEANUP(m_display)
#undef CLEANUP
}

void TestServerSideDecorationPalette::testCreateAndSet()
{
    QSignalSpy serverSurfaceCreated(m_compositorInterface, SIGNAL(surfaceCreated(KWayland::Server::SurfaceInterface*)));
    QVERIFY(serverSurfaceCreated.isValid());

    QScopedPointer<KWayland::Client::Surface> surface(m_compositor->createSurface());
    QVERIFY(serverSurfaceCreated.wait());

    auto serverSurface = serverSurfaceCreated.first().first().value<KWayland::Server::SurfaceInterface*>();
    QSignalSpy paletteCreatedSpy(m_paletteManagerInterface, &KWayland::Server::ServerSideDecorationPaletteManagerInterface::paletteCreated);

    QVERIFY(!m_paletteManagerInterface->paletteForSurface(serverSurface));

    auto palette = m_paletteManager->create(surface.data(), surface.data());
    QVERIFY(paletteCreatedSpy.wait());
    auto paletteInterface = paletteCreatedSpy.first().first().value<KWayland::Server::ServerSideDecorationPaletteInterface*>();
    QCOMPARE(m_paletteManagerInterface->paletteForSurface(serverSurface), paletteInterface);

    QCOMPARE(paletteInterface->palette(), QString());

    QSignalSpy changedSpy(paletteInterface, &KWayland::Server::ServerSideDecorationPaletteInterface::paletteChanged);

    palette->setPalette("foobar");

    QVERIFY(changedSpy.wait());
    QCOMPARE(paletteInterface->palette(), QString("foobar"));

    // and destroy
    QSignalSpy destroyedSpy(paletteInterface, &QObject::destroyed);
    QVERIFY(destroyedSpy.isValid());
    delete palette;
    QVERIFY(destroyedSpy.wait());
    QVERIFY(!m_paletteManagerInterface->paletteForSurface(serverSurface));
}

QTEST_GUILESS_MAIN(TestServerSideDecorationPalette)
#include "test_server_side_decoration_palette.moc"
