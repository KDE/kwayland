#include "test_xdg_shell.h"

class XdgShellTestV5 : public XdgShellTest {
    Q_OBJECT
public:
    XdgShellTestV5() :
        XdgShellTest(KWayland::Server::XdgShellInterfaceVersion::UnstableV5) {}
};

QTEST_GUILESS_MAIN(XdgShellTestV5)

#include "test_xdg_shell_v5.moc"

