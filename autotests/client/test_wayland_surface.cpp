/********************************************************************
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
#include <QImage>
// KWin
#include "../../src/client/compositor.h"
#include "../../src/client/connection_thread.h"
#include "../../src/client/event_queue.h"
#include "../../src/client/surface.h"
#include "../../src/client/region.h"
#include "../../src/client/registry.h"
#include "../../src/client/shm_pool.h"
#include "../../src/server/buffer_interface.h"
#include "../../src/server/compositor_interface.h"
#include "../../src/server/display.h"
#include "../../src/server/surface_interface.h"
// Wayland
#include <wayland-client-protocol.h>

class TestWaylandSurface : public QObject
{
    Q_OBJECT
public:
    explicit TestWaylandSurface(QObject *parent = nullptr);
private Q_SLOTS:
    void init();
    void cleanup();

    void testStaticAccessor();
    void testDamage();
    void testFrameCallback();
    void testAttachBuffer();
    void testMultipleSurfaces();
    void testOpaque();
    void testInput();
    void testDestroy();

private:
    KWayland::Server::Display *m_display;
    KWayland::Server::CompositorInterface *m_compositorInterface;
    KWayland::Client::ConnectionThread *m_connection;
    KWayland::Client::Compositor *m_compositor;
    KWayland::Client::ShmPool *m_shm;
    KWayland::Client::EventQueue *m_queue;
    QThread *m_thread;
};

static const QString s_socketName = QStringLiteral("kwin-test-wayland-surface-0");

TestWaylandSurface::TestWaylandSurface(QObject *parent)
    : QObject(parent)
    , m_display(nullptr)
    , m_compositorInterface(nullptr)
    , m_connection(nullptr)
    , m_compositor(nullptr)
    , m_thread(nullptr)
{
}

void TestWaylandSurface::init()
{
    using namespace KWayland::Server;
    delete m_display;
    m_display = new Display(this);
    m_display->setSocketName(s_socketName);
    m_display->start();
    QVERIFY(m_display->isRunning());
    m_display->createShm();

    m_compositorInterface = m_display->createCompositor(m_display);
    QVERIFY(m_compositorInterface);
    m_compositorInterface->create();
    QVERIFY(m_compositorInterface->isValid());

    // setup connection
    m_connection = new KWayland::Client::ConnectionThread;
    QSignalSpy connectedSpy(m_connection, SIGNAL(connected()));
    m_connection->setSocketName(s_socketName);

    m_thread = new QThread(this);
    m_connection->moveToThread(m_thread);
    m_thread->start();

    /*connect(QCoreApplication::eventDispatcher(), &QAbstractEventDispatcher::aboutToBlock, m_connection,
        [this]() {
            if (m_connection->display()) {
                wl_display_flush(m_connection->display());
            }
        }
    );*/

    m_connection->initConnection();
    QVERIFY(connectedSpy.wait());

    m_queue = new KWayland::Client::EventQueue(this);
    QVERIFY(!m_queue->isValid());
    m_queue->setup(m_connection);
    QVERIFY(m_queue->isValid());

    KWayland::Client::Registry registry;
    registry.setEventQueue(m_queue);
    QSignalSpy compositorSpy(&registry, SIGNAL(compositorAnnounced(quint32,quint32)));
    QSignalSpy shmSpy(&registry, SIGNAL(shmAnnounced(quint32,quint32)));
    QSignalSpy allAnnounced(&registry, SIGNAL(interfacesAnnounced()));
    QVERIFY(allAnnounced.isValid());
    QVERIFY(shmSpy.isValid());
    registry.create(m_connection->display());
    QVERIFY(registry.isValid());
    registry.setup();
    QVERIFY(allAnnounced.wait());
    QVERIFY(!compositorSpy.isEmpty());
    QVERIFY(!shmSpy.isEmpty());

    m_compositor = registry.createCompositor(compositorSpy.first().first().value<quint32>(), compositorSpy.first().last().value<quint32>(), this);
    QVERIFY(m_compositor->isValid());
    m_shm = registry.createShmPool(shmSpy.first().first().value<quint32>(), shmSpy.first().last().value<quint32>(), this);
    QVERIFY(m_shm->isValid());
}

