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
#include "../../src/client/outputdevice.h"
#include "../../src/client/outputconfiguration.h"
#include "../../src/client/outputmanagement.h"
#include "../../src/client/output.h"
#include "../../src/client/registry.h"
#include "../../src/server/display.h"
#include "../../src/server/shell_interface.h"
#include "../../src/server/compositor_interface.h"
#include "../../src/server/outputconfiguration_interface.h"
#include "../../src/server/outputdevice_interface.h"
#include "../../src/server/outputmanagement_interface.h"

// Wayland
#include <wayland-client-protocol.h>

using namespace KWayland::Client;
using namespace KWayland::Server;

class TestWaylandOutputManagement : public QObject
{
    Q_OBJECT
public:
    explicit TestWaylandOutputManagement(QObject *parent = nullptr);
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();


    void testCreate();
    void testOutputDevices();
    void createConfig();
    void testApplied();
    void testFailed();
private:
    void testEnable();
    void testPosition();
    void testScale();
    void testTransform();
    void testMode();

private Q_SLOTS:
    void testMultipleSettings();
    void testConfigFailed();

    void testExampleConfig();

    void testRemoval();

private:


    KWayland::Server::Display *m_display;
    KWayland::Server::OutputConfigurationInterface *m_outputConfigurationInterface;
    KWayland::Server::OutputManagementInterface *m_outputManagementInterface;
    QList<KWayland::Server::OutputDeviceInterface *> m_serverOutputs;


    KWayland::Client::Registry m_registry;
    KWayland::Client::OutputDevice *m_outputDevice;
    KWayland::Client::OutputManagement m_outputManagement;
    KWayland::Client::OutputConfiguration *m_outputConfiguration;
    QList<KWayland::Client::OutputDevice *> m_clientOutputs;
    QList<KWayland::Server::OutputDeviceInterface::Mode> m_modes;

    KWayland::Client::ConnectionThread *m_connection;
    KWayland::Client::EventQueue *m_queue;
    QThread *m_thread;

    QSignalSpy *m_announcedSpy;
    QSignalSpy *m_omSpy;
    QSignalSpy *m_configSpy;
};

static const QString s_socketName = QStringLiteral("kwin-test-wayland-output-0");

TestWaylandOutputManagement::TestWaylandOutputManagement(QObject *parent)
    : QObject(parent)
    , m_display(nullptr)
    , m_outputConfigurationInterface(nullptr)
    , m_outputManagementInterface(nullptr)
    , m_connection(nullptr)
    , m_queue(nullptr)
    , m_thread(nullptr)
    , m_announcedSpy(nullptr)
{
}

void TestWaylandOutputManagement::initTestCase()
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

    auto outputDeviceInterface = m_display->createOutputDevice(this);

    OutputDeviceInterface::Mode m0;
    m0.id = 0;
    m0.size = QSize(800, 600);
    m0.flags = OutputDeviceInterface::ModeFlags(OutputDeviceInterface::ModeFlag::Preferred);
    outputDeviceInterface->addMode(m0);

    OutputDeviceInterface::Mode m1;
    m1.id = 1;
    m1.size = QSize(1024, 768);
    outputDeviceInterface->addMode(m1);

    OutputDeviceInterface::Mode m2;
    m2.id = 2;
    m2.size = QSize(1280, 1024);
    m2.refreshRate = 90000;
    outputDeviceInterface->addMode(m2);

    OutputDeviceInterface::Mode m3;
    m3.id = 3;
    m3.size = QSize(1920, 1080);
    m3.flags = OutputDeviceInterface::ModeFlags();
    m3.refreshRate = 100000;
    outputDeviceInterface->addMode(m3);

    m_modes << m0 << m1 << m2 << m3;

    outputDeviceInterface->setCurrentMode(1);
    outputDeviceInterface->setGlobalPosition(QPoint(0, 1920));
    outputDeviceInterface->create();
    m_serverOutputs << outputDeviceInterface;

    m_outputManagementInterface = m_display->createOutputManagement(this);
    m_outputManagementInterface->create();
    QVERIFY(m_outputManagementInterface->isValid());

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

void TestWaylandOutputManagement::cleanupTestCase()
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

