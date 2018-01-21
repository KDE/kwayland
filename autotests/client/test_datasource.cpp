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
#include <QMimeDatabase>
// KWayland
#include "../../src/client/connection_thread.h"
#include "../../src/client/event_queue.h"
#include "../../src/client/datadevicemanager.h"
#include "../../src/client/datasource.h"
#include "../../src/client/registry.h"
#include "../../src/server/display.h"
#include "../../src/server/datadevicemanager_interface.h"
#include "../../src/server/datasource_interface.h"
// Wayland
#include <wayland-client.h>

class TestDataSource : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void init();
    void cleanup();

    void testOffer();
    void testTargetAccepts_data();
    void testTargetAccepts();
    void testRequestSend();
    void testRequestSendOnUnbound();
    void testCancel();
    void testServerGet();
    void testDestroy();

private:
    KWayland::Server::Display *m_display = nullptr;
    KWayland::Server::DataDeviceManagerInterface *m_dataDeviceManagerInterface = nullptr;
    KWayland::Client::ConnectionThread *m_connection = nullptr;
    KWayland::Client::DataDeviceManager *m_dataDeviceManager = nullptr;
    KWayland::Client::EventQueue *m_queue = nullptr;
    QThread *m_thread = nullptr;
};

static const QString s_socketName = QStringLiteral("kwayland-test-wayland-datasource-0");

void TestDataSource::init()
{
    using namespace KWayland::Server;
    delete m_display;
    m_display = new Display(this);
    m_display->setSocketName(s_socketName);
    m_display->start();
    QVERIFY(m_display->isRunning());

    // setup connection
    m_connection = new KWayland::Client::ConnectionThread;
    QSignalSpy connectedSpy(m_connection, SIGNAL(connected()));
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
    QSignalSpy dataDeviceManagerSpy(&registry, SIGNAL(dataDeviceManagerAnnounced(quint32,quint32)));
    QVERIFY(dataDeviceManagerSpy.isValid());
    QVERIFY(!registry.eventQueue());
    registry.setEventQueue(m_queue);
    QCOMPARE(registry.eventQueue(), m_queue);
    registry.create(m_connection->display());
    QVERIFY(registry.isValid());
    registry.setup();

    m_dataDeviceManagerInterface = m_display->createDataDeviceManager(m_display);
    m_dataDeviceManagerInterface->create();
    QVERIFY(m_dataDeviceManagerInterface->isValid());

    QVERIFY(dataDeviceManagerSpy.wait());
    m_dataDeviceManager = registry.createDataDeviceManager(dataDeviceManagerSpy.first().first().value<quint32>(),
                                                           dataDeviceManagerSpy.first().last().value<quint32>(), this);
}

