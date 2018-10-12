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
#include "../src/server/buffer_interface.h"
#include "../src/server/compositor_interface.h"
#include "../src/server/datadevicemanager_interface.h"
#include "../src/server/display.h"
#include "../src/server/keyboard_interface.h"
#include "../src/server/output_interface.h"
#include "../src/server/pointer_interface.h"
#include "../src/server/seat_interface.h"
#include "../src/server/shell_interface.h"
#include "../src/server/xdgshell_interface.h"
#include "../src/server/subcompositor_interface.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDateTime>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QWidget>
#include <QtConcurrent>

#include <unistd.h>
#include <iostream>

static int startXServer()
{
    const QByteArray process = QByteArrayLiteral("Xwayland");
    int pipeFds[2];
    if (pipe(pipeFds) != 0) {
        std::cerr << "FATAL ERROR failed to create pipe to start X Server "
                  << process.constData()
                  << std::endl;
        exit(1);
    }

    pid_t pid = fork();
    if (pid == 0) {
        // child process - should be turned into Xwayland
        // writes to pipe, closes read side
        close(pipeFds[0]);
        char fdbuf[16];
        sprintf(fdbuf, "%d", pipeFds[1]);
        execlp(process.constData(), process.constData(), "-displayfd", fdbuf, "-rootless", (char *)nullptr);
        close(pipeFds[1]);
        exit(20);
    }
    // parent process - this is the wayland server
    // reads from pipe, closes write side
    close(pipeFds[1]);
    return pipeFds[0];
}

static void readDisplayFromPipe(int pipe)
{
    QFile readPipe;
    if (!readPipe.open(pipe, QIODevice::ReadOnly)) {
        std::cerr << "FATAL ERROR failed to open pipe to start X Server XWayland" << std::endl;
        exit(1);
    }
    QByteArray displayNumber = readPipe.readLine();

    displayNumber.prepend(QByteArray(":"));
    displayNumber.remove(displayNumber.size() -1, 1);
    std::cout << "X-Server started on display " << displayNumber.constData() << std::endl;

    setenv("DISPLAY", displayNumber.constData(), true);

    // close our pipe
    close(pipe);
}

class CompositorWindow : public QWidget
{
    Q_OBJECT
public:
    explicit CompositorWindow(QWidget *parent = nullptr);
    virtual ~CompositorWindow();

    void surfaceCreated(KWayland::Server::XdgShellSurfaceInterface *surface);

    void setSeat(const QPointer<KWayland::Server::SeatInterface> &seat);

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    void updateFocus();
    QList<KWayland::Server::XdgShellSurfaceInterface*> m_stackingOrder;
    QPointer<KWayland::Server::SeatInterface> m_seat;
};

CompositorWindow::CompositorWindow(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
}

CompositorWindow::~CompositorWindow() = default;

void CompositorWindow::surfaceCreated(KWayland::Server::XdgShellSurfaceInterface  *surface)
{
    using namespace KWayland::Server;
    surface->configure(XdgShellSurfaceInterface::States());

    m_stackingOrder << surface;

    connect(surface->surface(), &SurfaceInterface::subSurfaceTreeChanged, this, static_cast<void (CompositorWindow::*)()>(&CompositorWindow::update));
    connect(surface->surface(), &SurfaceInterface::damaged, this, static_cast<void (CompositorWindow::*)()>(&CompositorWindow::update));
    connect(surface, &ShellSurfaceInterface::destroyed, this,
        [surface, this] {
            m_stackingOrder.removeAll(surface);
            updateFocus();
            update();
        }
    );
    updateFocus();
}

void CompositorWindow::updateFocus()
{
    using namespace KWayland::Server;
    if (!m_seat || m_stackingOrder.isEmpty()) {
        return;
    }
    auto it = std::find_if(m_stackingOrder.constBegin(), m_stackingOrder.constEnd(),
        [](XdgShellSurfaceInterface *s) {
            return s->surface()->buffer() != nullptr;
        }
    );
    if (it == m_stackingOrder.constEnd()) {
        return;
    }
    m_seat->setFocusedPointerSurface((*it)->surface());
    m_seat->setFocusedKeyboardSurface((*it)->surface());
}

void CompositorWindow::setSeat(const QPointer< KWayland::Server::SeatInterface > &seat)
{
    m_seat = seat;
}

