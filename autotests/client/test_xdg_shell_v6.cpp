    #include "test_xdg_shell.h"
#include <wayland-xdg-shell-v6-client-protocol.h>


class XdgShellTestV6 : public XdgShellTest {
    Q_OBJECT
public:
    XdgShellTestV6() :
        XdgShellTest(KWayland::Server::XdgShellInterfaceVersion::UnstableV6) {}

private Q_SLOTS:
    void testMaxSize();
    void testMinSize();
    void testMultipleRoles();
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

void XdgShellTestV6::testMultipleRoles()
{
    //setting multiple roles on an xdg surface should fail
    QSignalSpy xdgSurfaceCreatedSpy(m_xdgShellInterface, &XdgShellInterface::surfaceCreated);
    QVERIFY(xdgSurfaceCreatedSpy.isValid());

    QScopedPointer<Surface> surface(m_compositor->createSurface());
    //This is testing we work when a client does something stupid
    //we can't use KWayland API here because by design that stops you from doing anything stupid
    auto xdgSurface = zxdg_shell_v6_get_xdg_surface(*m_xdgShell, *surface.data());

    //create a top level
    auto xdgTopLevel1 = zxdg_surface_v6_get_toplevel(xdgSurface);
    QVERIFY(xdgSurfaceCreatedSpy.wait());

    //now try to create another top level for the same xdg surface. It should fail
    auto xdgTopLevel2 = zxdg_surface_v6_get_toplevel(xdgSurface);
    QVERIFY(!xdgSurfaceCreatedSpy.wait(10));

    zxdg_toplevel_v6_destroy(xdgTopLevel1);
    zxdg_toplevel_v6_destroy(xdgTopLevel2);
    zxdg_surface_v6_destroy(xdgSurface);

    //TODO:
    //toplevel then popup
    //popup then toplevel
    //popup then popup
}

QTEST_GUILESS_MAIN(XdgShellTestV6)

#include "test_xdg_shell_v6.moc"

