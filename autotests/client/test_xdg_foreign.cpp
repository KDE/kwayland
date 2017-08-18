/********************************************************************
Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
Copyright 2017  Marco Martin <mart@kde.org>

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
#include "../../src/client/xdgforeign_v1.h"
#include "../../src/server/display.h"
#include "../../src/server/compositor_interface.h"
#include "../../src/server/surface_interface.h"
#include "../../src/server/xdgforeign_interface.h"

using namespace KWayland::Client;

class TestForeign : public QObject
{
    Q_OBJECT
public:
    explicit TestForeign(QObject *parent = nullptr);
private Q_SLOTS:
    void init();
    void cleanup();

    void testExport();
    void testDeleteImported();
    void testDeleteChildSurface();
    void testDeleteParentSurface();
    void testDeleteExported();

private:
    void doExport();

    KWayland::Server::Display *m_display;
    KWayland::Server::CompositorInterface *m_compositorInterface;
    KWayland::Server::XdgForeignUnstableInterface *m_foreignInterface;
    KWayland::Client::ConnectionThread *m_connection;
    KWayland::Client::Compositor *m_compositor;
    KWayland::Client::EventQueue *m_queue;
    KWayland::Client::XdgExporterUnstableV1 *m_exporter;
    KWayland::Client::XdgImporterUnstableV1 *m_importer;

    QPointer<KWayland::Client::Surface> m_exportedSurface;
    QPointer<KWayland::Server::SurfaceInterface> m_exportedSurfaceInterface;

    QPointer<KWayland::Client::XdgExportedUnstableV1> m_exported;
    QPointer<KWayland::Client::XdgImportedUnstableV1> m_imported;

    QPointer<KWayland::Client::Surface> m_childSurface;
    QPointer<KWayland::Server::SurfaceInterface> m_childSurfaceInterface;

    

    QThread *m_thread;
};

static const QString s_socketName = QStringLiteral("kwayland-test-xdg-foreign-0");

TestForeign::TestForeign(QObject *parent)
    : QObject(parent)
    , m_display(nullptr)
    , m_compositorInterface(nullptr)
    , m_connection(nullptr)
    , m_compositor(nullptr)
    , m_queue(nullptr)
    , m_exporter(nullptr)
    , m_importer(nullptr)
    , m_thread(nullptr)
{
}

void TestForeign::init()
{
    using namespace KWayland::Server;
    delete m_display;
    m_display = new Display(this);
    m_display->setSocketName(s_socketName);
    m_display->start();
    QVERIFY(m_display->isRunning());

    qRegisterMetaType<KWayland::Server::SurfaceInterface*>("KWayland::Server::SurfaceInterface");

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

    QSignalSpy exporterSpy(&registry, &Registry::exporterUnstableV1Announced);
    QVERIFY(exporterSpy.isValid());

    QSignalSpy importerSpy(&registry, &Registry::importerUnstableV1Announced);
    QVERIFY(importerSpy.isValid());

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

    m_foreignInterface = m_display->createXdgForeignUnstableInterface(m_display);
    m_foreignInterface->create();
    QVERIFY(m_foreignInterface->isValid());
    
    QVERIFY(exporterSpy.wait());
    //Both importer and exporter should have been triggered by now
    QCOMPARE(exporterSpy.count(), 1);
    QCOMPARE(importerSpy.count(), 1);

    m_exporter = registry.createXdgExporterUnstableV1(exporterSpy.first().first().value<quint32>(), exporterSpy.first().last().value<quint32>(), this);
    m_importer = registry.createXdgImporterUnstableV1(importerSpy.first().first().value<quint32>(), importerSpy.first().last().value<quint32>(), this);
}

void TestForeign::cleanup()
{
#define CLEANUP(variable) \
    if (variable) { \
        delete variable; \
        variable = nullptr; \
    }

    CLEANUP(m_exported)
    CLEANUP(m_exporter)
    CLEANUP(m_imported)
    CLEANUP(m_importer)


    CLEANUP(m_exportedSurface)
    CLEANUP(m_childSurface)

    CLEANUP(m_compositor)
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

    CLEANUP(m_foreignInterface)
    CLEANUP(m_exportedSurfaceInterface)
    CLEANUP(m_childSurfaceInterface)

    CLEANUP(m_display)
#undef CLEANUP
}

void TestForeign::doExport()
{
    QSignalSpy serverSurfaceCreated(m_compositorInterface, SIGNAL(surfaceCreated(KWayland::Server::SurfaceInterface*)));
    QVERIFY(serverSurfaceCreated.isValid());

    m_exportedSurface = m_compositor->createSurface();
    QVERIFY(serverSurfaceCreated.wait());

    m_exportedSurfaceInterface = serverSurfaceCreated.first().first().value<KWayland::Server::SurfaceInterface*>();

    //Export a window
    m_exported = m_exporter->exportSurface(m_exportedSurface, this);
    QVERIFY(m_exported->handle().isEmpty());
    QSignalSpy doneSpy(m_exported.data(), &XdgExportedUnstableV1::done);
    QVERIFY(doneSpy.wait());
    QVERIFY(!m_exported->handle().isEmpty());

    QSignalSpy transientSpy(m_foreignInterface, &KWayland::Server::XdgForeignUnstableInterface::transientChanged);
    QVERIFY(transientSpy.isValid());

    //Import the just exported window
    m_imported = m_importer->import(m_exported->handle(), this);
    QVERIFY(m_imported->isValid());

    QSignalSpy childSurfaceInterfaceCreated(m_compositorInterface, SIGNAL(surfaceCreated(KWayland::Server::SurfaceInterface*)));
    QVERIFY(serverSurfaceCreated.isValid());
    m_childSurface = m_compositor->createSurface();
    QVERIFY(childSurfaceInterfaceCreated.wait());
    m_childSurfaceInterface = childSurfaceInterfaceCreated.first().first().value<KWayland::Server::SurfaceInterface*>();
    m_childSurface->commit(Surface::CommitFlag::None);

    m_imported->setParentOf(m_childSurface);
    QVERIFY(transientSpy.wait());

    QCOMPARE(transientSpy.first().first().value<KWayland::Server::SurfaceInterface *>(), m_childSurfaceInterface.data());
    QCOMPARE(transientSpy.first().at(1).value<KWayland::Server::SurfaceInterface *>(), m_exportedSurfaceInterface.data());

    //transientFor api
    QCOMPARE(m_foreignInterface->transientFor(m_childSurfaceInterface), m_exportedSurfaceInterface.data());
}

void TestForeign::testExport()
{
    doExport();
}

void TestForeign::testDeleteImported()
{
    doExport();

    QSignalSpy transientSpy(m_foreignInterface, &KWayland::Server::XdgForeignUnstableInterface::transientChanged);
 
    QVERIFY(transientSpy.isValid());
    m_imported->deleteLater();
    m_imported = nullptr;

    QVERIFY(transientSpy.wait());

    QCOMPARE(transientSpy.first().first().value<KWayland::Server::SurfaceInterface *>(), m_childSurfaceInterface.data());
    QCOMPARE(transientSpy.first().at(1).value<KWayland::Server::SurfaceInterface *>(), nullptr);
    QCOMPARE(m_foreignInterface->transientFor(m_childSurfaceInterface), nullptr);
}

void TestForeign::testDeleteChildSurface()
{
    doExport();

    QSignalSpy transientSpy(m_foreignInterface, &KWayland::Server::XdgForeignUnstableInterface::transientChanged);
 
    QVERIFY(transientSpy.isValid());
    m_childSurface->deleteLater();

    QVERIFY(transientSpy.wait());

    //when the client surface dies, the server one will eventually die too
    QSignalSpy surfaceDestroyedSpy(m_childSurfaceInterface, SIGNAL(destroyed()));
    QVERIFY(surfaceDestroyedSpy.wait());

    QCOMPARE(transientSpy.first().at(0).value<KWayland::Server::SurfaceInterface *>(), nullptr);
    QCOMPARE(transientSpy.first().at(1).value<KWayland::Server::SurfaceInterface *>(), m_exportedSurfaceInterface.data());    
}

void TestForeign::testDeleteParentSurface()
{
    doExport();

    QSignalSpy transientSpy(m_foreignInterface, &KWayland::Server::XdgForeignUnstableInterface::transientChanged);
 
    QVERIFY(transientSpy.isValid());
    m_exportedSurface->deleteLater();

    QVERIFY(transientSpy.wait());

    QCOMPARE(transientSpy.first().first().value<KWayland::Server::SurfaceInterface *>(), m_childSurfaceInterface.data());
    QCOMPARE(transientSpy.first().at(1).value<KWayland::Server::SurfaceInterface *>(), nullptr);
    QCOMPARE(m_foreignInterface->transientFor(m_childSurfaceInterface), nullptr);   
}

void TestForeign::testDeleteExported()
{
    doExport();

    QSignalSpy transientSpy(m_foreignInterface, &KWayland::Server::XdgForeignUnstableInterface::transientChanged);
    QSignalSpy destroyedSpy(m_imported.data(), &KWayland::Client::XdgImportedUnstableV1::importedDestroyed);
 
    QVERIFY(transientSpy.isValid());
    m_exported->deleteLater();
    m_exported = nullptr;

    QVERIFY(transientSpy.wait());
    QVERIFY(destroyedSpy.wait());

    QCOMPARE(transientSpy.first().first().value<KWayland::Server::SurfaceInterface *>(), m_childSurfaceInterface.data());
    QCOMPARE(transientSpy.first().at(1).value<KWayland::Server::SurfaceInterface *>(), nullptr);
    QCOMPARE(m_foreignInterface->transientFor(m_childSurfaceInterface), nullptr);

    QVERIFY(!m_imported->isValid());
}

QTEST_GUILESS_MAIN(TestForeign)
#include "test_xdg_foreign.moc"
