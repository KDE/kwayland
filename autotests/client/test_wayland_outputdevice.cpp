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
// KWin
#include "../../src/client/connection_thread.h"
#include "../../src/client/event_queue.h"
#include "../../src/client/outputdevice.h"
#include "../../src/client/registry.h"
#include "../../src/server/display.h"
#include "../../src/server/outputdevice_interface.h"
// Wayland
#include <wayland-client-protocol.h>

class TestWaylandOutputDevice : public QObject
{
    Q_OBJECT
public:
    explicit TestWaylandOutputDevice(QObject *parent = nullptr);
private Q_SLOTS:
    void init();
    void cleanup();

    void testRegistry();
    void testModeChanges();
    void testScaleChange();

    void testSubPixel_data();
    void testSubPixel();

    void testTransform_data();
    void testTransform();

    void testEnabled();
    void testEdid();

private:
    KWayland::Server::Display *m_display;
    KWayland::Server::OutputDeviceInterface *m_serverOutputDevice;
    KWayland::Server::OutputDeviceInterface::Edid m_edid;
    KWayland::Client::ConnectionThread *m_connection;
    KWayland::Client::EventQueue *m_queue;
    QThread *m_thread;

};

static const QString s_socketName = QStringLiteral("kwin-test-wayland-output-0");

TestWaylandOutputDevice::TestWaylandOutputDevice(QObject *parent)
    : QObject(parent)
    , m_display(nullptr)
    , m_serverOutputDevice(nullptr)
    , m_connection(nullptr)
    , m_thread(nullptr)
{
}

void TestWaylandOutputDevice::init()
{
    using namespace KWayland::Server;
    delete m_display;
    m_display = new Display(this);
    m_display->setSocketName(s_socketName);
    m_display->start();
    QVERIFY(m_display->isRunning());

    m_serverOutputDevice = m_display->createOutputDevice(this);
    m_serverOutputDevice->addMode(QSize(800, 600), OutputDeviceInterface::ModeFlags(OutputDeviceInterface::ModeFlag::Preferred));
    m_serverOutputDevice->addMode(QSize(1024, 768));
    m_serverOutputDevice->addMode(QSize(1280, 1024), OutputDeviceInterface::ModeFlags(), 90000);
    m_serverOutputDevice->setCurrentMode(QSize(1024, 768));

    m_edid.eisaId = "0xDEADBEEF";
    m_edid.monitorName = "DisplayPort-0";
    m_edid.serialNumber = "2222222";
    m_edid.physicalSize = QSize(1600, 900); // in mm
    m_edid.data = "AP///////wAQrBbwTExLQQ4WAQOANCB46h7Frk80sSYOUFSlSwCBgKlA0QBxTwEBAQEBAQEBKDyAoHCwI0AwIDYABkQhAAAaAAAA/wBGNTI1TTI0NUFLTEwKAAAA/ABERUxMIFUyNDEwCiAgAAAA/QA4TB5REQAKICAgICAgAToCAynxUJAFBAMCBxYBHxITFCAVEQYjCQcHZwMMABAAOC2DAQAA4wUDAQI6gBhxOC1AWCxFAAZEIQAAHgEdgBhxHBYgWCwlAAZEIQAAngEdAHJR0B4gbihVAAZEIQAAHowK0Iog4C0QED6WAAZEIQAAGAAAAAAAAAAAAAAAAAAAPg==";
    m_serverOutputDevice->setEdid(m_edid);

    m_serverOutputDevice->create();

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

void TestWaylandOutputDevice::cleanup()
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

    delete m_serverOutputDevice;
    m_serverOutputDevice = nullptr;

    delete m_display;
    m_display = nullptr;
}