void TestWaylandSurface::cleanup()
{
    if (m_compositor) {
        delete m_compositor;
        m_compositor = nullptr;
    }
    if (m_shm) {
        delete m_shm;
        m_shm = nullptr;
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

    delete m_compositorInterface;
    m_compositorInterface = nullptr;

    delete m_display;
    m_display = nullptr;
}

void TestWaylandSurface::testStaticAccessor()
{
    QSignalSpy serverSurfaceCreated(m_compositorInterface, SIGNAL(surfaceCreated(KWayland::Server::SurfaceInterface*)));
    QVERIFY(serverSurfaceCreated.isValid());

    QVERIFY(KWayland::Client::Surface::all().isEmpty());
    KWayland::Client::Surface *s1 = m_compositor->createSurface();
    QVERIFY(s1->isValid());
    QCOMPARE(KWayland::Client::Surface::all().count(), 1);
    QCOMPARE(KWayland::Client::Surface::all().first(), s1);
    QCOMPARE(KWayland::Client::Surface::get(*s1), s1);
    QVERIFY(serverSurfaceCreated.wait());
    auto serverSurface1 = serverSurfaceCreated.first().first().value<KWayland::Server::SurfaceInterface*>();
    QVERIFY(serverSurface1);
    QCOMPARE(KWayland::Server::SurfaceInterface::get(serverSurface1->resource()), serverSurface1);
    QCOMPARE(KWayland::Server::SurfaceInterface::get(serverSurface1->id(), serverSurface1->client()), serverSurface1);

    QVERIFY(!s1->size().isValid());
    QSignalSpy sizeChangedSpy(s1, SIGNAL(sizeChanged(QSize)));
    QVERIFY(sizeChangedSpy.isValid());
    const QSize testSize(200, 300);
    s1->setSize(testSize);
    QCOMPARE(s1->size(), testSize);
    QCOMPARE(sizeChangedSpy.count(), 1);
    QCOMPARE(sizeChangedSpy.first().first().toSize(), testSize);

    // add another surface
    KWayland::Client::Surface *s2 = m_compositor->createSurface();
    QVERIFY(s2->isValid());
    QCOMPARE(KWayland::Client::Surface::all().count(), 2);
    QCOMPARE(KWayland::Client::Surface::all().first(), s1);
    QCOMPARE(KWayland::Client::Surface::all().last(), s2);
    QCOMPARE(KWayland::Client::Surface::get(*s1), s1);
    QCOMPARE(KWayland::Client::Surface::get(*s2), s2);
    serverSurfaceCreated.clear();
    QVERIFY(serverSurfaceCreated.wait());
    auto serverSurface2 = serverSurfaceCreated.first().first().value<KWayland::Server::SurfaceInterface*>();
    QVERIFY(serverSurface2);
    QCOMPARE(KWayland::Server::SurfaceInterface::get(serverSurface1->resource()), serverSurface1);
    QCOMPARE(KWayland::Server::SurfaceInterface::get(serverSurface1->id(), serverSurface1->client()), serverSurface1);
    QCOMPARE(KWayland::Server::SurfaceInterface::get(serverSurface2->resource()), serverSurface2);
    QCOMPARE(KWayland::Server::SurfaceInterface::get(serverSurface2->id(), serverSurface2->client()), serverSurface2);

    // delete s2 again
    delete s2;
    QCOMPARE(KWayland::Client::Surface::all().count(), 1);
    QCOMPARE(KWayland::Client::Surface::all().first(), s1);
    QCOMPARE(KWayland::Client::Surface::get(*s1), s1);

    // and finally delete the last one
    delete s1;
    QVERIFY(KWayland::Client::Surface::all().isEmpty());
    QVERIFY(!KWayland::Client::Surface::get(nullptr));
}

void TestWaylandSurface::testDamage()
{
    QSignalSpy serverSurfaceCreated(m_compositorInterface, SIGNAL(surfaceCreated(KWayland::Server::SurfaceInterface*)));
    QVERIFY(serverSurfaceCreated.isValid());
    KWayland::Client::Surface *s = m_compositor->createSurface();
    QVERIFY(serverSurfaceCreated.wait());
    KWayland::Server::SurfaceInterface *serverSurface = serverSurfaceCreated.first().first().value<KWayland::Server::SurfaceInterface*>();
    QVERIFY(serverSurface);
    QCOMPARE(serverSurface->damage(), QRegion());
    QVERIFY(serverSurface->parentResource());

    QSignalSpy damageSpy(serverSurface, SIGNAL(damaged(QRegion)));
    QVERIFY(damageSpy.isValid());

    // send damage without a buffer
    s->damage(QRect(0, 0, 100, 100));
    s->commit(KWayland::Client::Surface::CommitFlag::None);
    wl_display_flush(m_connection->display());
    QCoreApplication::processEvents();
    QCoreApplication::processEvents();
    QVERIFY(damageSpy.isEmpty());

    QImage img(QSize(10, 10), QImage::Format_ARGB32);
    img.fill(Qt::black);
    auto b = m_shm->createBuffer(img);
    s->attachBuffer(b);
    s->damage(QRect(0, 0, 10, 10));
    s->commit(KWayland::Client::Surface::CommitFlag::None);
    QVERIFY(damageSpy.wait());
    QCOMPARE(serverSurface->damage(), QRegion(0, 0, 10, 10));
    QCOMPARE(damageSpy.first().first().value<QRegion>(), QRegion(0, 0, 10, 10));

    // damage multiple times
    QRegion testRegion(5, 8, 3, 6);
    testRegion = testRegion.united(QRect(10, 20, 30, 15));
    img = QImage(QSize(40, 35), QImage::Format_ARGB32);
    img.fill(Qt::black);
    b = m_shm->createBuffer(img);
    s->attachBuffer(b);
    s->damage(testRegion);
    damageSpy.clear();
    s->commit(KWayland::Client::Surface::CommitFlag::None);
    QVERIFY(damageSpy.wait());
    QCOMPARE(serverSurface->damage(), testRegion);
    QCOMPARE(damageSpy.first().first().value<QRegion>(), testRegion);
}

void TestWaylandSurface::testFrameCallback()
{
    QSignalSpy serverSurfaceCreated(m_compositorInterface, SIGNAL(surfaceCreated(KWayland::Server::SurfaceInterface*)));
    QVERIFY(serverSurfaceCreated.isValid());
    KWayland::Client::Surface *s = m_compositor->createSurface();
    QVERIFY(serverSurfaceCreated.wait());
    KWayland::Server::SurfaceInterface *serverSurface = serverSurfaceCreated.first().first().value<KWayland::Server::SurfaceInterface*>();
    QVERIFY(serverSurface);

    QSignalSpy damageSpy(serverSurface, SIGNAL(damaged(QRegion)));
    QVERIFY(damageSpy.isValid());

    QSignalSpy frameRenderedSpy(s, SIGNAL(frameRendered()));
    QVERIFY(frameRenderedSpy.isValid());
    QImage img(QSize(10, 10), QImage::Format_ARGB32);
    img.fill(Qt::black);
    auto b = m_shm->createBuffer(img);
    s->attachBuffer(b);
    s->damage(QRect(0, 0, 10, 10));
    s->commit();
    QVERIFY(damageSpy.wait());
    serverSurface->frameRendered(10);
    QVERIFY(frameRenderedSpy.isEmpty());
    QVERIFY(frameRenderedSpy.wait());
    QVERIFY(!frameRenderedSpy.isEmpty());
}

void TestWaylandSurface::testAttachBuffer()
{
    // create the surface
    QSignalSpy serverSurfaceCreated(m_compositorInterface, SIGNAL(surfaceCreated(KWayland::Server::SurfaceInterface*)));
    QVERIFY(serverSurfaceCreated.isValid());
    KWayland::Client::Surface *s = m_compositor->createSurface();
    QVERIFY(serverSurfaceCreated.wait());
    KWayland::Server::SurfaceInterface *serverSurface = serverSurfaceCreated.first().first().value<KWayland::Server::SurfaceInterface*>();
    QVERIFY(serverSurface);

    // create two images
    QImage black(24, 24, QImage::Format_RGB32);
    black.fill(Qt::black);
    QImage red(24, 24, QImage::Format_ARGB32);
    red.fill(QColor(255, 0, 0, 128));
    QImage blue(24, 24, QImage::Format_ARGB32_Premultiplied);
    blue.fill(QColor(0, 0, 255, 128));

    wl_buffer *blackBuffer = *(m_shm->createBuffer(black).data());
    auto redBuffer = m_shm->createBuffer(red);
    auto blueBuffer = m_shm->createBuffer(blue).toStrongRef();

    QCOMPARE(blueBuffer->format(), KWayland::Client::Buffer::Format::ARGB32);
    QCOMPARE(blueBuffer->size(), blue.size());
    QVERIFY(!blueBuffer->isReleased());
    QVERIFY(!blueBuffer->isUsed());
    QCOMPARE(blueBuffer->stride(), blue.bytesPerLine());

    s->attachBuffer(redBuffer.data());
    s->attachBuffer(blackBuffer);
    s->damage(QRect(0, 0, 24, 24));
    s->commit(KWayland::Client::Surface::CommitFlag::None);
    QSignalSpy damageSpy(serverSurface, SIGNAL(damaged(QRegion)));
    QVERIFY(damageSpy.isValid());
    QSignalSpy unmappedSpy(serverSurface, SIGNAL(unmapped()));
    QVERIFY(unmappedSpy.isValid());
    QVERIFY(damageSpy.wait());
    QVERIFY(unmappedSpy.isEmpty());

    // now the ServerSurface should have the black image attached as a buffer
    KWayland::Server::BufferInterface *buffer = serverSurface->buffer();
    buffer->ref();
    QVERIFY(buffer->shmBuffer());
    QCOMPARE(buffer->data(), black);
    QCOMPARE(buffer->data().format(), QImage::Format_RGB32);

    // render another frame
    s->attachBuffer(redBuffer);
    s->damage(QRect(0, 0, 24, 24));
    s->commit(KWayland::Client::Surface::CommitFlag::None);
    damageSpy.clear();
    QVERIFY(damageSpy.wait());
    QVERIFY(unmappedSpy.isEmpty());
    KWayland::Server::BufferInterface *buffer2 = serverSurface->buffer();
    buffer2->ref();
    QVERIFY(buffer2->shmBuffer());
    QCOMPARE(buffer2->data(), red);
    QCOMPARE(buffer2->data().format(), QImage::Format_ARGB32);
    buffer2->unref();
    QVERIFY(buffer2->isReferenced());
    QVERIFY(!redBuffer.data()->isReleased());

    // render another frame
    blueBuffer->setUsed(true);
    QVERIFY(blueBuffer->isUsed());
    s->attachBuffer(blueBuffer.data());
    s->damage(QRect(0, 0, 24, 24));
    QSignalSpy frameRenderedSpy(s, SIGNAL(frameRendered()));
    QVERIFY(frameRenderedSpy.isValid());
    s->commit();
    damageSpy.clear();
    QVERIFY(damageSpy.wait());
    QVERIFY(unmappedSpy.isEmpty());
    QVERIFY(!buffer2->isReferenced());
    delete buffer2;
    // TODO: we should have a signal on when the Buffer gets released
    QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);
    if (!redBuffer.data()->isReleased()) {
        QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);
    }
    QVERIFY(redBuffer.data()->isReleased());

    KWayland::Server::BufferInterface *buffer3 = serverSurface->buffer();
    buffer3->ref();
    QVERIFY(buffer3->shmBuffer());
    QCOMPARE(buffer3->data().format(), QImage::Format_ARGB32);
    QCOMPARE(buffer3->data().width(), 24);
    QCOMPARE(buffer3->data().height(), 24);
    for (int i = 0; i < 24; ++i) {
        for (int j = 0; j < 24; ++j) {
            // it's premultiplied in the format
            QCOMPARE(buffer3->data().pixel(i, j), qRgba(0, 0, 128, 128));
        }
    }
    buffer3->unref();
    QVERIFY(buffer3->isReferenced());

    serverSurface->frameRendered(1);
    QVERIFY(frameRenderedSpy.wait());

    // commit a different value shouldn't change our buffer
    QCOMPARE(serverSurface->buffer(), buffer3);
    QVERIFY(serverSurface->input().isNull());
    damageSpy.clear();
    s->setInputRegion(m_compositor->createRegion(QRegion(0, 0, 24, 24)).get());
    s->commit(KWayland::Client::Surface::CommitFlag::None);
    wl_display_flush(m_connection->display());
    QCoreApplication::processEvents();
    QCoreApplication::processEvents();
    QCOMPARE(serverSurface->input(), QRegion(0, 0, 24, 24));
    QCOMPARE(serverSurface->buffer(), buffer3);
    QVERIFY(damageSpy.isEmpty());
    QVERIFY(unmappedSpy.isEmpty());

    // clear the surface
    s->attachBuffer(blackBuffer);
    s->damage(QRect(0, 0, 1, 1));
    // TODO: better method
    s->attachBuffer((wl_buffer*)nullptr);
    s->damage(QRect(0, 0, 10, 10));
    s->commit(KWayland::Client::Surface::CommitFlag::None);
    QVERIFY(unmappedSpy.wait());
    QVERIFY(!unmappedSpy.isEmpty());
    QCOMPARE(unmappedSpy.count(), 1);
    QVERIFY(damageSpy.isEmpty());

    // TODO: add signal test on release
    buffer->unref();
}

