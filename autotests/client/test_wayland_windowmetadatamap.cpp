/********************************************************************
Copyright 2014 Martin Gräßlin <mgraesslin@kde.org>
Copyright 2015 Sebastian Kügler <sebas@kde.org>

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
#include "../../src/client/connection_thread.h"
#include "../../src/client/event_queue.h"
#include "../../src/client/windowmetadatamap.h"
#include "../../src/client/registry.h"

#include "../../src/server/display.h"
// #include "../../src/server/shell_interface.h"
// #include "../../src/server/compositor_interface.h"
// #include "../../src/server/outputconfiguration_interface.h"
// #include "../../src/server/outputdevice_interface.h"
#include "../../src/server/surface_interface.h"
#include "../../src/server/windowmetadatamap_interface.h"

// Wayland
#include <wayland-client-protocol.h>

using namespace KWayland::Client;
using namespace KWayland::Server;

class TestWaylandWindowMetadataMap : public QObject
{
    Q_OBJECT
public:
    explicit TestWaylandWindowMetadataMap(QObject *parent = nullptr);
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

    void testCreate();

private:
    KWayland::Server::Display *m_display;
    KWayland::Server::WindowMetadataMapInterface *m_windowMetadataMapInterface;


    KWayland::Client::Registry m_registry;
    KWayland::Client::WindowMetadataMap m_windowMetadataMap;

    KWayland::Client::ConnectionThread *m_connection;
    KWayland::Client::EventQueue *m_queue;
    QThread *m_thread;

    QSignalSpy *m_announcedSpy;
    QSignalSpy *m_regSpy;
};

static const QString s_socketName = QStringLiteral("kwin-test-wayland-windowmetadatamap-0");

TestWaylandWindowMetadataMap::TestWaylandWindowMetadataMap(QObject *parent)
    : QObject(parent)
    , m_display(nullptr)
    , m_windowMetadataMapInterface(nullptr)
    , m_connection(nullptr)
    , m_queue(nullptr)
    , m_thread(nullptr)
    , m_announcedSpy(nullptr)
    , m_regSpy(nullptr)
{
}

void TestWaylandWindowMetadataMap::initTestCase()
{
    using namespace KWayland::Server;
    delete m_display;
    m_display = new Display(this);
    m_display->setSocketName(s_socketName);
    m_display->start();
    QVERIFY(m_display->isRunning());


    m_windowMetadataMapInterface = m_display->createWindowMetadataMap(this);
    m_windowMetadataMapInterface->create();
    QVERIFY(m_windowMetadataMapInterface->isValid());

    // setup connection
    m_connection = new KWayland::Client::ConnectionThread;
    QSignalSpy connectedSpy(m_connection, &KWayland::Client::ConnectionThread::connected);
    m_connection->setSocketName(s_socketName);
    connect(m_connection, &ConnectionThread::connectionDied, &m_windowMetadataMap, &WindowMetadataMap::destroy);

    m_thread = new QThread(this);
    m_connection->moveToThread(m_thread);
    m_thread->start();

    m_connection->initConnection();
    QVERIFY(connectedSpy.wait());

    m_queue = new KWayland::Client::EventQueue(this);
    QVERIFY(!m_queue->isValid());
    m_queue->setup(m_connection);
    QVERIFY(m_queue->isValid());
}

void TestWaylandWindowMetadataMap::cleanupTestCase()
{
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

void TestWaylandWindowMetadataMap::testCreate()
{
    m_announcedSpy = new QSignalSpy(&m_registry, &KWayland::Client::Registry::windowMetadataMapAnnounced);
    QVERIFY(m_announcedSpy->isValid());

    qRegisterMetaType<SurfaceInterface*>();
    m_regSpy = new QSignalSpy(m_windowMetadataMapInterface, &KWayland::Server::WindowMetadataMapInterface::clientRegistered);
    QVERIFY(m_regSpy->isValid());

    m_registry.create(m_connection->display());
    QVERIFY(m_registry.isValid());
    m_registry.setEventQueue(m_queue);
    m_registry.setup();
    wl_display_flush(m_connection->display());

    QVERIFY(m_announcedSpy->wait());
    QCOMPARE(m_announcedSpy->count(), 1);

    m_windowMetadataMap.setup(m_registry.bindWindowMetadataMap(m_announcedSpy->first().first().value<quint32>(), m_announcedSpy->first().last().value<quint32>()));

    m_windowMetadataMap.registerClient(QStringLiteral("BlaFarghl"), nullptr);

    QVERIFY(m_regSpy->wait());
    QCOMPARE(m_regSpy->count(), 1);

    m_windowMetadataMap.destroy();
}


QTEST_GUILESS_MAIN(TestWaylandWindowMetadataMap)
#include "test_wayland_windowmetadatamap.moc"