void TestWaylandOutputDevice::testRegistry()
{
    m_serverOutputDevice->setGlobalPosition(QPoint(100, 50));
    m_serverOutputDevice->setPhysicalSize(QSize(200, 100));

    KWayland::Client::Registry registry;
    QSignalSpy announced(&registry, SIGNAL(outputDeviceAnnounced(quint32,quint32)));
    registry.create(m_connection->display());
    QVERIFY(registry.isValid());
    registry.setup();
    wl_display_flush(m_connection->display());
    QVERIFY(announced.wait());

    KWayland::Client::OutputDevice output;
    QVERIFY(!output.isValid());
    QCOMPARE(output.geometry(), QRect());
    QCOMPARE(output.globalPosition(), QPoint());
    QCOMPARE(output.manufacturer(), QString());
    QCOMPARE(output.model(), QString());
    QCOMPARE(output.physicalSize(), QSize());
    QCOMPARE(output.pixelSize(), QSize());
    QCOMPARE(output.refreshRate(), 0);
    QCOMPARE(output.scale(), 1);
    QCOMPARE(output.subPixel(), KWayland::Client::OutputDevice::SubPixel::Unknown);
    QCOMPARE(output.transform(), KWayland::Client::OutputDevice::Transform::Normal);
    QCOMPARE(output.enabled(), true);
    QCOMPARE(output.edid()->eisaId, QString());

    QSignalSpy outputChanged(&output, SIGNAL(changed()));
    QVERIFY(outputChanged.isValid());

    output.setup(registry.bindOutputDevice(announced.first().first().value<quint32>(), announced.first().last().value<quint32>()));
    wl_display_flush(m_connection->display());

    QVERIFY(outputChanged.wait());

    QCOMPARE(output.geometry(), QRect(100, 50, 1024, 768));
    QCOMPARE(output.globalPosition(), QPoint(100, 50));
    QCOMPARE(output.manufacturer(), QStringLiteral("org.kde.kwin"));
    QCOMPARE(output.model(), QStringLiteral("none"));
    QCOMPARE(output.physicalSize(), QSize(200, 100));
    QCOMPARE(output.pixelSize(), QSize(1024, 768));
    QCOMPARE(output.refreshRate(), 60000);
    QCOMPARE(output.scale(), 1);
    // for xwayland output it's unknown
    QCOMPARE(output.subPixel(), KWayland::Client::OutputDevice::SubPixel::Unknown);
    // for xwayland transform is normal
    QCOMPARE(output.transform(), KWayland::Client::OutputDevice::Transform::Normal);

    QCOMPARE(output.edid()->eisaId, QStringLiteral("0xDEADBEEF"));
    QCOMPARE(output.enabled(), true);

}