void TestWaylandSurface::testMultipleSurfaces()
{
    using namespace KWayland::Client;
    using namespace KWayland::Server;
    Registry registry;
    QSignalSpy shmSpy(&registry, SIGNAL(shmAnnounced(quint32,quint32)));
    registry.create(m_connection->display());
    QVERIFY(registry.isValid());
    registry.setup();
    QVERIFY(shmSpy.wait());

    ShmPool pool1;
    ShmPool pool2;
    pool1.setup(registry.bindShm(shmSpy.first().first().value<quint32>(), shmSpy.first().last().value<quint32>()));
    pool2.setup(registry.bindShm(shmSpy.first().first().value<quint32>(), shmSpy.first().last().value<quint32>()));
    QVERIFY(pool1.isValid());
    QVERIFY(pool2.isValid());

    // create the surfaces
    QSignalSpy serverSurfaceCreated(m_compositorInterface, SIGNAL(surfaceCreated(KWayland::Server::SurfaceInterface*)));
    QVERIFY(serverSurfaceCreated.isValid());
    QScopedPointer<Surface> s1(m_compositor->createSurface());
    QVERIFY(serverSurfaceCreated.wait());
    SurfaceInterface *serverSurface1 = serverSurfaceCreated.first().first().value<KWayland::Server::SurfaceInterface*>();
    QVERIFY(serverSurface1);
    //second surface
    QScopedPointer<Surface> s2(m_compositor->createSurface());
    QVERIFY(serverSurfaceCreated.wait());
    SurfaceInterface *serverSurface2 = serverSurfaceCreated.last().first().value<KWayland::Server::SurfaceInterface*>();
    QVERIFY(serverSurface2);
    QVERIFY(serverSurface1->resource() != serverSurface2->resource());

    // create two images
    QImage black(24, 24, QImage::Format_RGB32);
    black.fill(Qt::black);
    QImage red(24, 24, QImage::Format_ARGB32);
    red.fill(QColor(255, 0, 0, 128));

    auto blackBuffer = pool1.createBuffer(black);
    auto redBuffer = pool2.createBuffer(red);

    s1->attachBuffer(blackBuffer);
    s1->damage(QRect(0, 0, 24, 24));
    s1->commit(Surface::CommitFlag::None);
    QSignalSpy damageSpy1(serverSurface1, SIGNAL(damaged(QRegion)));
    QVERIFY(damageSpy1.isValid());
    QVERIFY(damageSpy1.wait());

    // now the ServerSurface should have the black image attached as a buffer
    BufferInterface *buffer1 = serverSurface1->buffer();
    QVERIFY(buffer1);
    QImage buffer1Data = buffer1->data();
    QCOMPARE(buffer1Data, black);
    // accessing the same buffer is OK
    QImage buffer1Data2 = buffer1->data();
    QCOMPARE(buffer1Data2, buffer1Data);
    buffer1Data = QImage();
    QVERIFY(buffer1Data.isNull());
    buffer1Data2 = QImage();
    QVERIFY(buffer1Data2.isNull());

    // attach a buffer for the other surface
    s2->attachBuffer(redBuffer);
    s2->damage(QRect(0, 0, 24, 24));
    s2->commit(Surface::CommitFlag::None);
    QSignalSpy damageSpy2(serverSurface2, SIGNAL(damaged(QRegion)));
    QVERIFY(damageSpy2.isValid());
    QVERIFY(damageSpy2.wait());

    BufferInterface *buffer2 = serverSurface2->buffer();
    QVERIFY(buffer2);
    QImage buffer2Data = buffer2->data();
    QCOMPARE(buffer2Data, red);

    // while buffer2 is accessed we cannot access buffer1
    buffer1Data = buffer1->data();
    QVERIFY(buffer1Data.isNull());

    // a deep copy can be kept around
    QImage deepCopy = buffer2Data.copy();
    QCOMPARE(deepCopy, red);
    buffer2Data = QImage();
    QVERIFY(buffer2Data.isNull());
    QCOMPARE(deepCopy, red);

    // now that buffer2Data is destroyed we can access buffer1 again
    buffer1Data = buffer1->data();
    QVERIFY(!buffer1Data.isNull());
    QCOMPARE(buffer1Data, black);
}