void TestWaylandOutputManagement::testCreate()
{
    m_announcedSpy = new QSignalSpy(&m_registry, &KWayland::Client::Registry::outputManagementAnnounced);
    m_omSpy = new QSignalSpy(&m_registry, &KWayland::Client::Registry::outputDeviceAnnounced);

    QVERIFY(m_announcedSpy->isValid());
    QVERIFY(m_omSpy->isValid());

    m_registry.create(m_connection->display());
    QVERIFY(m_registry.isValid());
    m_registry.setEventQueue(m_queue);
    m_registry.setup();
    wl_display_flush(m_connection->display());

    QVERIFY(m_announcedSpy->wait());
    QCOMPARE(m_announcedSpy->count(), 1);

    m_outputManagement.setup(m_registry.bindOutputManagement(m_announcedSpy->first().first().value<quint32>(), m_announcedSpy->first().last().value<quint32>()));
}

void TestWaylandOutputManagement::testOutputDevices()
{

    //m_display->createOutputDevice()->create();
    //QVERIFY(m_omSpy->wait());
    QCOMPARE(m_omSpy->count(), 1);
    QCOMPARE(m_registry.interfaces(KWayland::Client::Registry::Interface::OutputDevice).count(), m_serverOutputs.count());


    auto output = new KWayland::Client::OutputDevice();
    QVERIFY(!output->isValid());
    QCOMPARE(output->geometry(), QRect());
    QCOMPARE(output->globalPosition(), QPoint());
    QCOMPARE(output->manufacturer(), QString());
    QCOMPARE(output->model(), QString());
    QCOMPARE(output->physicalSize(), QSize());
    QCOMPARE(output->pixelSize(), QSize());
    QCOMPARE(output->refreshRate(), 0);
    QCOMPARE(output->scale(), 1);
    QCOMPARE(output->subPixel(), KWayland::Client::OutputDevice::SubPixel::Unknown);
    QCOMPARE(output->transform(), KWayland::Client::OutputDevice::Transform::Normal);
    QCOMPARE(output->enabled(), OutputDevice::Enablement::Enabled);
    QCOMPARE(output->edid(), QString());

    QSignalSpy outputChanged(output, &KWayland::Client::OutputDevice::changed);
    QVERIFY(outputChanged.isValid());

    output->setup(m_registry.bindOutputDevice(m_omSpy->first().first().value<quint32>(), m_omSpy->first().last().value<quint32>()));
    wl_display_flush(m_connection->display());

    QVERIFY(outputChanged.wait());
    QCOMPARE(output->globalPosition(), QPoint(0, 1920));
    QCOMPARE(output->enabled(), OutputDevice::Enablement::Enabled);

    m_clientOutputs << output;
    m_outputDevice = output;

    QVERIFY(m_outputManagement.isValid());
}

void TestWaylandOutputManagement::testRemoval()
{
    QSignalSpy outputManagementRemovedSpy(&m_registry,&KWayland::Client::Registry::outputManagementRemoved);
    QVERIFY(outputManagementRemovedSpy.isValid());

    delete m_outputManagementInterface;
    QVERIFY(outputManagementRemovedSpy.wait());
    QCOMPARE(outputManagementRemovedSpy.first().first(), m_announcedSpy->first().first());
    QVERIFY(!m_registry.hasInterface(KWayland::Client::Registry::Interface::OutputManagement));
    QVERIFY(m_registry.interfaces(KWayland::Client::Registry::Interface::OutputManagement).isEmpty());
}

void TestWaylandOutputManagement::createConfig()
{
    m_configSpy = new QSignalSpy(m_outputManagementInterface, &KWayland::Server::OutputManagementInterface::configurationCreated);
    connect(m_outputManagementInterface, &KWayland::Server::OutputManagementInterface::configurationCreated,
            [this] (KWayland::Server::OutputConfigurationInterface *config) {
                //qDebug() << "set output config" << config;
                m_outputConfigurationInterface = config;
            });
    QVERIFY(m_configSpy->isValid());

    m_outputConfiguration = m_outputManagement.createConfiguration();
    QVERIFY(m_outputConfiguration->isValid());
    QVERIFY(m_outputConfigurationInterface == nullptr);

    // make sure the server side emits the signal that a config has been created
    QVERIFY(m_configSpy->wait(200));
    QVERIFY(m_outputConfigurationInterface != nullptr);
}

