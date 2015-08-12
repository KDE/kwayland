/********************************************************************
Copyright 2015  Martin Gräßlin <mgraesslin@kde.org>

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
#include "qtwaylandintegrationtest.h"
// KWin::Wayland
#include <../src/client/buffer.h>
#include <../src/client/compositor.h>
#include <../src/client/connection_thread.h>
#include <../src/client/pointer.h>
#include <../src/client/registry.h>
#include <../src/client/shell.h>
#include <../src/client/shm_pool.h>
#include <../src/client/surface.h>
// Qt
#include <QAbstractEventDispatcher>
#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QTimer>

#include <linux/input.h>

using namespace KWayland::Client;

static Qt::GlobalColor s_colors[] = {
    Qt::white,
    Qt::red,
    Qt::green,
    Qt::blue,
    Qt::black
};
static int s_colorIndex = 0;

WaylandClientTest::WaylandClientTest(QObject *parent)
    : QObject(parent)
    , m_connectionThreadObject(ConnectionThread::fromApplication(this))
    , m_compositor(Compositor::fromApplication(this))
    , m_surface(nullptr)
    , m_shm(nullptr)
    , m_shellSurface(nullptr)
    , m_timer(new QTimer(this))
{
    init();
}

WaylandClientTest::~WaylandClientTest() = default;

void WaylandClientTest::init()
{
    connect(m_timer, &QTimer::timeout, this,
        [this]() {
            s_colorIndex = (s_colorIndex + 1) % 5;
            render();
        }
    );
    m_timer->setInterval(1000);
    m_timer->start();

    m_surface = m_compositor->createSurface(this);
    Registry *registry = new Registry(this);
    setupRegistry(registry);
}

void WaylandClientTest::setupRegistry(Registry *registry)
{
    connect(registry, &Registry::shellAnnounced, this,
        [this, registry](quint32 name) {
            Shell *shell = registry->createShell(name, 1, this);
            m_shellSurface = shell->createSurface(m_surface, m_surface);
            connect(m_shellSurface, &ShellSurface::sizeChanged, this, static_cast<void(WaylandClientTest::*)(const QSize&)>(&WaylandClientTest::render));
            render(QSize(200, 200));
        }
    );
    connect(registry, &Registry::shmAnnounced, this,
        [this, registry](quint32 name) {
            m_shm = registry->createShmPool(name, 1, this);
        }
    );
    registry->create(m_connectionThreadObject->display());
    registry->setup();
}

void WaylandClientTest::render(const QSize &size)
{
    m_currentSize = size;
    render();
}

void WaylandClientTest::render()
{
    if (!m_shm || !m_surface || !m_surface->isValid() || !m_currentSize.isValid()) {
        return;
    }
    auto buffer = m_shm->getBuffer(m_currentSize, m_currentSize.width() * 4).toStrongRef();
    buffer->setUsed(true);
    QImage image(buffer->address(), m_currentSize.width(), m_currentSize.height(), QImage::Format_ARGB32_Premultiplied);
    image.fill(s_colors[s_colorIndex]);

    m_surface->attachBuffer(*buffer);
    m_surface->damage(QRect(QPoint(0, 0), m_currentSize));
    m_surface->commit(Surface::CommitFlag::None);
    buffer->setUsed(false);
}

int main(int argc, char **argv)
{
    qputenv("QT_QPA_PLATFORM", "wayland");
    QGuiApplication app(argc, argv);

    new WaylandClientTest(&app);

    return app.exec();
}