void TestWaylandSurface::testOpaque()
{
    using namespace KWayland::Client;
    using namespace KWayland::Server;
    QSignalSpy serverSurfaceCreated(m_compositorInterface, SIGNAL(surfaceCreated(KWayland::Server::SurfaceInterface*)));
    QVERIFY(serverSurfaceCreated.isValid());
    Surface *s = m_compositor->createSurface();
    QVERIFY(serverSurfaceCreated.wait());
    SurfaceInterface *serverSurface = serverSurfaceCreated.first().first().value<KWayland::Server::SurfaceInterface*>();
    QVERIFY(serverSurface);
    QSignalSpy opaqueRegionChangedSpy(serverSurface, SIGNAL(opaqueChanged(QRegion)));
    QVERIFY(opaqueRegionChangedSpy.isValid());

    // by default there should be an empty opaque region
    QCOMPARE(serverSurface->opaque(), QRegion());

    // let's install an opaque region
    s->setOpaqueRegion(m_compositor->createRegion(QRegion(0, 10, 20, 30)).get());
    // the region should only be applied after the surface got committed
    wl_display_flush(m_connection->display());
    QCoreApplication::processEvents();
    QCOMPARE(serverSurface->opaque(), QRegion());
    QCOMPARE(opaqueRegionChangedSpy.count(), 0);

    // so let's commit to get the new region
    s->commit(Surface::CommitFlag::None);
    QVERIFY(opaqueRegionChangedSpy.wait());
    QCOMPARE(opaqueRegionChangedSpy.count(), 1);
    QCOMPARE(opaqueRegionChangedSpy.last().first().value<QRegion>(), QRegion(0, 10, 20, 30));
    QCOMPARE(serverSurface->opaque(), QRegion(0, 10, 20, 30));

    // committing without setting a new region shouldn't change
    s->commit(Surface::CommitFlag::None);
    wl_display_flush(m_connection->display());
    QCoreApplication::processEvents();
    QCOMPARE(opaqueRegionChangedSpy.count(), 1);
    QCOMPARE(serverSurface->opaque(), QRegion(0, 10, 20, 30));

    // let's change the opaque region
    s->setOpaqueRegion(m_compositor->createRegion(QRegion(10, 20, 30, 40)).get());
    s->commit(Surface::CommitFlag::None);
    QVERIFY(opaqueRegionChangedSpy.wait());
    QCOMPARE(opaqueRegionChangedSpy.count(), 2);
    QCOMPARE(opaqueRegionChangedSpy.last().first().value<QRegion>(), QRegion(10, 20, 30, 40));
    QCOMPARE(serverSurface->opaque(), QRegion(10, 20, 30, 40));

    // and let's go back to an empty region
    s->setOpaqueRegion();
    s->commit(Surface::CommitFlag::None);
    QVERIFY(opaqueRegionChangedSpy.wait());
    QCOMPARE(opaqueRegionChangedSpy.count(), 3);
    QCOMPARE(opaqueRegionChangedSpy.last().first().value<QRegion>(), QRegion());
    QCOMPARE(serverSurface->opaque(), QRegion());
}