void TestWaylandOutputManagement::testApplied()
{
    QVERIFY(m_outputConfiguration->isValid());
    QSignalSpy appliedSpy(m_outputConfiguration, &KWayland::Client::OutputConfiguration::applied);

    m_outputConfiguration->apply();
    // At this point, we fake the compositor and just
    // tell the server to emit the applied signal
    m_outputConfigurationInterface->setApplied();

    QVERIFY(appliedSpy.wait(200));
}

void TestWaylandOutputManagement::testFailed()
{
    QVERIFY(m_outputConfiguration->isValid());
    QSignalSpy failedSpy(m_outputConfiguration, &KWayland::Client::OutputConfiguration::failed);

    m_outputConfiguration->apply();
    // At this point, we fake the compositor and just
    // tell the server to emit the applied signal
    m_outputConfigurationInterface->setFailed();

    QVERIFY(failedSpy.wait(200));
}

void TestWaylandOutputManagement::testEnable()
{
    delete m_outputConfigurationInterface;
    m_outputConfigurationInterface = nullptr;
    createConfig();
    auto config = m_outputConfiguration;
    QVERIFY(config->isValid());

    KWayland::Client::OutputDevice *output = m_clientOutputs.first();
    QCOMPARE(output->enabled(), OutputDevice::Enablement::Enabled);

//     QSignalSpy pendingChangesSpy(m_serverOutputs.first(), &KWayland::Server::OutputDeviceInterface::pendingChangesChanged);
    QSignalSpy enabledChanged(output, &KWayland::Client::OutputDevice::enabledChanged);
    QVERIFY(enabledChanged.isValid());

//     config->setEnabled(output, OutputDevice::Enablement::Disabled);
//
//     QVERIFY(pendingChangesSpy.wait(200));

    QCOMPARE(enabledChanged.count(), 0);
    //QVERIFY(config->hasPendingChanges(m_serverOutputs.first()));
    //qDebug() << "Applying";
//     m_serverOutputs.first()->applyPendingChanges();
//     QVERIFY(!m_serverOutputs.first()->hasPendingChanges());
//
//     QVERIFY(enabledChanged.wait(200));
//     QCOMPARE(output->enabled(), OutputDevice::Enablement::Disabled);
//
//     m_serverOutputs.first()->setEnabled(OutputDeviceInterface::Enablement::Enabled);
//
//     QVERIFY(enabledChanged.wait(200));
//     QCOMPARE(output->enabled(), OutputDevice::Enablement::Enabled);

    // The following is never applied, but set back to its original value
    // as to make sure changes are correctly undone.
    config->setEnabled(output, OutputDevice::Enablement::Disabled);
    //QVERIFY(pendingChangesSpy.wait(200));
    //QVERIFY(m_serverOutputs.first()->hasPendingChanges());
    //qDebug() << "Setting again to disabled";
    config->apply();
    //config->setEnabled(output, OutputDevice::Enablement::Enabled);
    //QVERIFY(pendingChangesSpy.wait(200));
    QVERIFY(!m_serverOutputs.first()->hasPendingChanges());
}


void TestWaylandOutputManagement::testPosition()
{
    delete m_outputConfigurationInterface;
    m_outputConfigurationInterface = nullptr;
    createConfig();
    auto config = m_outputConfiguration;
    QVERIFY(config->isValid());

    KWayland::Client::OutputDevice *output = m_clientOutputs.first();
    QPoint pos = QPoint(0, 1920);
    QPoint pos2 = QPoint(500, 600);
    QCOMPARE(output->globalPosition(), pos);

    QSignalSpy pendingChangesSpy(m_serverOutputs.first(), &KWayland::Server::OutputDeviceInterface::pendingChangesChanged);
    QSignalSpy enabledChanged(output, &KWayland::Client::OutputDevice::changed);
    QVERIFY(enabledChanged.isValid());

    config->setPosition(output, QPoint(500, 600));

    QVERIFY(pendingChangesSpy.wait(200));
    // No changed signal should be fired, yet
    QCOMPARE(enabledChanged.count(), 0);
    QVERIFY(m_serverOutputs.first()->hasPendingChanges());

    m_serverOutputs.first()->applyPendingChanges();
    QVERIFY(!m_serverOutputs.first()->hasPendingChanges());

    QVERIFY(enabledChanged.wait(200));
    QCOMPARE(output->globalPosition(), QPoint(500, 600));

    m_serverOutputs.first()->setGlobalPosition(pos);

    QVERIFY(enabledChanged.wait(200));
    QCOMPARE(output->globalPosition(), pos);

    // The following is never applied, but set back to its original value
    // as to make sure changes are correctly undone.
    config->setPosition(output, pos2);
    QVERIFY(pendingChangesSpy.wait(200));
    QVERIFY(m_serverOutputs.first()->hasPendingChanges());

    config->setPosition(output, pos);
    QVERIFY(pendingChangesSpy.wait(200));
    QVERIFY(!m_serverOutputs.first()->hasPendingChanges());
}

