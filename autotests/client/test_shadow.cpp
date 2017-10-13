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
#include "../../src/client/connection_thread.h"
#include "../../src/client/compositor.h"
#include "../../src/client/event_queue.h"
#include "../../src/client/registry.h"
#include "../../src/client/shadow.h"
#include "../../src/client/shm_pool.h"
#include "../../src/client/surface.h"
// server
#include "../../src/server/buffer_interface.h"
#include "../../src/server/display.h"
#include "../../src/server/compositor_interface.h"
#include "../../src/server/shadow_interface.h"

using namespace KWayland::Client;
using namespace KWayland::Server;

class ShadowTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init();
    void cleanup();

    void testCreateShadow();
    void testShadowElements();
    void testSurfaceDestroy();

private:
    Display *m_display = nullptr;

    ConnectionThread *m_connection = nullptr;
    CompositorInterface *m_compositorInterface = nullptr;
    ShadowManagerInterface *m_shadowInterface = nullptr;
    QThread *m_thread = nullptr;
    EventQueue *m_queue = nullptr;
    ShmPool *m_shm = nullptr;
    Compositor *m_compositor = nullptr;
    ShadowManager *m_shadow = nullptr;
};

static const QString s_socketName = QStringLiteral("kwayland-test-shadow-0");

void ShadowTest::init()
{
    delete m_display;
    m_display = new Display(this);
    m_display->setSocketName(s_socketName);
    m_display->start();
    QVERIFY(m_display->isRunning());
    m_display->createShm();
    m_compositorInterface = m_display->createCompositor(m_display);
    m_compositorInterface->create();
    m_shadowInterface = m_display->createShadowManager(m_display);
    m_shadowInterface->create();

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
    registry.setEventQueue(m_queue);
    registry.create(m_connection);
    QVERIFY(registry.isValid());
    registry.setup();
    QVERIFY(interfacesAnnouncedSpy.wait());

    m_shm = registry.createShmPool(registry.interface(Registry::Interface::Shm).name,
                                   registry.interface(Registry::Interface::Shm).version,
                                   this);
    QVERIFY(m_shm->isValid());
    m_compositor = registry.createCompositor(registry.interface(Registry::Interface::Compositor).name,
                                             registry.interface(Registry::Interface::Compositor).version,
                                             this);
    QVERIFY(m_compositor->isValid());
    m_shadow = registry.createShadowManager(registry.interface(Registry::Interface::Shadow).name,
                                            registry.interface(Registry::Interface::Shadow).version,
                                            this);
    QVERIFY(m_shadow->isValid());
}

void ShadowTest::cleanup()
{
#define CLEANUP(variable) \
    if (variable) { \
        delete variable; \
        variable = nullptr; \
    }
    CLEANUP(m_shm)
    CLEANUP(m_compositor)
    CLEANUP(m_shadow)
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
    CLEANUP(m_shadowInterface)
    CLEANUP(m_display)
#undef CLEANUP
}