void TestWaylandOutputDevice::testModeChanges()
{
    using namespace KWayland::Client;
    KWayland::Client::Registry registry;
    QSignalSpy announced(&registry, SIGNAL(outputDeviceAnnounced(quint32,quint32)));
    registry.setEventQueue(m_queue);
    registry.create(m_connection->display());
    QVERIFY(registry.isValid());
    registry.setup();
    wl_display_flush(m_connection->display());
    QVERIFY(announced.wait());

    KWayland::Client::OutputDevice output;
    QSignalSpy outputChanged(&output, SIGNAL(changed()));
    QVERIFY(outputChanged.isValid());
    QSignalSpy modeAddedSpy(&output, SIGNAL(modeAdded(KWayland::Client::OutputDevice::Mode)));
    QVERIFY(modeAddedSpy.isValid());
    output.setup(registry.bindOutputDevice(announced.first().first().value<quint32>(), announced.first().last().value<quint32>()));
    wl_display_flush(m_connection->display());
    QVERIFY(outputChanged.wait());
    QCOMPARE(modeAddedSpy.count(), 3);
    QCOMPARE(modeAddedSpy.at(0).first().value<OutputDevice::Mode>().size, QSize(800, 600));
    QCOMPARE(modeAddedSpy.at(0).first().value<OutputDevice::Mode>().refreshRate, 60000);
    QCOMPARE(modeAddedSpy.at(0).first().value<OutputDevice::Mode>().flags, OutputDevice::Mode::Flags(OutputDevice::Mode::Flag::Preferred));
    QCOMPARE(modeAddedSpy.at(0).first().value<OutputDevice::Mode>().output, QPointer<OutputDevice>(&output));
    QCOMPARE(modeAddedSpy.at(1).first().value<OutputDevice::Mode>().size, QSize(1280, 1024));
    QCOMPARE(modeAddedSpy.at(1).first().value<OutputDevice::Mode>().refreshRate, 90000);
    QCOMPARE(modeAddedSpy.at(1).first().value<OutputDevice::Mode>().flags, OutputDevice::Mode::Flags(OutputDevice::Mode::Flag::None));
    QCOMPARE(modeAddedSpy.at(1).first().value<OutputDevice::Mode>().output, QPointer<OutputDevice>(&output));
    QCOMPARE(modeAddedSpy.at(2).first().value<OutputDevice::Mode>().size, QSize(1024, 768));
    QCOMPARE(modeAddedSpy.at(2).first().value<OutputDevice::Mode>().refreshRate, 60000);
    QCOMPARE(modeAddedSpy.at(2).first().value<OutputDevice::Mode>().flags, OutputDevice::Mode::Flags(OutputDevice::Mode::Flag::Current));
    QCOMPARE(modeAddedSpy.at(2).first().value<OutputDevice::Mode>().output, QPointer<OutputDevice>(&output));
    const QList<OutputDevice::Mode> &modes = output.modes();
    QCOMPARE(modes.size(), 3);
    QCOMPARE(modes.at(0), modeAddedSpy.at(0).first().value<OutputDevice::Mode>());
    QCOMPARE(modes.at(1), modeAddedSpy.at(1).first().value<OutputDevice::Mode>());
    QCOMPARE(modes.at(2), modeAddedSpy.at(2).first().value<OutputDevice::Mode>());

    QCOMPARE(output.pixelSize(), QSize(1024, 768));

    // change the current mode
    outputChanged.clear();
    QSignalSpy modeChangedSpy(&output, &KWayland::Client::OutputDevice::modeChanged);
    QVERIFY(modeChangedSpy.isValid());
    m_serverOutputDevice->setCurrentMode(QSize(800, 600));
    QVERIFY(modeChangedSpy.wait());
    if (modeChangedSpy.size() == 1) {
        QVERIFY(modeChangedSpy.wait());
    }
    QCOMPARE(modeChangedSpy.size(), 2);
    // the one which lost the current flag
    QCOMPARE(modeChangedSpy.first().first().value<OutputDevice::Mode>().size, QSize(1024, 768));
    QCOMPARE(modeChangedSpy.first().first().value<OutputDevice::Mode>().refreshRate, 60000);
    QCOMPARE(modeChangedSpy.first().first().value<OutputDevice::Mode>().flags, OutputDevice::Mode::Flags());
    // the one which got the current flag
    QCOMPARE(modeChangedSpy.last().first().value<OutputDevice::Mode>().size, QSize(800, 600));
    QCOMPARE(modeChangedSpy.last().first().value<OutputDevice::Mode>().refreshRate, 60000);
    QCOMPARE(modeChangedSpy.last().first().value<OutputDevice::Mode>().flags, OutputDevice::Mode::Flags(OutputDevice::Mode::Flag::Current | OutputDevice::Mode::Flag::Preferred));
    QVERIFY(!outputChanged.isEmpty());
    QCOMPARE(output.pixelSize(), QSize(800, 600));
    const QList<OutputDevice::Mode> &modes2 = output.modes();
    QCOMPARE(modes2.at(0).size, QSize(1280, 1024));
    QCOMPARE(modes2.at(0).refreshRate, 90000);
    QCOMPARE(modes2.at(0).flags, OutputDevice::Mode::Flag::None);
    QCOMPARE(modes2.at(1).size, QSize(1024, 768));
    QCOMPARE(modes2.at(1).refreshRate, 60000);
    QCOMPARE(modes2.at(1).flags, OutputDevice::Mode::Flag::None);
    QCOMPARE(modes2.at(2).size, QSize(800, 600));
    QCOMPARE(modes2.at(2).refreshRate, 60000);
    QCOMPARE(modes2.at(2).flags, OutputDevice::Mode::Flag::Current | OutputDevice::Mode::Flag::Preferred);

    // change once more
    outputChanged.clear();
    modeChangedSpy.clear();
    m_serverOutputDevice->setCurrentMode(QSize(1280, 1024), 90000);
    QVERIFY(modeChangedSpy.wait());
    if (modeChangedSpy.size() == 1) {
        QVERIFY(modeChangedSpy.wait());
    }
    QCOMPARE(modeChangedSpy.size(), 2);
    // the one which lost the current flag
    QCOMPARE(modeChangedSpy.first().first().value<OutputDevice::Mode>().size, QSize(800, 600));
    QCOMPARE(modeChangedSpy.first().first().value<OutputDevice::Mode>().refreshRate, 60000);
    QCOMPARE(modeChangedSpy.first().first().value<OutputDevice::Mode>().flags, OutputDevice::Mode::Flags(OutputDevice::Mode::Flag::Preferred));
    // the one which got the current flag
    QCOMPARE(modeChangedSpy.last().first().value<OutputDevice::Mode>().size, QSize(1280, 1024));
    QCOMPARE(modeChangedSpy.last().first().value<OutputDevice::Mode>().refreshRate, 90000);
    QCOMPARE(modeChangedSpy.last().first().value<OutputDevice::Mode>().flags, OutputDevice::Mode::Flags(OutputDevice::Mode::Flag::Current));
    QVERIFY(!outputChanged.isEmpty());
    QCOMPARE(output.pixelSize(), QSize(1280, 1024));
}