void TestWaylandOutputManagement::testScale()
{
    delete m_outputConfigurationInterface;
    m_outputConfigurationInterface = nullptr;
    createConfig();
    auto config = m_outputConfiguration;
    QVERIFY(config->isValid());

    KWayland::Client::OutputDevice *output = m_clientOutputs.first();
    QCOMPARE(output->currentMode().id, 1);

    QSignalSpy pendingChangesSpy(m_serverOutputs.first(), &KWayland::Server::OutputDeviceInterface::pendingChangesChanged);
    QSignalSpy scaledSpy(output, &KWayland::Client::OutputDevice::changed);
    QVERIFY(scaledSpy.isValid());

    config->setScale(output, 2);

    QVERIFY(pendingChangesSpy.wait(200));

    QCOMPARE(scaledSpy.count(), 0);
    QVERIFY(m_serverOutputs.first()->hasPendingChanges());
    //qDebug() << "Applying";
    m_serverOutputs.first()->applyPendingChanges();
    QVERIFY(!m_serverOutputs.first()->hasPendingChanges());

    QVERIFY(scaledSpy.wait(200));
    QCOMPARE(output->scale(), 2);

    m_serverOutputs.first()->setScale(1);

    QVERIFY(scaledSpy.wait(200));
    QCOMPARE(output->scale(), 1);

    // The following is never applied, but set back to its original value
    // as to make sure changes are correctly undone.
    config->setScale(output, 1337);
    QVERIFY(pendingChangesSpy.wait(200));
    QVERIFY(m_serverOutputs.first()->hasPendingChanges());
    //qDebug() << "Setting again to disabled";

    config->setScale(output, 1);
    QVERIFY(pendingChangesSpy.wait(200));
    QVERIFY(!m_serverOutputs.first()->hasPendingChanges());
    m_outputConfiguration->setScale(output, 0);
    m_serverOutputs.first()->applyPendingChanges();
    QVERIFY(!scaledSpy.wait(200));
    QVERIFY(!m_serverOutputs.first()->hasPendingChanges());
}

void TestWaylandOutputManagement::testMode()
{
    delete m_outputConfigurationInterface;
    m_outputConfigurationInterface = nullptr;
    createConfig();
    auto config = m_outputConfiguration;
    QVERIFY(config->isValid());

    KWayland::Client::OutputDevice *output = m_clientOutputs.first();
    QCOMPARE(output->currentMode().id, 1);

    QSignalSpy pendingChangesSpy(m_serverOutputs.first(), &KWayland::Server::OutputDeviceInterface::pendingChangesChanged);
    QSignalSpy modeChanged(output, &KWayland::Client::OutputDevice::changed);
    QVERIFY(modeChanged.isValid());

    config->setMode(output, 0);

    QVERIFY(pendingChangesSpy.wait(200));

    QCOMPARE(modeChanged.count(), 0);
    QVERIFY(m_serverOutputs.first()->hasPendingChanges());
    //qDebug() << "Applying";
    m_serverOutputs.first()->applyPendingChanges();
    QVERIFY(!m_serverOutputs.first()->hasPendingChanges());

    QVERIFY(modeChanged.wait(200));
    QCOMPARE(output->currentMode().id, 0);

    m_serverOutputs.first()->setCurrentMode(1);

    QVERIFY(modeChanged.wait(200));
    QCOMPARE(output->currentMode().id, 1);

    // The following is never applied, but set back to its original value
    // as to make sure changes are correctly undone.
    config->setMode(output, 0);
    QVERIFY(pendingChangesSpy.wait(200));
    QVERIFY(m_serverOutputs.first()->hasPendingChanges());
    //qDebug() << "Setting again to disabled";

    config->setMode(output, 1);
    QVERIFY(pendingChangesSpy.wait(200));
    QVERIFY(!m_serverOutputs.first()->hasPendingChanges());
    m_outputConfiguration->setMode(output, -1);
    m_serverOutputs.first()->applyPendingChanges();
    QVERIFY(!modeChanged.wait(200));
    QVERIFY(!m_serverOutputs.first()->hasPendingChanges());
}

