#include "test_xdg_shell.cpp"

class XdgShellTestV6 : public XdgShellTest {
    Q_OBJECT
public:
    XdgShellTestV6() :
        XdgShellTest(KWayland::Server::XdgShellInterfaceVersion::UnstableV6) {}
};

QTEST_GUILESS_MAIN(XdgShellTestV6)

#include "test_xdg_shell_v6.moc"

