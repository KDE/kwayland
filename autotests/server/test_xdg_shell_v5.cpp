/********************************************************************
Copyright 2016  Martin Gräßlin <mgraesslin@kde.org>

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
// client
#include "xdg_shell_v5.h"
#include "../../src/client/connection_thread.h"
#include "../../src/client/compositor.h"
#include "../../src/client/event_queue.h"
#include "../../src/client/registry.h"
#include "../../src/client/output.h"
#include "../../src/client/seat.h"
#include "../../src/client/shm_pool.h"
#include "../../src/client/surface.h"
// server
#include "../../src/server/display.h"
#include "../../src/server/compositor_interface.h"
#include "../../src/server/output_interface.h"
#include "../../src/server/seat_interface.h"
#include "../../src/server/surface_interface.h"
#include "../../src/server/xdg_shell_v5_interface.h"
#include <wayland-client-protocol.h>
#include <wayland-xdg-shell-v5-client-protocol.h>

using namespace KWayland::Client;
using namespace KWayland::Server;

Q_DECLARE_METATYPE(Qt::MouseButton)

class XdgShellV5Test : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init();
    void cleanup();

    void testCreateSurface();
    void testTitle();
    void testWindowClass();
    void testMaximize();
    void testMinimize();
    void testFullscreen();
    void testShowWindowMenu();
    void testMove();
    void testResize_data();
    void testResize();
    void testTransient();

private:
    Display *m_display = nullptr;
    CompositorInterface *m_compositorInterface = nullptr;
    OutputInterface *m_o1Interface = nullptr;
    OutputInterface *m_o2Interface = nullptr;
    SeatInterface *m_seatInterface = nullptr;
    XdgShellV5Interface *m_xdgShellInterface = nullptr;
    ConnectionThread *m_connection = nullptr;
    QThread *m_thread = nullptr;
    EventQueue *m_queue = nullptr;
    Compositor *m_compositor = nullptr;
    ShmPool *m_shmPool = nullptr;
    XdgShellV5 *m_xdgShell = nullptr;
    Output *m_output1 = nullptr;
    Output *m_output2 = nullptr;
    Seat *m_seat = nullptr;
};

static const QString s_socketName = QStringLiteral("kwayland-test-xdg_shell-0");

void XdgShellV5Test::init()
{
    delete m_display;
    m_display = new Display(this);
    m_display->setSocketName(s_socketName);
    m_display->start();
    QVERIFY(m_display->isRunning());
    m_display->createShm();
    m_o1Interface = m_display->createOutput(m_display);
    m_o1Interface->addMode(QSize(1024, 768));
    m_o1Interface->create();
    m_o2Interface = m_display->createOutput(m_display);
    m_o2Interface->addMode(QSize(1024, 768));
    m_o2Interface->create();
    m_seatInterface = m_display->createSeat(m_display);
    m_seatInterface->setHasKeyboard(true);
    m_seatInterface->setHasPointer(true);
    m_seatInterface->setHasTouch(true);
    m_seatInterface->create();
    m_compositorInterface = m_display->createCompositor(m_display);
    m_compositorInterface->create();
    m_xdgShellInterface = m_display->createXdgShellUnstableVersion5(m_display);
    m_xdgShellInterface->create();

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

    m_queue = new EventQueue(this);
    m_queue->setup(m_connection);

    Registry registry;
    QSignalSpy interfacesAnnouncedSpy(&registry, &Registry::interfacesAnnounced);
    QVERIFY(interfacesAnnouncedSpy.isValid());
    QSignalSpy interfaceAnnouncedSpy(&registry, &Registry::interfaceAnnounced);
    QVERIFY(interfaceAnnouncedSpy.isValid());
    QSignalSpy outputAnnouncedSpy(&registry, &Registry::outputAnnounced);
    QVERIFY(outputAnnouncedSpy.isValid());
    registry.setEventQueue(m_queue);
    registry.create(m_connection);
    QVERIFY(registry.isValid());
    registry.setup();
    QVERIFY(interfacesAnnouncedSpy.wait());

    QCOMPARE(outputAnnouncedSpy.count(), 2);
    m_output1 = registry.createOutput(outputAnnouncedSpy.first().at(0).value<quint32>(), outputAnnouncedSpy.first().at(1).value<quint32>(), this);
    m_output2 = registry.createOutput(outputAnnouncedSpy.last().at(0).value<quint32>(), outputAnnouncedSpy.last().at(1).value<quint32>(), this);

    m_shmPool = registry.createShmPool(registry.interface(Registry::Interface::Shm).name, registry.interface(Registry::Interface::Shm).version, this);
    QVERIFY(m_shmPool);
    QVERIFY(m_shmPool->isValid());

    m_compositor = registry.createCompositor(registry.interface(Registry::Interface::Compositor).name, registry.interface(Registry::Interface::Compositor).version, this);
    QVERIFY(m_compositor);
    QVERIFY(m_compositor->isValid());

    m_seat = registry.createSeat(registry.interface(Registry::Interface::Seat).name, registry.interface(Registry::Interface::Seat).version, this);
    QVERIFY(m_seat);
    QVERIFY(m_seat->isValid());

    for (auto it = interfaceAnnouncedSpy.constBegin(); it != interfaceAnnouncedSpy.constEnd(); ++it) {
        if ((*it).at(0).toByteArray() == QByteArrayLiteral("xdg_shell")) {
            m_xdgShell = new XdgShellV5(this);
            m_xdgShell->setEventQueue(m_queue);
            auto s = reinterpret_cast<xdg_shell*>(wl_registry_bind(registry, (*it).at(1).value<quint32>(), &xdg_shell_interface, (*it).at(2).value<quint32>()));
            m_xdgShell->setup(s);
            m_queue->addProxy(s);
            break;
        }
    }
    QVERIFY(m_xdgShell);
}

void XdgShellV5Test::cleanup()
{
#define CLEANUP(variable) \
    if (variable) { \
        delete variable; \
        variable = nullptr; \
    }
    CLEANUP(m_xdgShell)
    CLEANUP(m_compositor)
    CLEANUP(m_shmPool)
    CLEANUP(m_output1)
    CLEANUP(m_output2)
    CLEANUP(m_seat)
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
    CLEANUP(m_xdgShellInterface)
    CLEANUP(m_o1Interface);
    CLEANUP(m_o2Interface);
    CLEANUP(m_seatInterface);
    CLEANUP(m_display)
#undef CLEANUP
}

void XdgShellV5Test::testCreateSurface()
{
    // this test verifies that we can create a surface
    // first created the signal spies for the server
    QSignalSpy surfaceCreatedSpy(m_compositorInterface, &CompositorInterface::surfaceCreated);
    QVERIFY(surfaceCreatedSpy.isValid());
    QSignalSpy xdgSurfaceCreatedSpy(m_xdgShellInterface, &XdgShellV5Interface::surfaceCreated);
    QVERIFY(xdgSurfaceCreatedSpy.isValid());

    // create surface
    QScopedPointer<Surface> surface(m_compositor->createSurface());
    QVERIFY(!surface.isNull());
    QVERIFY(surfaceCreatedSpy.wait());
    auto serverSurface = surfaceCreatedSpy.first().first().value<SurfaceInterface*>();
    QVERIFY(serverSurface);

    // create shell surface
    QScopedPointer<XdgSurfaceV5> xdgSurface(m_xdgShell->getXdgSurface(surface.data()));
    QVERIFY(!xdgSurface.isNull());
    QVERIFY(xdgSurfaceCreatedSpy.wait());
    // verify base things
    auto serverXdgSurface = xdgSurfaceCreatedSpy.first().first().value<XdgSurfaceV5Interface*>();
    QVERIFY(serverXdgSurface);
    QCOMPARE(serverXdgSurface->isConfigurePending(), false);
    QCOMPARE(serverXdgSurface->title(), QString());
    QCOMPARE(serverXdgSurface->windowClass(), QByteArray());
    QCOMPARE(serverXdgSurface->isTransient(), false);
    QCOMPARE(serverXdgSurface->transientFor(), QPointer<XdgSurfaceV5Interface>());
    QCOMPARE(serverXdgSurface->surface(), serverSurface);

    // now let's destroy it
    QSignalSpy destroyedSpy(serverXdgSurface, &QObject::destroyed);
    QVERIFY(destroyedSpy.isValid());
    xdgSurface.reset();
    QVERIFY(destroyedSpy.wait());
}

#define SURFACE \
    QSignalSpy xdgSurfaceCreatedSpy(m_xdgShellInterface, &XdgShellV5Interface::surfaceCreated); \
    QVERIFY(xdgSurfaceCreatedSpy.isValid()); \
    QScopedPointer<Surface> surface(m_compositor->createSurface()); \
    QScopedPointer<XdgSurfaceV5> xdgSurface(m_xdgShell->getXdgSurface(surface.data())); \
    QVERIFY(xdgSurfaceCreatedSpy.wait()); \
    auto serverXdgSurface = xdgSurfaceCreatedSpy.first().first().value<XdgSurfaceV5Interface*>(); \
    QVERIFY(serverXdgSurface);

void XdgShellV5Test::testTitle()
{
    // this test verifies that we can change the title of a shell surface
    // first create surface
    SURFACE

    // should not have a title yet
    QCOMPARE(serverXdgSurface->title(), QString());

    // lets' change the title
    QSignalSpy titleChangedSpy(serverXdgSurface, &XdgSurfaceV5Interface::titleChanged);
    QVERIFY(titleChangedSpy.isValid());
    xdgSurface->setTitle(QStringLiteral("foo"));
    QVERIFY(titleChangedSpy.wait());
    QCOMPARE(titleChangedSpy.count(), 1);
    QCOMPARE(titleChangedSpy.first().first().toString(), QStringLiteral("foo"));
    QCOMPARE(serverXdgSurface->title(), QStringLiteral("foo"));
}

void XdgShellV5Test::testWindowClass()
{
    // this test verifies that we can change the window class/app id of a shell surface
    // first create surface
    SURFACE

    // should not have a window class yet
    QCOMPARE(serverXdgSurface->windowClass(), QByteArray());

    // let's change the window class
    QSignalSpy windowClassChanged(serverXdgSurface, &XdgSurfaceV5Interface::windowClassChanged);
    QVERIFY(windowClassChanged.isValid());
    xdgSurface->setAppId(QByteArrayLiteral("org.kde.xdgsurfacetest"));
    QVERIFY(windowClassChanged.wait());
    QCOMPARE(windowClassChanged.count(), 1);
    QCOMPARE(windowClassChanged.first().first().toByteArray(), QByteArrayLiteral("org.kde.xdgsurfacetest"));
    QCOMPARE(serverXdgSurface->windowClass(), QByteArrayLiteral("org.kde.xdgsurfacetest"));
}

void XdgShellV5Test::testMaximize()
{
    // this test verifies that the maximize/unmaximize calls work
    SURFACE

    QSignalSpy maximizeRequestedSpy(serverXdgSurface, &XdgSurfaceV5Interface::maximizedChanged);
    QVERIFY(maximizeRequestedSpy.isValid());

    xdgSurface->setMaximized();
    QVERIFY(maximizeRequestedSpy.wait());
    QCOMPARE(maximizeRequestedSpy.count(), 1);
    QCOMPARE(maximizeRequestedSpy.last().first().toBool(), true);

    xdgSurface->unsetMaximized();
    QVERIFY(maximizeRequestedSpy.wait());
    QCOMPARE(maximizeRequestedSpy.count(), 2);
    QCOMPARE(maximizeRequestedSpy.last().first().toBool(), false);
}

void XdgShellV5Test::testMinimize()
{
    // this test verifies that the minimize request is delivered
    SURFACE

    QSignalSpy minimizeRequestedSpy(serverXdgSurface, &XdgSurfaceV5Interface::minimizeRequested);
    QVERIFY(minimizeRequestedSpy.isValid());

    xdgSurface->setMinimized();
    QVERIFY(minimizeRequestedSpy.wait());
    QCOMPARE(minimizeRequestedSpy.count(), 1);
}

void XdgShellV5Test::testFullscreen()
{
    qRegisterMetaType<OutputInterface*>();
    // this test verifies going to/from fullscreen
    QSignalSpy xdgSurfaceCreatedSpy(m_xdgShellInterface, &XdgShellV5Interface::surfaceCreated);
    QVERIFY(xdgSurfaceCreatedSpy.isValid());
    QScopedPointer<Surface> surface(m_compositor->createSurface());
    QScopedPointer<XdgSurfaceV5> xdgSurface(m_xdgShell->getXdgSurface(surface.data()));
    QVERIFY(xdgSurfaceCreatedSpy.wait());
    auto serverXdgSurface = xdgSurfaceCreatedSpy.first().first().value<XdgSurfaceV5Interface*>();
    QVERIFY(serverXdgSurface);

    QSignalSpy fullscreenSpy(serverXdgSurface, &XdgSurfaceV5Interface::fullscreenChanged);
    QVERIFY(fullscreenSpy.isValid());

    // without an output
    xdgSurface->setFullscreen(nullptr);
    QVERIFY(fullscreenSpy.wait());
    QCOMPARE(fullscreenSpy.count(), 1);
    QCOMPARE(fullscreenSpy.last().at(0).toBool(), true);
    QVERIFY(!fullscreenSpy.last().at(1).value<OutputInterface*>());

    // unset
    xdgSurface->unsetFullscreen();
    QVERIFY(fullscreenSpy.wait());
    QCOMPARE(fullscreenSpy.count(), 2);
    QCOMPARE(fullscreenSpy.last().at(0).toBool(), false);
    QVERIFY(!fullscreenSpy.last().at(1).value<OutputInterface*>());

    // with outputs
    xdgSurface->setFullscreen(m_output1);
    QVERIFY(fullscreenSpy.wait());
    QCOMPARE(fullscreenSpy.count(), 3);
    QCOMPARE(fullscreenSpy.last().at(0).toBool(), true);
    QCOMPARE(fullscreenSpy.last().at(1).value<OutputInterface*>(), m_o1Interface);

    // now other output
    xdgSurface->setFullscreen(m_output2);
    QVERIFY(fullscreenSpy.wait());
    QCOMPARE(fullscreenSpy.count(), 4);
    QCOMPARE(fullscreenSpy.last().at(0).toBool(), true);
    QCOMPARE(fullscreenSpy.last().at(1).value<OutputInterface*>(), m_o2Interface);
}

void XdgShellV5Test::testShowWindowMenu()
{
    qRegisterMetaType<SeatInterface*>();
    // this test verifies that the show window menu request works
    SURFACE

    QSignalSpy windowMenuSpy(serverXdgSurface, &XdgSurfaceV5Interface::windowMenuRequested);
    QVERIFY(windowMenuSpy.isValid());

    // TODO: the serial needs to be a proper one
    xdgSurface->showWindowMenu(m_seat, 20, 30, 40);
    QVERIFY(windowMenuSpy.wait());
    QCOMPARE(windowMenuSpy.count(), 1);
    QCOMPARE(windowMenuSpy.first().at(0).value<SeatInterface*>(), m_seatInterface);
    QCOMPARE(windowMenuSpy.first().at(1).value<quint32>(), 20u);
    QCOMPARE(windowMenuSpy.first().at(2).toPoint(), QPoint(30, 40));
}

void XdgShellV5Test::testMove()
{
    qRegisterMetaType<SeatInterface*>();
    // this test verifies that the move request works
    SURFACE

    QSignalSpy moveSpy(serverXdgSurface, &XdgSurfaceV5Interface::moveRequested);
    QVERIFY(moveSpy.isValid());

    // TODO: the serial needs to be a proper one
    xdgSurface->move(m_seat, 50);
    QVERIFY(moveSpy.wait());
    QCOMPARE(moveSpy.count(), 1);
    QCOMPARE(moveSpy.first().at(0).value<SeatInterface*>(), m_seatInterface);
    QCOMPARE(moveSpy.first().at(1).value<quint32>(), 50u);
}

void XdgShellV5Test::testResize_data()
{
    QTest::addColumn<quint32>("wlEdge");
    QTest::addColumn<Qt::Edges>("edges");

    QTest::newRow("none")         << quint32(XDG_SURFACE_RESIZE_EDGE_NONE)         << Qt::Edges();
    QTest::newRow("top")          << quint32(XDG_SURFACE_RESIZE_EDGE_TOP)          << Qt::Edges(Qt::TopEdge);
    QTest::newRow("bottom")       << quint32(XDG_SURFACE_RESIZE_EDGE_BOTTOM)       << Qt::Edges(Qt::BottomEdge);
    QTest::newRow("left")         << quint32(XDG_SURFACE_RESIZE_EDGE_LEFT)         << Qt::Edges(Qt::LeftEdge);
    QTest::newRow("top left")     << quint32(XDG_SURFACE_RESIZE_EDGE_TOP_LEFT)     << Qt::Edges(Qt::TopEdge | Qt::LeftEdge);
    QTest::newRow("bottom left")  << quint32(XDG_SURFACE_RESIZE_EDGE_BOTTOM_LEFT)  << Qt::Edges(Qt::BottomEdge | Qt::LeftEdge);
    QTest::newRow("right")        << quint32(XDG_SURFACE_RESIZE_EDGE_RIGHT)        << Qt::Edges(Qt::RightEdge);
    QTest::newRow("top right")    << quint32(XDG_SURFACE_RESIZE_EDGE_TOP_RIGHT)    << Qt::Edges(Qt::TopEdge | Qt::RightEdge);
    QTest::newRow("bottom right") << quint32(XDG_SURFACE_RESIZE_EDGE_BOTTOM_RIGHT) << Qt::Edges(Qt::BottomEdge | Qt::RightEdge);
}

void XdgShellV5Test::testResize()
{
    qRegisterMetaType<SeatInterface*>();
    // this test verifies that the resize request works
    SURFACE

    QSignalSpy resizeSpy(serverXdgSurface, &XdgSurfaceV5Interface::resizeRequested);
    QVERIFY(resizeSpy.isValid());

    // TODO: the serial needs to be a proper one
    QFETCH(quint32, wlEdge);
    xdgSurface->resize(m_seat, 60, wlEdge);
    QVERIFY(resizeSpy.wait());
    QCOMPARE(resizeSpy.count(), 1);
    QCOMPARE(resizeSpy.first().at(0).value<SeatInterface*>(), m_seatInterface);
    QCOMPARE(resizeSpy.first().at(1).value<quint32>(), 60u);
    QTEST(resizeSpy.first().at(2).value<Qt::Edges>(), "edges");
}

void XdgShellV5Test::testTransient()
{
    // this test verifies that setting the transient for works
    SURFACE
    QScopedPointer<Surface> surface2(m_compositor->createSurface());
    QScopedPointer<XdgSurfaceV5> xdgSurface2(m_xdgShell->getXdgSurface(surface2.data()));
    QVERIFY(xdgSurfaceCreatedSpy.wait());
    auto serverXdgSurface2 = xdgSurfaceCreatedSpy.last().first().value<XdgSurfaceV5Interface*>();
    QVERIFY(serverXdgSurface2);

    QVERIFY(!serverXdgSurface->isTransient());
    QVERIFY(!serverXdgSurface2->isTransient());

    // now make xdsgSurface2 a transient for xdgSurface
    QSignalSpy transientForSpy(serverXdgSurface2, &XdgSurfaceV5Interface::transientForChanged);
    QVERIFY(transientForSpy.isValid());
    xdgSurface2->setTransientFor(xdgSurface.data());

    QVERIFY(transientForSpy.wait());
    QCOMPARE(transientForSpy.count(), 1);
    QVERIFY(serverXdgSurface2->isTransient());
    QCOMPARE(serverXdgSurface2->transientFor().data(), serverXdgSurface);
    QVERIFY(!serverXdgSurface->isTransient());

    // unset the transient for
    xdgSurface2->setTransientFor(nullptr);
    QVERIFY(transientForSpy.wait());
    QCOMPARE(transientForSpy.count(), 2);
    QVERIFY(!serverXdgSurface2->isTransient());
    QVERIFY(serverXdgSurface2->transientFor().isNull());
    QVERIFY(!serverXdgSurface->isTransient());
}

QTEST_GUILESS_MAIN(XdgShellV5Test)
#include "test_xdg_shell_v5.moc"