void TestWaylandOutputManagement::testTransform()
{
    delete m_outputConfigurationInterface;
    m_outputConfigurationInterface = nullptr;
    createConfig();
    auto config = m_outputConfiguration;
    QVERIFY(config->isValid());

    auto t1 = OutputDevice::Transform::Normal;
    auto ts1 = OutputDeviceInterface::Transform::Normal;
    auto t2 = OutputDevice::Transform::Rotated90;

    KWayland::Client::OutputDevice *output = m_clientOutputs.first();
    QCOMPARE(output->transform(), t1);

    QSignalSpy pendingChangesSpy(m_serverOutputs.first(), &KWayland::Server::OutputDeviceInterface::pendingChangesChanged);
    QSignalSpy transformChanged(output, &KWayland::Client::OutputDevice::changed);
    QVERIFY(transformChanged.isValid());

    config->setTransform(output, t2);

    QVERIFY(pendingChangesSpy.wait(200));

    QCOMPARE(transformChanged.count(), 0);
    QVERIFY(m_serverOutputs.first()->hasPendingChanges());
    //qDebug() << "Applying";
    m_serverOutputs.first()->applyPendingChanges();
    QVERIFY(!m_serverOutputs.first()->hasPendingChanges());

    QVERIFY(transformChanged.wait(200));
    QCOMPARE(output->transform(), t2);

    m_serverOutputs.first()->setTransform(ts1);

    QVERIFY(transformChanged.wait(200));
    QCOMPARE(output->transform(), t1);

    // The following is never applied, but set back to its original value
    // as to make sure changes are correctly undone.
    config->setTransform(output, t2);
    QVERIFY(pendingChangesSpy.wait(200));
    QVERIFY(m_serverOutputs.first()->hasPendingChanges());
    //qDebug() << "Setting again to disabled";

    config->setTransform(output, t1);
    QVERIFY(pendingChangesSpy.wait(200));
    QVERIFY(!m_serverOutputs.first()->hasPendingChanges());
}

void TestWaylandOutputManagement::testMultipleSettings()
{
    delete m_outputConfigurationInterface;
    m_outputConfigurationInterface = nullptr;
    createConfig();
    auto config = m_outputConfiguration;
    QVERIFY(config->isValid());

    KWayland::Client::OutputDevice *output = m_clientOutputs.first();
    QSignalSpy outputChangedSpy(output, &KWayland::Client::OutputDevice::changed);
    QSignalSpy serverApplySpy(m_outputConfigurationInterface, &OutputConfigurationInterface::applyRequested);
    QVERIFY(serverApplySpy.isValid());

    config->setMode(output, m_modes.first().id);
    config->setTransform(output, OutputDevice::Transform::Rotated90);
    config->setPosition(output, QPoint(13, 37));
    config->setScale(output, 2);
    config->setEnabled(output, OutputDevice::Enablement::Disabled);
    config->apply();

    QVERIFY(serverApplySpy.wait(200));
    QCOMPARE(serverApplySpy.count(), 1);

    m_outputConfigurationInterface->setApplied();

    QSignalSpy configAppliedSpy(config, &OutputConfiguration::applied);
    QVERIFY(configAppliedSpy.isValid());
    QVERIFY(configAppliedSpy.wait(200));
    QCOMPARE(configAppliedSpy.count(), 1);
    QCOMPARE(outputChangedSpy.count(), 5);

    config->setMode(output, m_modes.at(1).id);
    config->setTransform(output, OutputDevice::Transform::Normal);
    config->setPosition(output, QPoint(0, 1920));
    config->setScale(output, 1);
    config->setEnabled(output, OutputDevice::Enablement::Enabled);
    config->apply();

    QVERIFY(serverApplySpy.wait(200));
    QCOMPARE(serverApplySpy.count(), 2);

    m_outputConfigurationInterface->setApplied();

    QVERIFY(configAppliedSpy.wait(200));
    QCOMPARE(configAppliedSpy.count(), 2);
    QCOMPARE(outputChangedSpy.count(), 10);

}