void ShadowTest::testCreateShadow()
{
    // this test verifies the basic shadow behavior, create for surface, commit it, etc.
    QSignalSpy surfaceCreatedSpy(m_compositorInterface, &CompositorInterface::surfaceCreated);
    QVERIFY(surfaceCreatedSpy.isValid());
    QScopedPointer<Surface> surface(m_compositor->createSurface());
    QVERIFY(surfaceCreatedSpy.wait());
    auto serverSurface = surfaceCreatedSpy.first().first().value<SurfaceInterface*>();
    QVERIFY(serverSurface);
    // a surface without anything should not have a Shadow
    QVERIFY(!serverSurface->shadow());
    QSignalSpy shadowChangedSpy(serverSurface, &SurfaceInterface::shadowChanged);
    QVERIFY(shadowChangedSpy.isValid());

    // let's create a shadow for the Surface
    QScopedPointer<Shadow> shadow(m_shadow->createShadow(surface.data()));
    // that should not have triggered the shadowChangedSpy)
    QVERIFY(!shadowChangedSpy.wait(100));

    // now let's commit the surface, that should trigger the shadow changed
    surface->commit(Surface::CommitFlag::None);
    QVERIFY(shadowChangedSpy.wait());
    QCOMPARE(shadowChangedSpy.count(), 1);

    // we didn't set anything on the shadow, so it should be all default values
    auto serverShadow = serverSurface->shadow();
    QVERIFY(serverShadow);
    QCOMPARE(serverShadow->offset(), QMarginsF());
    QVERIFY(!serverShadow->topLeft());
    QVERIFY(!serverShadow->top());
    QVERIFY(!serverShadow->topRight());
    QVERIFY(!serverShadow->right());
    QVERIFY(!serverShadow->bottomRight());
    QVERIFY(!serverShadow->bottom());
    QVERIFY(!serverShadow->bottomLeft());
    QVERIFY(!serverShadow->left());

    // now let's remove the shadow
    m_shadow->removeShadow(surface.data());
    // just removing should not remove it yet, surface needs to be committed
    QVERIFY(!shadowChangedSpy.wait(100));
    surface->commit(Surface::CommitFlag::None);
    QVERIFY(shadowChangedSpy.wait());
    QCOMPARE(shadowChangedSpy.count(), 2);
    QVERIFY(!serverSurface->shadow());
}

void ShadowTest::testShadowElements()
{
    // this test verifies that all shadow elements are correctly passed to the server
    // first create surface
    QSignalSpy surfaceCreatedSpy(m_compositorInterface, &CompositorInterface::surfaceCreated);
    QVERIFY(surfaceCreatedSpy.isValid());
    QScopedPointer<Surface> surface(m_compositor->createSurface());
    QVERIFY(surfaceCreatedSpy.wait());
    auto serverSurface = surfaceCreatedSpy.first().first().value<SurfaceInterface*>();
    QVERIFY(serverSurface);
    QSignalSpy shadowChangedSpy(serverSurface, &SurfaceInterface::shadowChanged);
    QVERIFY(shadowChangedSpy.isValid());

    // now create the shadow
    QScopedPointer<Shadow> shadow(m_shadow->createShadow(surface.data()));
    QImage topLeftImage(QSize(10, 10), QImage::Format_ARGB32_Premultiplied);
    topLeftImage.fill(Qt::white);
    shadow->attachTopLeft(m_shm->createBuffer(topLeftImage));
    QImage topImage(QSize(11, 11), QImage::Format_ARGB32_Premultiplied);
    topImage.fill(Qt::black);
    shadow->attachTop(m_shm->createBuffer(topImage));
    QImage topRightImage(QSize(12, 12), QImage::Format_ARGB32_Premultiplied);
    topRightImage.fill(Qt::red);
    shadow->attachTopRight(m_shm->createBuffer(topRightImage));
    QImage rightImage(QSize(13, 13), QImage::Format_ARGB32_Premultiplied);
    rightImage.fill(Qt::darkRed);
    shadow->attachRight(m_shm->createBuffer(rightImage));
    QImage bottomRightImage(QSize(14, 14), QImage::Format_ARGB32_Premultiplied);
    bottomRightImage.fill(Qt::green);
    shadow->attachBottomRight(m_shm->createBuffer(bottomRightImage));
    QImage bottomImage(QSize(15, 15), QImage::Format_ARGB32_Premultiplied);
    bottomImage.fill(Qt::darkGreen);
    shadow->attachBottom(m_shm->createBuffer(bottomImage));
    QImage bottomLeftImage(QSize(16, 16), QImage::Format_ARGB32_Premultiplied);
    bottomLeftImage.fill(Qt::blue);
    shadow->attachBottomLeft(m_shm->createBuffer(bottomLeftImage));
    QImage leftImage(QSize(17, 17), QImage::Format_ARGB32_Premultiplied);
    leftImage.fill(Qt::darkBlue);
    shadow->attachLeft(m_shm->createBuffer(leftImage));
    shadow->setOffsets(QMarginsF(1, 2, 3, 4));
    shadow->commit();
    surface->commit(Surface::CommitFlag::None);

    QVERIFY(shadowChangedSpy.wait());
    auto serverShadow = serverSurface->shadow();
    QVERIFY(serverShadow);
    QCOMPARE(serverShadow->offset(), QMarginsF(1, 2, 3, 4));
    QCOMPARE(serverShadow->topLeft()->data(), topLeftImage);
    QCOMPARE(serverShadow->top()->data(), topImage);
    QCOMPARE(serverShadow->topRight()->data(), topRightImage);
    QCOMPARE(serverShadow->right()->data(), rightImage);
    QCOMPARE(serverShadow->bottomRight()->data(), bottomRightImage);
    QCOMPARE(serverShadow->bottom()->data(), bottomImage);
    QCOMPARE(serverShadow->bottomLeft()->data(), bottomLeftImage);
    QCOMPARE(serverShadow->left()->data(), leftImage);

    // try to destroy the buffer
    // first attach one buffer
    shadow->attachTopLeft(m_shm->createBuffer(topLeftImage));
    // create a destroyed signal
    QSignalSpy destroyedSpy(serverShadow->topLeft(), &BufferInterface::aboutToBeDestroyed);
    QVERIFY(destroyedSpy.isValid());
    delete m_shm;
    m_shm = nullptr;
    QVERIFY(destroyedSpy.wait());

    // now all buffers should be gone
    // TODO: does that need a signal?
    QVERIFY(!serverShadow->topLeft());
    QVERIFY(!serverShadow->top());
    QVERIFY(!serverShadow->topRight());
    QVERIFY(!serverShadow->right());
    QVERIFY(!serverShadow->bottomRight());
    QVERIFY(!serverShadow->bottom());
    QVERIFY(!serverShadow->bottomLeft());
    QVERIFY(!serverShadow->left());
}

