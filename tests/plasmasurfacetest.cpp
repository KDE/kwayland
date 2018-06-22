/********************************************************************
Copyright 2016  Martin Gräßlin <mgraesslin@kde.org>

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
#include "../src/client/compositor.h"
#include "../src/client/connection_thread.h"
#include "../src/client/event_queue.h"
#include "../src/client/registry.h"
#include "../src/client/shell.h"
#include "../src/client/shm_pool.h"
#include "../src/client/surface.h"
#include "../src/client/plasmashell.h"
// Qt
#include <QCommandLineParser>
#include <QGuiApplication>
#include <QImage>
#include <QThread>

using namespace KWayland::Client;

class PlasmaSurfaceTest : public QObject
{
    Q_OBJECT
public:
    explicit PlasmaSurfaceTest(QObject *parent = nullptr);
    virtual ~PlasmaSurfaceTest();

    void init();

    void setRole(PlasmaShellSurface::Role role) {
        m_role = role;
    }
    void setSkipTaskbar(bool set) {
        m_skipTaskbar = set;
    }

    void setSkipSwitcher(bool set) {
        m_skipSwitcher = set;
    }

private:
    void setupRegistry(Registry *registry);
    void render();
    QThread *m_connectionThread;
    ConnectionThread *m_connectionThreadObject;
    EventQueue *m_eventQueue = nullptr;
    Compositor *m_compositor = nullptr;
    Shell *m_shell = nullptr;
    ShellSurface *m_shellSurface = nullptr;
    ShmPool *m_shm = nullptr;
    Surface *m_surface = nullptr;
    PlasmaShell *m_plasmaShell = nullptr;
    PlasmaShellSurface *m_plasmaShellSurface = nullptr;
    PlasmaShellSurface::Role m_role = PlasmaShellSurface::Role::Normal;
    bool m_skipTaskbar = false;
    bool m_skipSwitcher = false;
};

PlasmaSurfaceTest::PlasmaSurfaceTest(QObject *parent)
    : QObject(parent)
    , m_connectionThread(new QThread(this))
    , m_connectionThreadObject(new ConnectionThread())
{
}

PlasmaSurfaceTest::~PlasmaSurfaceTest()
{
    m_connectionThread->quit();
    m_connectionThread->wait();
    m_connectionThreadObject->deleteLater();
}

void PlasmaSurfaceTest::init()
{
    connect(m_connectionThreadObject, &ConnectionThread::connected, this,
        [this] {
            m_eventQueue = new EventQueue(this);
            m_eventQueue->setup(m_connectionThreadObject);

            Registry *registry = new Registry(this);
            setupRegistry(registry);
        },
        Qt::QueuedConnection
    );
    m_connectionThreadObject->moveToThread(m_connectionThread);
    m_connectionThread->start();

    m_connectionThreadObject->initConnection();
}

void PlasmaSurfaceTest::setupRegistry(Registry *registry)
{
    connect(registry, &Registry::compositorAnnounced, this,
        [this, registry](quint32 name, quint32 version) {
            m_compositor = registry->createCompositor(name, version, this);
        }
    );
    connect(registry, &Registry::shellAnnounced, this,
        [this, registry](quint32 name, quint32 version) {
            m_shell = registry->createShell(name, version, this);
        }
    );
    connect(registry, &Registry::shmAnnounced, this,
        [this, registry](quint32 name, quint32 version) {
            m_shm = registry->createShmPool(name, version, this);
        }
    );
    connect(registry, &Registry::plasmaShellAnnounced, this,
        [this, registry](quint32 name, quint32 version) {
            m_plasmaShell = registry->createPlasmaShell(name, version, this);
            m_plasmaShell->setEventQueue(m_eventQueue);
        }
    );
    connect(registry, &Registry::interfacesAnnounced, this,
        [this] {
            Q_ASSERT(m_compositor);
            Q_ASSERT(m_shell);
            Q_ASSERT(m_shm);
            Q_ASSERT(m_plasmaShell);
            m_surface = m_compositor->createSurface(this);
            Q_ASSERT(m_surface);
            m_shellSurface = m_shell->createSurface(m_surface, this);
            Q_ASSERT(m_shellSurface);
            m_shellSurface->setToplevel();
            connect(m_shellSurface, &ShellSurface::sizeChanged, this, &PlasmaSurfaceTest::render);
            m_plasmaShellSurface = m_plasmaShell->createSurface(m_surface, this);
            Q_ASSERT(m_plasmaShellSurface);
            m_plasmaShellSurface->setSkipTaskbar(m_skipTaskbar);
            m_plasmaShellSurface->setSkipSwitcher(m_skipSwitcher);
            m_plasmaShellSurface->setRole(m_role);
            render();
        }
    );
    registry->setEventQueue(m_eventQueue);
    registry->create(m_connectionThreadObject);
    registry->setup();
}

void PlasmaSurfaceTest::render()
{
    const QSize &size = m_shellSurface->size().isValid() ? m_shellSurface->size() : QSize(300, 200);
    auto buffer = m_shm->getBuffer(size, size.width() * 4).toStrongRef();
    buffer->setUsed(true);
    QImage image(buffer->address(), size.width(), size.height(), QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(255, 255, 255, 128));

    m_surface->attachBuffer(*buffer);
    m_surface->damage(QRect(QPoint(0, 0), size));
    m_surface->commit(Surface::CommitFlag::None);
    buffer->setUsed(false);
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addHelpOption();
    QCommandLineOption notificationOption(QStringLiteral("notification"));
    parser.addOption(notificationOption);
    QCommandLineOption panelOption(QStringLiteral("panel"));
    parser.addOption(panelOption);
    QCommandLineOption desktopOption(QStringLiteral("desktop"));
    parser.addOption(desktopOption);
    QCommandLineOption osdOption(QStringLiteral("osd"));
    parser.addOption(osdOption);
    QCommandLineOption tooltipOption(QStringLiteral("tooltip"));
    parser.addOption(tooltipOption);
    QCommandLineOption skipTaskbarOption(QStringLiteral("skipTaskbar"));
    parser.addOption(skipTaskbarOption);
    parser.process(app);
    QCommandLineOption skipSwitcherOption(QStringLiteral("skipSwitcher"));
    parser.addOption(skipSwitcherOption);
    parser.process(app);

    PlasmaSurfaceTest client;

    if (parser.isSet(notificationOption)) {
        client.setRole(PlasmaShellSurface::Role::Notification);
    } else if (parser.isSet(panelOption)) {
        client.setRole(PlasmaShellSurface::Role::Panel);
    } else if (parser.isSet(desktopOption)) {
        client.setRole(PlasmaShellSurface::Role::Desktop);
    } else if (parser.isSet(osdOption)) {
        client.setRole(PlasmaShellSurface::Role::OnScreenDisplay);
    } else if (parser.isSet(tooltipOption)) {
        client.setRole(PlasmaShellSurface::Role::ToolTip);
    }
    client.setSkipTaskbar(parser.isSet(skipTaskbarOption));
    client.setSkipSwitcher(parser.isSet(skipSwitcherOption));

    client.init();

    return app.exec();
}

#include "plasmasurfacetest.moc"