void TestWaylandOutputDevice::testScaleChange()
{
    KWayland::Client::Registry registry;
    QSignalSpy announced(&registry, SIGNAL(outputDeviceAnnounced(quint32,quint32)));
    registry.create(m_connection->display());
    QVERIFY(registry.isValid());
    registry.setup();
    wl_display_flush(m_connection->display());
    QVERIFY(announced.wait());

    KWayland::Client::OutputDevice output;
    QSignalSpy outputChanged(&output, SIGNAL(changed()));
    QVERIFY(outputChanged.isValid());
    output.setup(registry.bindOutputDevice(announced.first().first().value<quint32>(), announced.first().last().value<quint32>()));
    wl_display_flush(m_connection->display());
    QVERIFY(outputChanged.wait());
    QCOMPARE(output.scale(), 1);

    // change the scale
    outputChanged.clear();
    m_serverOutputDevice->setScale(2);
    QVERIFY(outputChanged.wait());
    QCOMPARE(output.scale(), 2);

    // change once more
    outputChanged.clear();
    m_serverOutputDevice->setScale(4);
    QVERIFY(outputChanged.wait());
    QCOMPARE(output.scale(), 4);
}

void TestWaylandOutputDevice::testSubPixel_data()
{
    using namespace KWayland::Client;
    using namespace KWayland::Server;
    QTest::addColumn<KWayland::Client::OutputDevice::SubPixel>("expected");
    QTest::addColumn<KWayland::Server::OutputDeviceInterface::SubPixel>("actual");

    QTest::newRow("none") << OutputDevice::SubPixel::None << OutputDeviceInterface::SubPixel::None;
    QTest::newRow("horizontal/rgb") << OutputDevice::SubPixel::HorizontalRGB << OutputDeviceInterface::SubPixel::HorizontalRGB;
    QTest::newRow("horizontal/bgr") << OutputDevice::SubPixel::HorizontalBGR << OutputDeviceInterface::SubPixel::HorizontalBGR;
    QTest::newRow("vertical/rgb") << OutputDevice::SubPixel::VerticalRGB << OutputDeviceInterface::SubPixel::VerticalRGB;
    QTest::newRow("vertical/bgr") << OutputDevice::SubPixel::VerticalBGR << OutputDeviceInterface::SubPixel::VerticalBGR;
}

void TestWaylandOutputDevice::testSubPixel()
{
    using namespace KWayland::Client;
    using namespace KWayland::Server;
    QFETCH(OutputDeviceInterface::SubPixel, actual);
    m_serverOutputDevice->setSubPixel(actual);

    KWayland::Client::Registry registry;
    QSignalSpy announced(&registry, SIGNAL(outputDeviceAnnounced(quint32,quint32)));
    registry.create(m_connection->display());
    QVERIFY(registry.isValid());
    registry.setup();
    wl_display_flush(m_connection->display());
    QVERIFY(announced.wait());

    KWayland::Client::OutputDevice output;
    QSignalSpy outputChanged(&output, SIGNAL(changed()));
    QVERIFY(outputChanged.isValid());
    output.setup(registry.bindOutputDevice(announced.first().first().value<quint32>(), announced.first().last().value<quint32>()));
    wl_display_flush(m_connection->display());
    if (outputChanged.isEmpty()) {
        QVERIFY(outputChanged.wait());
    }

    QTEST(output.subPixel(), "expected");

    // change back to unknown
    outputChanged.clear();
    m_serverOutputDevice->setSubPixel(OutputDeviceInterface::SubPixel::Unknown);
    if (outputChanged.isEmpty()) {
        QVERIFY(outputChanged.wait());
    }
    QCOMPARE(output.subPixel(), OutputDevice::SubPixel::Unknown);
}

void TestWaylandOutputDevice::testTransform_data()
{
    using namespace KWayland::Client;
    using namespace KWayland::Server;
    QTest::addColumn<KWayland::Client::OutputDevice::Transform>("expected");
    QTest::addColumn<KWayland::Server::OutputDeviceInterface::Transform>("actual");

    QTest::newRow("90")          << OutputDevice::Transform::Rotated90  << OutputDeviceInterface::Transform::Rotated90;
    QTest::newRow("180")         << OutputDevice::Transform::Rotated180 << OutputDeviceInterface::Transform::Rotated180;
    QTest::newRow("270")         << OutputDevice::Transform::Rotated270 << OutputDeviceInterface::Transform::Rotated270;
    QTest::newRow("Flipped")     << OutputDevice::Transform::Flipped    << OutputDeviceInterface::Transform::Flipped;
    QTest::newRow("Flipped 90")  << OutputDevice::Transform::Flipped90  << OutputDeviceInterface::Transform::Flipped90;
    QTest::newRow("Flipped 180") << OutputDevice::Transform::Flipped180 << OutputDeviceInterface::Transform::Flipped180;
    QTest::newRow("Flipped 280") << OutputDevice::Transform::Flipped270 << OutputDeviceInterface::Transform::Flipped270;
}