void ShadowTest::testSurfaceDestroy()
{
    using namespace KWayland::Client;
    using namespace KWayland::Server;
    QSignalSpy serverSurfaceCreated(m_compositorInterface, &CompositorInterface::surfaceCreated);
    QVERIFY(serverSurfaceCreated.isValid());

    QScopedPointer<KWayland::Client::Surface> surface(m_compositor->createSurface());
    QVERIFY(serverSurfaceCreated.wait());
    auto serverSurface = serverSurfaceCreated.first().first().value<SurfaceInterface*>();
    QSignalSpy shadowChangedSpy(serverSurface, &SurfaceInterface::shadowChanged);
    QVERIFY(shadowChangedSpy.isValid());

    QScopedPointer<Shadow> shadow(m_shadow->createShadow(surface.data()));
    shadow->commit();
    surface->commit(Surface::CommitFlag::None);
    QVERIFY(shadowChangedSpy.wait());
    auto serverShadow = serverSurface->shadow();
    QVERIFY(serverShadow);

    // destroy the parent surface
    QSignalSpy surfaceDestroyedSpy(serverSurface, &QObject::destroyed);
    QVERIFY(surfaceDestroyedSpy.isValid());
    QSignalSpy shadowDestroyedSpy(serverShadow.data(), &QObject::destroyed);
    QVERIFY(shadowDestroyedSpy.isValid());
    surface.reset();
    QVERIFY(surfaceDestroyedSpy.wait());
    QVERIFY(shadowDestroyedSpy.isEmpty());
    // destroy the shadow
    shadow.reset();
    QVERIFY(shadowDestroyedSpy.wait());
    QCOMPARE(shadowDestroyedSpy.count(), 1);
}

QTEST_GUILESS_MAIN(ShadowTest)
#include "test_shadow.moc"