void TestWaylandOutputManagement::testConfigFailed()
{
    auto config = m_outputConfiguration;
    auto s_o = m_serverOutputs.first();
    KWayland::Client::OutputDevice *output = m_clientOutputs.first();

    QVERIFY(config->isValid());
    QVERIFY(s_o->isValid());
    QVERIFY(output->isValid());

    QSignalSpy serverApplySpy(m_outputConfigurationInterface, &OutputConfigurationInterface::applyRequested);
    QVERIFY(serverApplySpy.isValid());
    QSignalSpy pendingChangesSpy(s_o, &KWayland::Server::OutputDeviceInterface::pendingChangesChanged);
    QVERIFY(pendingChangesSpy.isValid());
    QSignalSpy outputChangedSpy(output, &KWayland::Client::OutputDevice::changed);
    QVERIFY(outputChangedSpy.isValid());
    QSignalSpy configAppliedSpy(config, &OutputConfiguration::applied);
    QVERIFY(configAppliedSpy.isValid());
    QSignalSpy configFailedSpy(config, &KWayland::Client::OutputConfiguration::failed);
    QVERIFY(configFailedSpy.isValid());

    QVERIFY(!m_serverOutputs.first()->hasPendingChanges());
    config->setMode(output, m_modes.last().id);
    config->setTransform(output, OutputDevice::Transform::Normal);
    config->setPosition(output, QPoint(-1, -1));

    // Check if changes have arrived
    // Note that it isn't necessary to wait here in order to proceed to config->apply()
    QVERIFY(pendingChangesSpy.wait(200));
    QCOMPARE(pendingChangesSpy.count(), 2); // Transform::Normal was already set
    QVERIFY(m_serverOutputs.first()->hasPendingChanges());

    config->apply();
    QVERIFY(serverApplySpy.wait(200));
    QVERIFY(m_serverOutputs.first()->hasPendingChanges());

    // Artificialy make the server fail to apply the settings
    m_outputConfigurationInterface->setFailed();
    // Make sure the applied signal never comes, and that failed has been received
    QVERIFY(!configAppliedSpy.wait(200));
    QCOMPARE(configFailedSpy.count(), 1);
    QCOMPARE(configAppliedSpy.count(), 0);
    QVERIFY(!m_serverOutputs.first()->hasPendingChanges());
    QCOMPARE(outputChangedSpy.count(), 0);
}

void TestWaylandOutputManagement::testExampleConfig()
{
    //auto config = m_outputManagement.createConfiguration();

    delete m_outputConfigurationInterface;
    m_outputConfigurationInterface = nullptr;
    createConfig();

    auto config = m_outputConfiguration;
    KWayland::Client::OutputDevice *output = m_clientOutputs.first();

    config->setMode(output, m_clientOutputs.first()->modes().last().id);
    config->setTransform(output, OutputDevice::Transform::Normal);
    config->setPosition(output, QPoint(-1, -1));

    connect(config, &OutputConfiguration::applied, []() {
        qDebug() << "Configuration applied!";
    });
    connect(config, &OutputConfiguration::failed, []() {
        qDebug() << "Configuration failed!";
    });

    config->apply();

    QSignalSpy configAppliedSpy(config, &OutputConfiguration::applied);
    m_outputConfigurationInterface->setApplied();
    QVERIFY(configAppliedSpy.isValid());
    QVERIFY(configAppliedSpy.wait(200));
    QVERIFY(!configAppliedSpy.wait(200));
}

QTEST_GUILESS_MAIN(TestWaylandOutputManagement)
#include "test_wayland_outputmanagement.moc"