void TestWaylandSurface::testInput()
{
    using namespace KWayland::Client;
    using namespace KWayland::Server;
    QSignalSpy serverSurfaceCreated(m_compositorInterface, SIGNAL(surfaceCreated(KWayland::Server::SurfaceInterface*)));
    QVERIFY(serverSurfaceCreated.isValid());
    Surface *s = m_compositor->createSurface();
    QVERIFY(serverSurfaceCreated.wait());
    SurfaceInterface *serverSurface = serverSurfaceCreated.first().first().value<KWayland::Server::SurfaceInterface*>();
    QVERIFY(serverSurface);
    QSignalSpy inputRegionChangedSpy(serverSurface, SIGNAL(inputChanged(QRegion)));
    QVERIFY(inputRegionChangedSpy.isValid());

    // by default there should be an empty == infinite input region
    QCOMPARE(serverSurface->input(), QRegion());
    QCOMPARE(serverSurface->inputIsInfinite(), true);

    // let's install an input region
    s->setInputRegion(m_compositor->createRegion(QRegion(0, 10, 20, 30)).get());
    // the region should only be applied after the surface got committed
    wl_display_flush(m_connection->display());
    QCoreApplication::processEvents();
    QCOMPARE(serverSurface->input(), QRegion());
    QCOMPARE(serverSurface->inputIsInfinite(), true);
    QCOMPARE(inputRegionChangedSpy.count(), 0);

    // so let's commit to get the new region
    s->commit(Surface::CommitFlag::None);
    QVERIFY(inputRegionChangedSpy.wait());
    QCOMPARE(inputRegionChangedSpy.count(), 1);
    QCOMPARE(inputRegionChangedSpy.last().first().value<QRegion>(), QRegion(0, 10, 20, 30));
    QCOMPARE(serverSurface->input(), QRegion(0, 10, 20, 30));
    QCOMPARE(serverSurface->inputIsInfinite(), false);

    // committing without setting a new region shouldn't change
    s->commit(Surface::CommitFlag::None);
    wl_display_flush(m_connection->display());
    QCoreApplication::processEvents();
    QCOMPARE(inputRegionChangedSpy.count(), 1);
    QCOMPARE(serverSurface->input(), QRegion(0, 10, 20, 30));
    QCOMPARE(serverSurface->inputIsInfinite(), false);

    // let's change the input region
    s->setInputRegion(m_compositor->createRegion(QRegion(10, 20, 30, 40)).get());
    s->commit(Surface::CommitFlag::None);
    QVERIFY(inputRegionChangedSpy.wait());
    QCOMPARE(inputRegionChangedSpy.count(), 2);
    QCOMPARE(inputRegionChangedSpy.last().first().value<QRegion>(), QRegion(10, 20, 30, 40));
    QCOMPARE(serverSurface->input(), QRegion(10, 20, 30, 40));
    QCOMPARE(serverSurface->inputIsInfinite(), false);

    // and let's go back to an empty region
    s->setInputRegion();
    s->commit(Surface::CommitFlag::None);
    QVERIFY(inputRegionChangedSpy.wait());
    QCOMPARE(inputRegionChangedSpy.count(), 3);
    QCOMPARE(inputRegionChangedSpy.last().first().value<QRegion>(), QRegion());
    QCOMPARE(serverSurface->input(), QRegion());
    QCOMPARE(serverSurface->inputIsInfinite(), true);
}

void TestWaylandSurface::testDestroy()
{
    using namespace KWayland::Client;
    Surface *s = m_compositor->createSurface();

    connect(m_connection, &ConnectionThread::connectionDied, s, &Surface::destroy);
    connect(m_connection, &ConnectionThread::connectionDied, m_compositor, &Compositor::destroy);
    connect(m_connection, &ConnectionThread::connectionDied, m_shm, &ShmPool::destroy);
    connect(m_connection, &ConnectionThread::connectionDied, m_queue, &EventQueue::destroy);
    QVERIFY(s->isValid());

    QSignalSpy connectionDiedSpy(m_connection, SIGNAL(connectionDied()));
    QVERIFY(connectionDiedSpy.isValid());
    delete m_display;
    m_display = nullptr;
    m_compositorInterface = nullptr;
    QVERIFY(connectionDiedSpy.wait());

    // now the Surface should be destroyed;
    QVERIFY(!s->isValid());

    // calling destroy again should not fail
    s->destroy();
}

QTEST_GUILESS_MAIN(TestWaylandSurface)
#include "test_wayland_surface.moc"
