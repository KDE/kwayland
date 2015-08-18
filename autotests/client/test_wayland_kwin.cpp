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
#include "../../src/client/kwin_screen_management.h"
#include "../../src/client/output.h"
#include "../../src/client/registry.h"
#include "../../src/server/display.h"
#include "../../src/server/shell_interface.h"
#include "../../src/server/compositor_interface.h"
#include "../../src/server/output_interface.h"
#include "../../src/server/kwin_screen_management_interface.h"

// Wayland
#include <wayland-client-protocol.h>

class TestWaylandKWin : public QObject
{
    Q_OBJECT
public:
    explicit TestWaylandKWin(QObject *parent = nullptr);
private Q_SLOTS:
    void init();
    void cleanup();

    void testGetOutputs();

private:
    KWayland::Server::Display *m_display;
    KWayland::Server::KWinScreenManagementInterface *m_kwinInterface;
    KWayland::Server::OutputInterface *m_serverOutput;
    //     KWayland::Server::KWin *m_kwin;
    KWayland::Client::ConnectionThread *m_connection;
    KWayland::Client::EventQueue *m_queue;
    QThread *m_thread;
};

static const QString s_socketName = QStringLiteral("kwin-test-wayland-output-0");

TestWaylandKWin::TestWaylandKWin(QObject *parent)
    : QObject(parent)
    , m_display(nullptr)
    , m_connection(nullptr)
    , m_thread(nullptr)
{
}

void TestWaylandKWin::init()
{
    using namespace KWayland::Server;
    delete m_display;
    m_display = new Display(this);
    m_display->setSocketName(s_socketName);
    m_display->start();
    QVERIFY(m_display->isRunning());

    auto shell = m_display->createShell(this);
    shell->create();
    auto comp = m_display->createCompositor(this);
    comp->create();

    m_serverOutput = m_display->createOutput(this);
    m_serverOutput->addMode(QSize(800, 600), OutputInterface::ModeFlags(OutputInterface::ModeFlag::Preferred));
    m_serverOutput->addMode(QSize(1024, 768));
    m_serverOutput->addMode(QSize(1280, 1024), OutputInterface::ModeFlags(), 90000);
    m_serverOutput->setCurrentMode(QSize(1024, 768));
    m_serverOutput->create();

    qDebug() << "creating m_kwinInterface";
    m_kwinInterface = m_display->createKWinScreenManagement(this);
    m_kwinInterface->create();
    QVERIFY(m_kwinInterface->isValid());

    m_kwinInterface->addDisabledOutput("", "DiscoScreen", "HDMI1");
    m_kwinInterface->addDisabledOutput("INVALID_EDID_INFO", "LargeMonitor", "DisplayPort-0");

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
}

void TestWaylandKWin::cleanup()
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

void TestWaylandKWin::testGetOutputs()
{
     /*-*/
     //auto kwin = m_display->createKWinScreenManagement();
     //kwin->getOutputs();

     KWayland::Client::Registry registry;
     //QSignalSpy announced(&registry, SIGNAL(outputAnnounced(quint32,quint32)));
     //QSignalSpy announced(&registry, SIGNAL(interfacesAnnounced()));
     QSignalSpy announced(&registry, SIGNAL(kwinScreenManagementAnnounced(quint32,quint32)));
     registry.create(m_connection->display());
     QVERIFY(registry.isValid());
     registry.setup();
     wl_display_flush(m_connection->display());
     QVERIFY(announced.wait(1000));


     KWayland::Client::KWinScreenManagement *kwin = registry.createKWinScreenManagement(announced.first().first().value<quint32>(), 1, &registry);
     QVERIFY(kwin->isValid());

     QSignalSpy oSpy(kwin, SIGNAL(disabledOutputAdded(const QString&, const QString&, const QString&)));
     QSignalSpy rSpy(kwin, SIGNAL(disabledOutputRemoved(const QString&, const QString&)));
     QSignalSpy doneSpy(kwin, SIGNAL(done()));

     QVERIFY(doneSpy.wait(200));
     QCOMPARE(oSpy.count(), 2);

     m_kwinInterface->removeDisabledOutput("DiscoScreen", "HDMI1");
     QVERIFY(rSpy.wait(1000));
     QCOMPARE(rSpy.count(), 1);
     //m_kwinInterface->addDisabledOutput("INVALID_EDID_INFO", "LargeMonitor", "DisplayPort-0");


}



QTEST_GUILESS_MAIN(TestWaylandKWin)
#include "test_wayland_kwin.moc"
