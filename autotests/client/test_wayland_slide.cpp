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
#include "../../src/client/slide.h"
#include "../../src/client/surface.h"
#include "../../src/server/compositor_interface.h"
#include "../../src/server/display.h"
#include "../../src/server/region_interface.h"
#include "../../src/server/slide_interface.h"

using namespace KWayland::Client;

class TestSlide : public QObject
{
    Q_OBJECT
public:
    explicit TestSlide(QObject *parent = nullptr);
private Q_SLOTS:
    void init();
    void cleanup();

    void testCreate();
    void testSurfaceDestroy();

private:
    KWayland::Server::Display *m_display;
    KWayland::Server::CompositorInterface *m_compositorInterface;
    KWayland::Server::SlideManagerInterface *m_slideManagerInterface;
    KWayland::Client::ConnectionThread *m_connection;
    KWayland::Client::Compositor *m_compositor;
    KWayland::Client::SlideManager *m_slideManager;
    KWayland::Client::EventQueue *m_queue;
    QThread *m_thread;
};

static const QString s_socketName = QStringLiteral("kwayland-test-wayland-slide-0");

TestSlide::TestSlide(QObject *parent)
    : QObject(parent)
    , m_display(nullptr)
    , m_compositorInterface(nullptr)
    , m_connection(nullptr)
    , m_compositor(nullptr)
    , m_queue(nullptr)
    , m_thread(nullptr)
{
}

void TestSlide::init()
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

    Registry registry;
    QSignalSpy compositorSpy(&registry, &Registry::compositorAnnounced);
    QVERIFY(compositorSpy.isValid());

    QSignalSpy slideSpy(&registry, &Registry::slideAnnounced);
    QVERIFY(slideSpy.isValid());

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

    m_slideManagerInterface = m_display->createSlideManager(m_display);
    m_slideManagerInterface->create();
    QVERIFY(m_slideManagerInterface->isValid());

    QVERIFY(slideSpy.wait());
    m_slideManager = registry.createSlideManager(slideSpy.first().first().value<quint32>(), slideSpy.first().last().value<quint32>(), this);
}

void TestSlide::cleanup()
{
#define CLEANUP(variable)                                                                                                                                      \
    if (variable) {                                                                                                                                            \
        delete variable;                                                                                                                                       \
        variable = nullptr;                                                                                                                                    \
    }
    CLEANUP(m_compositor)
    CLEANUP(m_slideManager)
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
    CLEANUP(m_slideManagerInterface)
    CLEANUP(m_display)
#undef CLEANUP
}

void TestSlide::testCreate()
{
    QSignalSpy serverSurfaceCreated(m_compositorInterface, &KWayland::Server::CompositorInterface::surfaceCreated);
    QVERIFY(serverSurfaceCreated.isValid());

    QScopedPointer<KWayland::Client::Surface> surface(m_compositor->createSurface());
    QVERIFY(serverSurfaceCreated.wait());

    auto serverSurface = serverSurfaceCreated.first().first().value<KWayland::Server::SurfaceInterface *>();
    QSignalSpy slideChanged(serverSurface, &KWayland::Server::SurfaceInterface::slideOnShowHideChanged);

    auto slide = m_slideManager->createSlide(surface.data(), surface.data());
    slide->setLocation(KWayland::Client::Slide::Location::Top);
    slide->setOffset(15);
    slide->commit();
    surface->commit(KWayland::Client::Surface::CommitFlag::None);

    QVERIFY(slideChanged.wait());
    QCOMPARE(serverSurface->slideOnShowHide()->location(), KWayland::Server::SlideInterface::Location::Top);
    QCOMPARE(serverSurface->slideOnShowHide()->offset(), 15);

    // and destroy
    QSignalSpy destroyedSpy(serverSurface->slideOnShowHide().data(), &QObject::destroyed);
    QVERIFY(destroyedSpy.isValid());
    delete slide;
    QVERIFY(destroyedSpy.wait());
}

void TestSlide::testSurfaceDestroy()
{
    using namespace KWayland::Server;
    QSignalSpy serverSurfaceCreated(m_compositorInterface, &CompositorInterface::surfaceCreated);
    QVERIFY(serverSurfaceCreated.isValid());

    QScopedPointer<KWayland::Client::Surface> surface(m_compositor->createSurface());
    QVERIFY(serverSurfaceCreated.wait());

    auto serverSurface = serverSurfaceCreated.first().first().value<SurfaceInterface *>();
    QSignalSpy slideChanged(serverSurface, &SurfaceInterface::slideOnShowHideChanged);
    QVERIFY(slideChanged.isValid());

    QScopedPointer<Slide> slide(m_slideManager->createSlide(surface.data()));
    slide->commit();
    surface->commit(KWayland::Client::Surface::CommitFlag::None);
    QVERIFY(slideChanged.wait());
    auto serverSlide = serverSurface->slideOnShowHide();
    QVERIFY(!serverSlide.isNull());

    // destroy the parent surface
    QSignalSpy surfaceDestroyedSpy(serverSurface, &QObject::destroyed);
    QVERIFY(surfaceDestroyedSpy.isValid());
    QSignalSpy slideDestroyedSpy(serverSlide.data(), &QObject::destroyed);
    QVERIFY(slideDestroyedSpy.isValid());
    surface.reset();
    QVERIFY(surfaceDestroyedSpy.wait());
    QVERIFY(slideDestroyedSpy.isEmpty());
    // destroy the slide
    slide.reset();
    QVERIFY(slideDestroyedSpy.wait());
}

QTEST_GUILESS_MAIN(TestSlide)
#include "test_wayland_slide.moc"