void TestWaylandOutputDevice::testTransform()
{
    using namespace KWayland::Client;
    using namespace KWayland::Server;
    QFETCH(OutputDeviceInterface::Transform, actual);
    m_serverOutputDevice->setTransform(actual);

    KWayland::Client::Registry registry;
    QSignalSpy announced(&registry, SIGNAL(outputDeviceAnnounced(quint32,quint32)));
    registry.create(m_connection->display());
    QVERIFY(registry.isValid());
    registry.setup();
    wl_display_flush(m_connection->display());
    QVERIFY(announced.wait());

    KWayland::Client::OutputDevice *output = registry.createOutputDevice(announced.first().first().value<quint32>(), announced.first().last().value<quint32>(), &registry);
    QSignalSpy outputChanged(output, SIGNAL(changed()));
    QVERIFY(outputChanged.isValid());
    wl_display_flush(m_connection->display());
    if (outputChanged.isEmpty()) {
        QVERIFY(outputChanged.wait());
    }

    QTEST(output->transform(), "expected");

    // change back to normal
    outputChanged.clear();
    m_serverOutputDevice->setTransform(OutputDeviceInterface::Transform::Normal);
    if (outputChanged.isEmpty()) {
        QVERIFY(outputChanged.wait());
    }
    QCOMPARE(output->transform(), OutputDevice::Transform::Normal);
}

void TestWaylandOutputDevice::testEnabled()
{
    KWayland::Client::Registry registry;
    QSignalSpy announced(&registry, SIGNAL(outputDeviceAnnounced(quint32,quint32)));
    registry.create(m_connection->display());
    QVERIFY(registry.isValid());
    registry.setup();
    wl_display_flush(m_connection->display());
    QVERIFY(announced.wait());

    KWayland::Client::OutputDevice output;
    QSignalSpy outputChanged(&output, SIGNAL(changed()));
    QVERIFY(outputChanged.isValid());
    output.setup(registry.bindOutputDevice(announced.first().first().value<quint32>(), announced.first().last().value<quint32>()));
    wl_display_flush(m_connection->display());
    QVERIFY(outputChanged.wait());

    QCOMPARE(output.enabled(), true);

    QSignalSpy enabledChanged(&output, SIGNAL(changed()));
    QVERIFY(enabledChanged.isValid());

    m_serverOutputDevice->setEnabled(false);
    //wl_display_flush(m_connection->display());

    QVERIFY(enabledChanged.wait(200));
    QCOMPARE(output.enabled(), false);

    m_serverOutputDevice->setEnabled(true);

    QVERIFY(enabledChanged.wait(200));
    QCOMPARE(output.enabled(), true);


}

void TestWaylandOutputDevice::testEdid()
{
    KWayland::Client::Registry registry;
    QSignalSpy announced(&registry, SIGNAL(outputDeviceAnnounced(quint32,quint32)));
    registry.create(m_connection->display());
    QVERIFY(registry.isValid());
    registry.setup();
    wl_display_flush(m_connection->display());
    QVERIFY(announced.wait());

    KWayland::Client::OutputDevice output;

    QCOMPARE(output.edid()->eisaId, QString());
    QCOMPARE(output.edid()->monitorName, QString());
    QCOMPARE(output.edid()->serialNumber, QString());
    QCOMPARE(output.edid()->physicalSize, QSize());
    QCOMPARE(output.edid()->data, QString());

    QSignalSpy outputChanged(&output, SIGNAL(changed()));
    QVERIFY(outputChanged.isValid());
    output.setup(registry.bindOutputDevice(announced.first().first().value<quint32>(), announced.first().last().value<quint32>()));
    wl_display_flush(m_connection->display());
    QVERIFY(outputChanged.wait());

    QCOMPARE(output.edid()->eisaId, m_edid.eisaId);
    QCOMPARE(output.edid()->monitorName, m_edid.monitorName);
    QCOMPARE(output.edid()->serialNumber, m_edid.serialNumber);
    QCOMPARE(output.edid()->physicalSize, m_edid.physicalSize);
    QCOMPARE(output.edid()->data, m_edid.data);
}


QTEST_GUILESS_MAIN(TestWaylandOutputDevice)
#include "test_wayland_outputdevice.moc"
