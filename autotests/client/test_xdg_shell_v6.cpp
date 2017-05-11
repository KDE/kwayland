#include "test_xdg_shell.cpp"

class XdgShellTestV6 : public XdgShellTest {
    Q_OBJECT
public:
    XdgShellTestV6() :
        XdgShellTest(KWayland::Server::XdgShellInterfaceVersion::UnstableV6) {}

private Q_SLOTS:
    void testMaxSize();
    void testMinSize();
};

void XdgShellTestV6::testMaxSize()
{
    qRegisterMetaType<OutputInterface*>();
    // this test verifies changing the window maxSize
    QSignalSpy xdgSurfaceCreatedSpy(m_xdgShellInterface, &XdgShellInterface::surfaceCreated);
    QVERIFY(xdgSurfaceCreatedSpy.isValid());
    QScopedPointer<Surface> surface(m_compositor->createSurface());
    QScopedPointer<XdgShellSurface> xdgSurface(m_xdgShell->createSurface(surface.data()));
    QVERIFY(xdgSurfaceCreatedSpy.wait());
    auto serverXdgSurface = xdgSurfaceCreatedSpy.first().first().value<XdgShellSurfaceInterface*>();
    QVERIFY(serverXdgSurface);

    QSignalSpy maxSizeSpy(serverXdgSurface, &XdgShellSurfaceInterface::maxSizeChanged);
    QVERIFY(maxSizeSpy.isValid());

    xdgSurface->setMaxSize(QSize(100, 100));
    QVERIFY(maxSizeSpy.wait());
    QCOMPARE(maxSizeSpy.count(), 1);
    QCOMPARE(maxSizeSpy.last().at(0).value<QSize>(), QSize(100,100));

    xdgSurface->setMaxSize(QSize(200, 200));
    QVERIFY(maxSizeSpy.wait());
    QCOMPARE(maxSizeSpy.count(), 2);
    QCOMPARE(maxSizeSpy.last().at(0).value<QSize>(), QSize(200,200));
}

void XdgShellTestV6::testMinSize()
{
    qRegisterMetaType<OutputInterface*>();
    // this test verifies changing the window minSize
    QSignalSpy xdgSurfaceCreatedSpy(m_xdgShellInterface, &XdgShellInterface::surfaceCreated);
    QVERIFY(xdgSurfaceCreatedSpy.isValid());
    QScopedPointer<Surface> surface(m_compositor->createSurface());
    QScopedPointer<XdgShellSurface> xdgSurface(m_xdgShell->createSurface(surface.data()));
    QVERIFY(xdgSurfaceCreatedSpy.wait());
    auto serverXdgSurface = xdgSurfaceCreatedSpy.first().first().value<XdgShellSurfaceInterface*>();
    QVERIFY(serverXdgSurface);

    QSignalSpy minSizeSpy(serverXdgSurface, &XdgShellSurfaceInterface::minSizeChanged);
    QVERIFY(minSizeSpy.isValid());

    xdgSurface->setMinSize(QSize(200, 200));
    QVERIFY(minSizeSpy.wait());
    QCOMPARE(minSizeSpy.count(), 1);
    QCOMPARE(minSizeSpy.last().at(0).value<QSize>(), QSize(200,200));

    xdgSurface->setMinSize(QSize(100, 100));
    QVERIFY(minSizeSpy.wait());
    QCOMPARE(minSizeSpy.count(), 2);
    QCOMPARE(minSizeSpy.last().at(0).value<QSize>(), QSize(100,100));
}

QTEST_GUILESS_MAIN(XdgShellTestV6)

#include "test_xdg_shell_v6.moc"

