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
    void testClose();
    void testConfigureStates_data();
    void testConfigureStates();
    void testConfigureMultipleAcks();
    void testPopup();

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

void XdgShellV5Test::testClose()
{
    // this test verifies that a close request is sent to the client
    SURFACE

    QSignalSpy closeSpy(xdgSurface.data(), &XdgSurfaceV5::closeRequested);
    QVERIFY(closeSpy.isValid());

    serverXdgSurface->close();
    QVERIFY(closeSpy.wait());
    QCOMPARE(closeSpy.count(), 1);

    QSignalSpy destroyedSpy(serverXdgSurface, &XdgSurfaceV5Interface::destroyed);
    QVERIFY(destroyedSpy.isValid());
    xdgSurface.reset();
    QVERIFY(destroyedSpy.wait());
}

void XdgShellV5Test::testConfigureStates_data()
{
    QTest::addColumn<XdgSurfaceV5Interface::States>("serverStates");
    QTest::addColumn<XdgSurfaceV5::States>("clientStates");

    const auto sa = XdgSurfaceV5Interface::States(XdgSurfaceV5Interface::State::Activated);
    const auto sm = XdgSurfaceV5Interface::States(XdgSurfaceV5Interface::State::Maximized);
    const auto sf = XdgSurfaceV5Interface::States(XdgSurfaceV5Interface::State::Fullscreen);
    const auto sr = XdgSurfaceV5Interface::States(XdgSurfaceV5Interface::State::Resizing);

    const auto ca = XdgSurfaceV5::States(XdgSurfaceV5::State::Activated);
    const auto cm = XdgSurfaceV5::States(XdgSurfaceV5::State::Maximized);
    const auto cf = XdgSurfaceV5::States(XdgSurfaceV5::State::Fullscreen);
    const auto cr = XdgSurfaceV5::States(XdgSurfaceV5::State::Resizing);

    QTest::newRow("none")       << XdgSurfaceV5Interface::States()   << XdgSurfaceV5::States();
    QTest::newRow("Active")     << sa << ca;
    QTest::newRow("Maximize")   << sm << cm;
    QTest::newRow("Fullscreen") << sf << cf;
    QTest::newRow("Resizing")   << sr << cr;

    QTest::newRow("Active/Maximize")       << (sa | sm) << (ca | cm);
    QTest::newRow("Active/Fullscreen")     << (sa | sf) << (ca | cf);
    QTest::newRow("Active/Resizing")       << (sa | sr) << (ca | cr);
    QTest::newRow("Maximize/Fullscreen")   << (sm | sf) << (cm | cf);
    QTest::newRow("Maximize/Resizing")     << (sm | sr) << (cm | cr);
    QTest::newRow("Fullscreen/Resizing")   << (sf | sr) << (cf | cr);

    QTest::newRow("Active/Maximize/Fullscreen")   << (sa | sm | sf) << (ca | cm | cf);
    QTest::newRow("Active/Maximize/Resizing")     << (sa | sm | sr) << (ca | cm | cr);
    QTest::newRow("Maximize/Fullscreen|Resizing") << (sm | sf | sr) << (cm | cf | cr);

    QTest::newRow("Active/Maximize/Fullscreen/Resizing")   << (sa | sm | sf | sr) << (ca | cm | cf | cr);
}

void XdgShellV5Test::testConfigureStates()
{
    qRegisterMetaType<XdgSurfaceV5::States>();
    // this test verifies that configure states works
    SURFACE

    QSignalSpy configureSpy(xdgSurface.data(), &XdgSurfaceV5::configureRequested);
    QVERIFY(configureSpy.isValid());

    QFETCH(XdgSurfaceV5Interface::States, serverStates);
    serverXdgSurface->configure(serverStates);
    QVERIFY(configureSpy.wait());
    QCOMPARE(configureSpy.count(), 1);
    QCOMPARE(configureSpy.first().at(0).toSize(), QSize(0, 0));
    QTEST(configureSpy.first().at(1).value<XdgSurfaceV5::States>(), "clientStates");
    QCOMPARE(configureSpy.first().at(2).value<quint32>(), m_display->serial());

    QSignalSpy ackSpy(serverXdgSurface, &XdgSurfaceV5Interface::configureAcknowledged);
    QVERIFY(ackSpy.isValid());

    xdgSurface->ackConfigure(configureSpy.first().at(2).value<quint32>());
    QVERIFY(ackSpy.wait());
    QCOMPARE(ackSpy.count(), 1);
    QCOMPARE(ackSpy.first().first().value<quint32>(), configureSpy.first().at(2).value<quint32>());
}