void CompositorWindow::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter p(this);
    for (auto s : m_stackingOrder) {
        if (auto *b = s->surface()->buffer()) {
            p.drawImage(QPoint(0, 0), b->data());
            s->surface()->frameRendered(QDateTime::currentMSecsSinceEpoch());
        }
        for (auto child : s->surface()->childSubSurfaces()) {
            auto surface = child->surface();
            if (auto *b = surface->buffer()) {
                p.drawImage(child->position(), b->data());
                surface->frameRendered(QDateTime::currentMSecsSinceEpoch());
            }
        }
    }
}

void CompositorWindow::keyPressEvent(QKeyEvent *event)
{
    QWidget::keyPressEvent(event);
    if (!m_seat) {
        return;
    }
    if (!m_seat->focusedKeyboardSurface()) {
        updateFocus();
    }
    m_seat->setTimestamp(event->timestamp());
    m_seat->keyPressed(event->nativeScanCode() - 8);
}

void CompositorWindow::keyReleaseEvent(QKeyEvent *event)
{
    QWidget::keyReleaseEvent(event);
    if (!m_seat) {
        return;
    }
    m_seat->setTimestamp(event->timestamp());
    m_seat->keyReleased(event->nativeScanCode() - 8);
}

void CompositorWindow::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
    if (!m_seat->focusedPointerSurface()) {
        updateFocus();
    }
    m_seat->setTimestamp(event->timestamp());
    m_seat->setPointerPos(event->localPos().toPoint());
}

void CompositorWindow::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    if (!m_seat->focusedPointerSurface()) {
        if (!m_stackingOrder.isEmpty()) {
            m_seat->setFocusedPointerSurface(m_stackingOrder.last()->surface());
        }
    }
    m_seat->setTimestamp(event->timestamp());
    m_seat->pointerButtonPressed(event->button());
}

void CompositorWindow::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
    m_seat->setTimestamp(event->timestamp());
    m_seat->pointerButtonReleased(event->button());
}

void CompositorWindow::wheelEvent(QWheelEvent *event)
{
    QWidget::wheelEvent(event);
    m_seat->setTimestamp(event->timestamp());
    const QPoint &angle = event->angleDelta() / (8 * 15);
    if (angle.x() != 0) {
        m_seat->pointerAxis(Qt::Horizontal, angle.x());
    }
    if (angle.y() != 0) {
        m_seat->pointerAxis(Qt::Vertical, angle.y());
    }
}

int main(int argc, char **argv)
{
    using namespace KWayland::Server;
    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption xwaylandOption(QStringList{QStringLiteral("x"), QStringLiteral("xwayland")},
                                      QStringLiteral("Start a rootless Xwayland server"));
    parser.addOption(xwaylandOption);
    parser.process(app);

    Display display;
    display.start();
    DataDeviceManagerInterface *ddm = display.createDataDeviceManager();
    ddm->create();
    CompositorInterface *compositor = display.createCompositor(&display);
    compositor->create();
    XdgShellInterface *xdgShell = display.createXdgShell(KWayland::Server::XdgShellInterfaceVersion::UnstableV6, &display);
    xdgShell->create();

    SubCompositorInterface *subcomp = display.createSubCompositor(&display);
    subcomp->create();
    display.createShm();
    OutputInterface *output = display.createOutput(&display);
    output->setPhysicalSize(QSize(269, 202));
    const QSize windowSize(1024, 768);
    output->addMode(windowSize);
    output->create();
    SeatInterface *seat = display.createSeat();
    seat->setHasKeyboard(true);
    seat->setHasPointer(true);
    seat->setName(QStringLiteral("testSeat0"));
    seat->create();

    CompositorWindow compositorWindow;
    compositorWindow.setSeat(seat);
    compositorWindow.setMinimumSize(windowSize);
    compositorWindow.setMaximumSize(windowSize);
    compositorWindow.setGeometry(QRect(QPoint(0, 0), windowSize));
    compositorWindow.show();
    QObject::connect(xdgShell, &KWayland::Server::XdgShellInterface::surfaceCreated, &compositorWindow, &CompositorWindow::surfaceCreated);

    // start XWayland
    if (parser.isSet(xwaylandOption)) {
        // starts XWayland by forking and opening a pipe
        const int pipe = startXServer();
        if (pipe == -1) {
            exit(1);
        }

        QtConcurrent::run([pipe] { readDisplayFromPipe(pipe); });
    }

    return app.exec();
}

#include "renderingservertest.moc"
