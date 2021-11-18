/*
    SPDX-FileCopyrightText: 2018 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
// Qt
#include <QSignalSpy>
#include <QTest>
// KWin
#include "../../src/client/connection_thread.h"
#include "../../src/client/dpms.h"
#include "../../src/client/event_queue.h"
#include "../../src/client/output.h"
#include "../../src/client/registry.h"
#include "../../src/client/xdgoutput.h"
#include "../../src/server/display.h"
#include "../../src/server/dpms_interface.h"
#include "../../src/server/output_interface.h"
#include "../../src/server/xdgoutput_interface.h"

// Wayland

class TestXdgOutput : public QObject
{
    Q_OBJECT
public:
    explicit TestXdgOutput(QObject *parent = nullptr);
private Q_SLOTS:
    void init();
    void cleanup();
    void testChanges();

private:
    KWayland::Server::Display *m_display;
    KWayland::Server::OutputInterface *m_serverOutput;
    KWayland::Server::XdgOutputManagerInterface *m_serverXdgOutputManager;
    KWayland::Server::XdgOutputInterface *m_serverXdgOutput;
    KWayland::Client::ConnectionThread *m_connection;
    KWayland::Client::EventQueue *m_queue;
    QThread *m_thread;
};

static const QString s_socketName = QStringLiteral("kwin-test-xdg-output-0");

TestXdgOutput::TestXdgOutput(QObject *parent)
    : QObject(parent)
    , m_display(nullptr)
    , m_serverOutput(nullptr)
    , m_connection(nullptr)
    , m_thread(nullptr)
{
}

void TestXdgOutput::init()
{
    using namespace KWayland::Server;
    delete m_display;
    m_display = new Display(this);
    m_display->setSocketName(s_socketName);
    m_display->start();
    QVERIFY(m_display->isRunning());

    m_serverOutput = m_display->createOutput(this);
    m_serverOutput->addMode(QSize(1920, 1080), OutputInterface::ModeFlags(OutputInterface::ModeFlag::Preferred));
    m_serverOutput->setCurrentMode(QSize(1920, 1080));
    m_serverOutput->create();

    m_serverXdgOutputManager = m_display->createXdgOutputManager(this);
    m_serverXdgOutputManager->create();
    m_serverXdgOutput = m_serverXdgOutputManager->createXdgOutput(m_serverOutput, this);
    m_serverXdgOutput->setLogicalSize(QSize(1280, 720)); // a 1.5 scale factor
    m_serverXdgOutput->setLogicalPosition(QPoint(11, 12)); // not a sensible value for one monitor, but works for this test
    m_serverXdgOutput->setName("testName");
    m_serverXdgOutput->setDescription("testDescription");

    m_serverXdgOutput->done();

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
}

void TestXdgOutput::cleanup()
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

    delete m_serverOutput;
    m_serverOutput = nullptr;

    delete m_display;
    m_display = nullptr;
}

void TestXdgOutput::testChanges()
{
    // verify the server modes
    using namespace KWayland::Server;
    using namespace KWayland::Client;
    KWayland::Client::Registry registry;
    QSignalSpy announced(&registry, &KWayland::Client::Registry::outputAnnounced);
    QSignalSpy xdgOutputAnnounced(&registry, &KWayland::Client::Registry::xdgOutputAnnounced);

    registry.setEventQueue(m_queue);
    registry.create(m_connection->display());
    QVERIFY(registry.isValid());
    registry.setup();
    QVERIFY(announced.wait());
    if (xdgOutputAnnounced.count() != 1) {
        QVERIFY(xdgOutputAnnounced.wait());
    }

    KWayland::Client::Output output;
    QSignalSpy outputChanged(&output, &KWayland::Client::Output::changed);

    output.setup(registry.bindOutput(announced.first().first().value<quint32>(), announced.first().last().value<quint32>()));
    QVERIFY(outputChanged.wait());

    QScopedPointer<KWayland::Client::XdgOutputManager> xdgOutputManager(
        registry.createXdgOutputManager(xdgOutputAnnounced.first().first().value<quint32>(), xdgOutputAnnounced.first().last().value<quint32>(), this));

    QScopedPointer<KWayland::Client::XdgOutput> xdgOutput(xdgOutputManager->getXdgOutput(&output, this));
    QSignalSpy xdgOutputChanged(xdgOutput.data(), &KWayland::Client::XdgOutput::changed);

    // check details are sent on client bind
    QVERIFY(xdgOutputChanged.wait());
    xdgOutputChanged.clear();
    QCOMPARE(xdgOutput->logicalPosition(), QPoint(11, 12));
    QCOMPARE(xdgOutput->logicalSize(), QSize(1280, 720));
    QCOMPARE(xdgOutput->name(), "testName");
    QCOMPARE(xdgOutput->description(), "testDescription");

    // dynamic updates
    m_serverXdgOutput->setLogicalPosition(QPoint(1000, 2000));
    m_serverXdgOutput->setLogicalSize(QSize(100, 200));
    // names cannot dynamically change according to the spec

    m_serverXdgOutput->done();

    QVERIFY(xdgOutputChanged.wait());
    QCOMPARE(xdgOutputChanged.count(), 1);
    QCOMPARE(xdgOutput->logicalPosition(), QPoint(1000, 2000));
    QCOMPARE(xdgOutput->logicalSize(), QSize(100, 200));
}

QTEST_GUILESS_MAIN(TestXdgOutput)
#include "test_xdg_output.moc"