void XdgShellV5Test::testConfigureMultipleAcks()
{
    qRegisterMetaType<XdgSurfaceV5::States>();
    // this test verifies that with multiple configure requests the last acknowledged one acknowledges all
    SURFACE

    QSignalSpy configureSpy(xdgSurface.data(), &XdgSurfaceV5::configureRequested);
    QVERIFY(configureSpy.isValid());
    QSignalSpy ackSpy(serverXdgSurface, &XdgSurfaceV5Interface::configureAcknowledged);
    QVERIFY(ackSpy.isValid());

    serverXdgSurface->configure(XdgSurfaceV5Interface::States(), QSize(10, 20));
    const quint32 serial1 = m_display->serial();
    serverXdgSurface->configure(XdgSurfaceV5Interface::States(), QSize(20, 30));
    const quint32 serial2 = m_display->serial();
    QVERIFY(serial1 != serial2);
    serverXdgSurface->configure(XdgSurfaceV5Interface::States(), QSize(30, 40));
    const quint32 serial3 = m_display->serial();
    QVERIFY(serial1 != serial3);
    QVERIFY(serial2 != serial3);

    QVERIFY(configureSpy.wait());
    QCOMPARE(configureSpy.count(), 3);
    QCOMPARE(configureSpy.at(0).at(0).toSize(), QSize(10, 20));
    QCOMPARE(configureSpy.at(0).at(1).value<XdgSurfaceV5::States>(), XdgSurfaceV5::States());
    QCOMPARE(configureSpy.at(0).at(2).value<quint32>(), serial1);
    QCOMPARE(configureSpy.at(1).at(0).toSize(), QSize(20, 30));
    QCOMPARE(configureSpy.at(1).at(1).value<XdgSurfaceV5::States>(), XdgSurfaceV5::States());
    QCOMPARE(configureSpy.at(1).at(2).value<quint32>(), serial2);
    QCOMPARE(configureSpy.at(2).at(0).toSize(), QSize(30, 40));
    QCOMPARE(configureSpy.at(2).at(1).value<XdgSurfaceV5::States>(), XdgSurfaceV5::States());
    QCOMPARE(configureSpy.at(2).at(2).value<quint32>(), serial3);

    xdgSurface->ackConfigure(serial3);
    QVERIFY(ackSpy.wait());
    QCOMPARE(ackSpy.count(), 3);
    QCOMPARE(ackSpy.at(0).first().value<quint32>(), serial1);
    QCOMPARE(ackSpy.at(1).first().value<quint32>(), serial2);
    QCOMPARE(ackSpy.at(2).first().value<quint32>(), serial3);
}

void XdgShellV5Test::testPopup()
{
    // this test verifies that the creation of popups works correctly
    SURFACE
    QSignalSpy surfaceCreatedSpy(m_compositorInterface, &CompositorInterface::surfaceCreated);
    QVERIFY(surfaceCreatedSpy.isValid());
    QSignalSpy xdgPopupSpy(m_xdgShellInterface, &XdgShellV5Interface::popupCreated);
    QVERIFY(xdgPopupSpy.isValid());

    QScopedPointer<Surface> popupSurface(m_compositor->createSurface());
    QVERIFY(surfaceCreatedSpy.wait());

    // TODO: proper serial
    QScopedPointer<XdgPopupV5> xdgPopup(m_xdgShell->getXdgPopup(popupSurface.data(), surface.data(), m_seat, 120, QPoint(10, 20)));
    QVERIFY(xdgPopupSpy.wait());
    QCOMPARE(xdgPopupSpy.count(), 1);
    QCOMPARE(xdgPopupSpy.first().at(1).value<SeatInterface*>(), m_seatInterface);
    QCOMPARE(xdgPopupSpy.first().at(2).value<quint32>(), 120u);
    auto serverXdgPopup = xdgPopupSpy.first().first().value<XdgPopupV5Interface*>();
    QVERIFY(serverXdgPopup);

    QCOMPARE(serverXdgPopup->surface(), surfaceCreatedSpy.first().first().value<SurfaceInterface*>());
    QCOMPARE(serverXdgPopup->transientFor().data(), serverXdgSurface->surface());
    QCOMPARE(serverXdgPopup->transientOffset(), QPoint(10, 20));

    // now also a popup for the popup
    QScopedPointer<Surface> popup2Surface(m_compositor->createSurface());
    QScopedPointer<XdgPopupV5> xdgPopup2(m_xdgShell->getXdgPopup(popup2Surface.data(), popupSurface.data(), m_seat, 121, QPoint(5, 7)));
    QVERIFY(xdgPopupSpy.wait());
    QCOMPARE(xdgPopupSpy.count(), 2);
    QCOMPARE(xdgPopupSpy.last().at(1).value<SeatInterface*>(), m_seatInterface);
    QCOMPARE(xdgPopupSpy.last().at(2).value<quint32>(), 121u);
    auto serverXdgPopup2 = xdgPopupSpy.last().first().value<XdgPopupV5Interface*>();
    QVERIFY(serverXdgPopup2);

    QCOMPARE(serverXdgPopup2->surface(), surfaceCreatedSpy.last().first().value<SurfaceInterface*>());
    QCOMPARE(serverXdgPopup2->transientFor().data(), serverXdgPopup->surface());
    QCOMPARE(serverXdgPopup2->transientOffset(), QPoint(5, 7));

    QSignalSpy popup2DoneSpy(xdgPopup2.data(), &XdgPopupV5::popupDone);
    QVERIFY(popup2DoneSpy.isValid());
    serverXdgPopup2->popupDone();
    QVERIFY(popup2DoneSpy.wait());
    // TODO: test that this sends also the done to all parents
}

QTEST_GUILESS_MAIN(XdgShellV5Test)
#include "test_xdg_shell_v5.moc"