void TestDataSource::cleanup()
{
    if (m_dataDeviceManager) {
        delete m_dataDeviceManager;
        m_dataDeviceManager = nullptr;
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

void TestDataSource::testOffer()
{
    using namespace KWayland::Client;
    using namespace KWayland::Server;

    qRegisterMetaType<KWayland::Server::DataSourceInterface*>();
    QSignalSpy dataSourceCreatedSpy(m_dataDeviceManagerInterface, SIGNAL(dataSourceCreated(KWayland::Server::DataSourceInterface*)));
    QVERIFY(dataSourceCreatedSpy.isValid());

    QScopedPointer<DataSource> dataSource(m_dataDeviceManager->createDataSource());
    QVERIFY(dataSource->isValid());

    QVERIFY(dataSourceCreatedSpy.wait());
    QCOMPARE(dataSourceCreatedSpy.count(), 1);

    QPointer<DataSourceInterface> serverDataSource = dataSourceCreatedSpy.first().first().value<DataSourceInterface*>();
    QVERIFY(!serverDataSource.isNull());
    QCOMPARE(serverDataSource->mimeTypes().count(), 0);
    QVERIFY(serverDataSource->parentResource());

    QSignalSpy offeredSpy(serverDataSource.data(), SIGNAL(mimeTypeOffered(QString)));
    QVERIFY(offeredSpy.isValid());

    const QString plain = QStringLiteral("text/plain");
    QMimeDatabase db;
    dataSource->offer(db.mimeTypeForName(plain));

    QVERIFY(offeredSpy.wait());
    QCOMPARE(offeredSpy.count(), 1);
    QCOMPARE(offeredSpy.last().first().toString(), plain);
    QCOMPARE(serverDataSource->mimeTypes().count(), 1);
    QCOMPARE(serverDataSource->mimeTypes().first(), plain);

    const QString html = QStringLiteral("text/html");
    dataSource->offer(db.mimeTypeForName(html));

    QVERIFY(offeredSpy.wait());
    QCOMPARE(offeredSpy.count(), 2);
    QCOMPARE(offeredSpy.first().first().toString(), plain);
    QCOMPARE(offeredSpy.last().first().toString(), html);
    QCOMPARE(serverDataSource->mimeTypes().count(), 2);
    QCOMPARE(serverDataSource->mimeTypes().first(), plain);
    QCOMPARE(serverDataSource->mimeTypes().last(), html);

    // try destroying the client side, should trigger a destroy of server side
    dataSource.reset();
    QVERIFY(!serverDataSource.isNull());
    wl_display_flush(m_connection->display());
    // after running the event loop the Wayland event should be delivered, but it uses delete later
    QCoreApplication::processEvents();
    QVERIFY(!serverDataSource.isNull());
    // so once more event loop
    QCoreApplication::processEvents();
    QVERIFY(serverDataSource.isNull());
}

void TestDataSource::testTargetAccepts_data()
{
    QTest::addColumn<QString>("mimeType");

    QTest::newRow("empty") << QString();
    QTest::newRow("text/plain") << QStringLiteral("text/plain");
}

void TestDataSource::testTargetAccepts()
{
    using namespace KWayland::Client;
    using namespace KWayland::Server;

    QSignalSpy dataSourceCreatedSpy(m_dataDeviceManagerInterface, SIGNAL(dataSourceCreated(KWayland::Server::DataSourceInterface*)));
    QVERIFY(dataSourceCreatedSpy.isValid());

    QScopedPointer<DataSource> dataSource(m_dataDeviceManager->createDataSource());
    QVERIFY(dataSource->isValid());

    QSignalSpy targetAcceptsSpy(dataSource.data(), SIGNAL(targetAccepts(QString)));
    QVERIFY(targetAcceptsSpy.isValid());

    QVERIFY(dataSourceCreatedSpy.wait());
    QCOMPARE(dataSourceCreatedSpy.count(), 1);

    QFETCH(QString, mimeType);
    dataSourceCreatedSpy.first().first().value<DataSourceInterface*>()->accept(mimeType);

    QVERIFY(targetAcceptsSpy.wait());
    QCOMPARE(targetAcceptsSpy.count(), 1);
    QCOMPARE(targetAcceptsSpy.first().first().toString(), mimeType);
}

void TestDataSource::testRequestSend()
{
    using namespace KWayland::Client;
    using namespace KWayland::Server;

    QSignalSpy dataSourceCreatedSpy(m_dataDeviceManagerInterface, SIGNAL(dataSourceCreated(KWayland::Server::DataSourceInterface*)));
    QVERIFY(dataSourceCreatedSpy.isValid());

    QScopedPointer<DataSource> dataSource(m_dataDeviceManager->createDataSource());
    QVERIFY(dataSource->isValid());

    QSignalSpy sendRequestedSpy(dataSource.data(), SIGNAL(sendDataRequested(QString,qint32)));
    QVERIFY(sendRequestedSpy.isValid());

    const QString plain = QStringLiteral("text/plain");
    QVERIFY(dataSourceCreatedSpy.wait());
    QCOMPARE(dataSourceCreatedSpy.count(), 1);
    QTemporaryFile file;
    QVERIFY(file.open());
    dataSourceCreatedSpy.first().first().value<DataSourceInterface*>()->requestData(plain, file.handle());

    QVERIFY(sendRequestedSpy.wait());
    QCOMPARE(sendRequestedSpy.count(), 1);
    QCOMPARE(sendRequestedSpy.first().first().toString(), plain);
    QCOMPARE(sendRequestedSpy.first().last().value<qint32>(), file.handle());
    QVERIFY(sendRequestedSpy.first().last().value<qint32>() != -1);

    QFile writeFile;
    QVERIFY(writeFile.open(sendRequestedSpy.first().last().value<qint32>(), QFile::WriteOnly, QFileDevice::AutoCloseHandle));
    writeFile.close();
}

void TestDataSource::testRequestSendOnUnbound()
{
    // this test verifies that the server doesn't crash when requesting a send on an unbound DataSource
    using namespace KWayland::Client;
    using namespace KWayland::Server;
    QSignalSpy dataSourceCreatedSpy(m_dataDeviceManagerInterface, &DataDeviceManagerInterface::dataSourceCreated);
    QVERIFY(dataSourceCreatedSpy.isValid());

    QScopedPointer<DataSource> dataSource(m_dataDeviceManager->createDataSource());
    QVERIFY(dataSource->isValid());
    QVERIFY(dataSourceCreatedSpy.wait());
    QCOMPARE(dataSourceCreatedSpy.count(), 1);
    auto sds = dataSourceCreatedSpy.first().first().value<DataSourceInterface*>();
    QVERIFY(sds);

    QSignalSpy unboundSpy(sds, &Resource::unbound);
    QVERIFY(unboundSpy.isValid());
    dataSource.reset();
    QVERIFY(unboundSpy.wait());
    sds->requestData(QStringLiteral("text/plain"), -1);
}

void TestDataSource::testCancel()
{
    using namespace KWayland::Client;
    using namespace KWayland::Server;

    QSignalSpy dataSourceCreatedSpy(m_dataDeviceManagerInterface, SIGNAL(dataSourceCreated(KWayland::Server::DataSourceInterface*)));
    QVERIFY(dataSourceCreatedSpy.isValid());

    QScopedPointer<DataSource> dataSource(m_dataDeviceManager->createDataSource());
    QVERIFY(dataSource->isValid());
    QSignalSpy cancelledSpy(dataSource.data(), SIGNAL(cancelled()));
    QVERIFY(cancelledSpy.isValid());

    QVERIFY(dataSourceCreatedSpy.wait());

    QCOMPARE(cancelledSpy.count(), 0);
    dataSourceCreatedSpy.first().first().value<DataSourceInterface*>()->cancel();

    QVERIFY(cancelledSpy.wait());
    QCOMPARE(cancelledSpy.count(), 1);
}

void TestDataSource::testServerGet()
{
    using namespace KWayland::Client;
    using namespace KWayland::Server;

    QSignalSpy dataSourceCreatedSpy(m_dataDeviceManagerInterface, SIGNAL(dataSourceCreated(KWayland::Server::DataSourceInterface*)));
    QVERIFY(dataSourceCreatedSpy.isValid());

    QScopedPointer<DataSource> dataSource(m_dataDeviceManager->createDataSource());
    QVERIFY(dataSource->isValid());

    QVERIFY(!DataSourceInterface::get(nullptr));
    QVERIFY(dataSourceCreatedSpy.wait());
    auto d = dataSourceCreatedSpy.first().first().value<DataSourceInterface*>();

    QCOMPARE(DataSourceInterface::get(d->resource()), d);
    QVERIFY(!DataSourceInterface::get(nullptr));
}

void TestDataSource::testDestroy()
{
    using namespace KWayland::Client;

    QScopedPointer<DataSource> dataSource(m_dataDeviceManager->createDataSource());
    QVERIFY(dataSource->isValid());

    connect(m_connection, &ConnectionThread::connectionDied, m_dataDeviceManager, &DataDeviceManager::destroy);
    connect(m_connection, &ConnectionThread::connectionDied, m_queue, &EventQueue::destroy);
    connect(m_connection, &ConnectionThread::connectionDied, dataSource.data(), &DataSource::destroy);

    QSignalSpy connectionDiedSpy(m_connection, SIGNAL(connectionDied()));
    QVERIFY(connectionDiedSpy.isValid());
    delete m_display;
    m_display = nullptr;
    QVERIFY(connectionDiedSpy.wait());

    // now the pool should be destroyed;
    QVERIFY(!dataSource->isValid());

    // calling destroy again should not fail
    dataSource->destroy();
}

QTEST_GUILESS_MAIN(TestDataSource)
#include "test_datasource.moc"
