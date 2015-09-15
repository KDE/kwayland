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
#include "registry.h"
#include "compositor.h"
#include "connection_thread.h"
#include "datadevicemanager.h"
#include "dpms.h"
#include "event_queue.h"
#include "fakeinput.h"
#include "fullscreen_shell.h"
#include "idle.h"
#include "logging_p.h"
#include "outputconfiguration.h"
#include "outputmanagement.h"
#include "outputdevice.h"
#include "output.h"
#include "plasmashell.h"
#include "plasmawindowmanagement.h"
#include "seat.h"
#include "shadow.h"
#include "blur.h"
#include "contrast.h"
#include "slide.h"
#include "shell.h"
#include "shm_pool.h"
#include "subcompositor.h"
#include "wayland_pointer_p.h"
// Qt
#include <QDebug>
// wayland
#include <wayland-client-protocol.h>
#include <wayland-fullscreen-shell-client-protocol.h>
#include <wayland-plasma-shell-client-protocol.h>
#include <wayland-plasma-window-management-client-protocol.h>
#include <wayland-idle-client-protocol.h>
#include <wayland-fake-input-client-protocol.h>
#include <wayland-shadow-client-protocol.h>
#include <wayland-output-management-client-protocol.h>
#include <wayland-org_kde_kwin_outputdevice-client-protocol.h>
#include <wayland-blur-client-protocol.h>
#include <wayland-contrast-client-protocol.h>
#include <wayland-slide-client-protocol.h>
#include <wayland-dpms-client-protocol.h>

/*****
 * How to add another interface:
 * * define a new enum value in Registry::Interface
 * * define the bind<InterfaceName> method
 * * define the create<InterfaceName> method
 * * define the <interfaceName>Announced signal
 * * define the <interfaceName>Removed signal
 * * add a block to s_interfaces
 * * add the BIND macro for the new bind<InterfaceName>
 * * add the CREATE macro for the new create<InterfaceName>
 * * extend registry unit test to verify that it works
 ****/

namespace KWayland
{
namespace Client
{
struct SuppertedInterfaceData {
    quint32 maxVersion;
    QByteArray name;
    const wl_interface *interface;
    void (Registry::*announcedSignal)(quint32, quint32);
    void (Registry::*removedSignal)(quint32);
};
static const QMap<Registry::Interface, SuppertedInterfaceData> s_interfaces = {
    {Registry::Interface::Compositor, {
        3,
        QByteArrayLiteral("wl_compositor"),
        &wl_compositor_interface,
        &Registry::compositorAnnounced,
        &Registry::compositorRemoved
    }},
    {Registry::Interface::DataDeviceManager, {
        1,
        QByteArrayLiteral("wl_data_device_manager"),
        &wl_data_device_manager_interface,
        &Registry::dataDeviceManagerAnnounced,
        &Registry::dataDeviceManagerRemoved
    }},
    {Registry::Interface::Output, {
        2,
        QByteArrayLiteral("wl_output"),
        &wl_output_interface,
        &Registry::outputAnnounced,
        &Registry::outputRemoved
    }},
    {Registry::Interface::Shm, {
        1,
        QByteArrayLiteral("wl_shm"),
        &wl_shm_interface,
        &Registry::shmAnnounced,
        &Registry::shmRemoved
    }},
    {Registry::Interface::Seat, {
        4,
        QByteArrayLiteral("wl_seat"),
        &wl_seat_interface,
        &Registry::seatAnnounced,
        &Registry::seatRemoved
    }},
    {Registry::Interface::Shell, {
        1,
        QByteArrayLiteral("wl_shell"),
        &wl_shell_interface,
        &Registry::shellAnnounced,
        &Registry::shellRemoved
    }},
    {Registry::Interface::SubCompositor, {
        1,
        QByteArrayLiteral("wl_subcompositor"),
        &wl_subcompositor_interface,
        &Registry::subCompositorAnnounced,
        &Registry::subCompositorRemoved
    }},
    {Registry::Interface::PlasmaShell, {
        1,
        QByteArrayLiteral("org_kde_plasma_shell"),
        &org_kde_plasma_shell_interface,
        &Registry::plasmaShellAnnounced,
        &Registry::plasmaShellRemoved
    }},
    {Registry::Interface::PlasmaWindowManagement, {
        1,
        QByteArrayLiteral("org_kde_plasma_window_management"),
        &org_kde_plasma_window_management_interface,
        &Registry::plasmaWindowManagementAnnounced,
        &Registry::plasmaWindowManagementRemoved
    }},
    {Registry::Interface::Idle, {
        1,
        QByteArrayLiteral("org_kde_kwin_idle"),
        &org_kde_kwin_idle_interface,
        &Registry::idleAnnounced,
        &Registry::idleRemoved
    }},
    {Registry::Interface::FakeInput, {
        1,
        QByteArrayLiteral("org_kde_kwin_fake_input"),
        &org_kde_kwin_fake_input_interface,
        &Registry::fakeInputAnnounced,
        &Registry::fakeInputRemoved
    }},
    {Registry::Interface::OutputManagement, {
        1,
        QByteArrayLiteral("org_kde_kwin_outputmanagement"),
        &org_kde_kwin_outputmanagement_interface,
        &Registry::outputManagementAnnounced,
        &Registry::outputManagementRemoved
    }},
    {Registry::Interface::OutputDevice, {
        1,
        QByteArrayLiteral("org_kde_kwin_outputdevice"),
        &org_kde_kwin_outputdevice_interface,
        &Registry::outputDeviceAnnounced,
        &Registry::outputDeviceRemoved
    }},
    {Registry::Interface::Shadow, {
        1,
        QByteArrayLiteral("org_kde_kwin_shadow_manager"),
        &org_kde_kwin_shadow_manager_interface,
        &Registry::shadowAnnounced,
        &Registry::shadowRemoved
    }},
    {Registry::Interface::Blur, {
        1,
        QByteArrayLiteral("org_kde_kwin_blur_manager"),
        &org_kde_kwin_blur_manager_interface,
        &Registry::blurAnnounced,
        &Registry::blurRemoved
    }},
    {Registry::Interface::Contrast, {
        1,
        QByteArrayLiteral("org_kde_kwin_contrast_manager"),
        &org_kde_kwin_contrast_manager_interface,
        &Registry::contrastAnnounced,
        &Registry::contrastRemoved
    }},
    {Registry::Interface::Slide, {
        1,
        QByteArrayLiteral("org_kde_kwin_slide_manager"),
        &org_kde_kwin_slide_manager_interface,
        &Registry::slideAnnounced,
        &Registry::slideRemoved
    }},
    {Registry::Interface::FullscreenShell, {
        1,
        QByteArrayLiteral("_wl_fullscreen_shell"),
        &_wl_fullscreen_shell_interface,
        &Registry::fullscreenShellAnnounced,
        &Registry::fullscreenShellRemoved
    }},
    {Registry::Interface::Dpms, {
        1,
        QByteArrayLiteral("org_kde_kwin_dpms_manager"),
        &org_kde_kwin_dpms_manager_interface,
        &Registry::dpmsAnnounced,
        &Registry::dpmsRemoved
    }}
};

static quint32 maxVersion(const Registry::Interface &interface)
{
    auto it = s_interfaces.find(interface);
    if (it != s_interfaces.end()) {
        return it.value().maxVersion;
    }
    return 0;
}

class Registry::Private
{
public:
    Private(Registry *q);
    void setup();
    bool hasInterface(Interface interface) const;
    AnnouncedInterface interface(Interface interface) const;
    QVector<AnnouncedInterface> interfaces(Interface interface) const;
    template <typename T>
    T *bind(Interface interface, uint32_t name, uint32_t version) const;
    template <class T, typename WL>
    T *create(quint32 name, quint32 version, QObject *parent, WL *(Registry::*bindMethod)(uint32_t, uint32_t) const);

    WaylandPointer<wl_registry, wl_registry_destroy> registry;
    static const struct wl_callback_listener s_callbackListener;
    WaylandPointer<wl_callback, wl_callback_destroy> callback;
    EventQueue *queue = nullptr;

private:
    void handleAnnounce(uint32_t name, const char *interface, uint32_t version);
    void handleRemove(uint32_t name);
    void handleGlobalSync();
    static void globalAnnounce(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version);
    static void globalRemove(void *data, struct wl_registry *registry, uint32_t name);
    static void globalSync(void *data, struct wl_callback *callback, uint32_t serial);

    Registry *q;
    struct InterfaceData {
        Interface interface;
        uint32_t name;
        uint32_t version;
    };
    QList<InterfaceData> m_interfaces;
    static const struct wl_registry_listener s_registryListener;
};

Registry::Private::Private(Registry *q)
    : q(q)
{
}

void Registry::Private::setup()
{
    wl_registry_add_listener(registry, &s_registryListener, this);
    wl_callback_add_listener(callback, &s_callbackListener, this);
}

Registry::Registry(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

Registry::~Registry()
{
    release();
}

void Registry::release()
{
    d->registry.release();
    d->callback.release();
}

void Registry::destroy()
{
    d->registry.destroy();
    d->callback.destroy();
}

void Registry::create(wl_display *display)
{
    Q_ASSERT(display);
    Q_ASSERT(!isValid());
    d->registry.setup(wl_display_get_registry(display));
    d->callback.setup(wl_display_sync(display));
    if (d->queue) {
        d->queue->addProxy(d->registry);
        d->queue->addProxy(d->callback);
    }
}

void Registry::create(ConnectionThread *connection)
{
    create(connection->display());
}

void Registry::setup()
{
    Q_ASSERT(isValid());
    d->setup();
}

void Registry::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
    if (!queue) {
        return;
    }
    if (d->registry) {
        d->queue->addProxy(d->registry);
    }
    if (d->callback) {
        d->queue->addProxy(d->callback);
    }
}

EventQueue *Registry::eventQueue()
{
    return d->queue;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const struct wl_registry_listener Registry::Private::s_registryListener = {
    globalAnnounce,
    globalRemove
};

const struct wl_callback_listener Registry::Private::s_callbackListener = {
   globalSync
};
#endif

void Registry::Private::globalAnnounce(void *data, wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
    auto r = reinterpret_cast<Registry::Private*>(data);
    Q_ASSERT(registry == r->registry);
    r->handleAnnounce(name, interface, version);
}

void Registry::Private::globalRemove(void *data, wl_registry *registry, uint32_t name)
{
    auto r = reinterpret_cast<Registry::Private*>(data);
    Q_ASSERT(registry == r->registry);
    r->handleRemove(name);
}

void Registry::Private::globalSync(void* data, wl_callback* callback, uint32_t serial)
{
    Q_UNUSED(serial)
    auto r = reinterpret_cast<Registry::Private*>(data);
    Q_ASSERT(r->callback == callback);
    r->handleGlobalSync();
    r->callback.destroy();
}

void Registry::Private::handleGlobalSync()
{
    emit q->interfacesAnnounced();
}

static Registry::Interface nameToInterface(const char *interface)
{
    for (auto it = s_interfaces.begin(); it != s_interfaces.end(); ++it) {
        if (qstrcmp(interface, it.value().name) == 0) {
            return it.key();
        }
    }
    return Registry::Interface::Unknown;
}

void Registry::Private::handleAnnounce(uint32_t name, const char *interface, uint32_t version)
{
    Interface i = nameToInterface(interface);
    emit q->interfaceAnnounced(QByteArray(interface), name, version);
    if (i == Interface::Unknown) {
        qCDebug(KWAYLAND_CLIENT) << "Unknown interface announced: " << interface << "/" << name << "/" << version;
        return;
    }
    qCDebug(KWAYLAND_CLIENT) << "Wayland Interface: " << interface << "/" << name << "/" << version;
    m_interfaces.append({i, name, version});
    auto it = s_interfaces.constFind(i);
    if (it != s_interfaces.end()) {
        emit (q->*it.value().announcedSignal)(name, version);
    }
}

void Registry::Private::handleRemove(uint32_t name)
{
    auto it = std::find_if(m_interfaces.begin(), m_interfaces.end(),
        [name](const InterfaceData &data) {
            return data.name == name;
        }
    );
    if (it != m_interfaces.end()) {
        InterfaceData data = *(it);
        m_interfaces.erase(it);
        auto sit = s_interfaces.find(data.interface);
        if (sit != s_interfaces.end()) {
            emit (q->*sit.value().removedSignal)(data.name);
        }
    }
    emit q->interfaceRemoved(name);
}

bool Registry::Private::hasInterface(Registry::Interface interface) const
{
    auto it = std::find_if(m_interfaces.begin(), m_interfaces.end(),
        [interface](const InterfaceData &data) {
            return data.interface == interface;
        }
    );
    return it != m_interfaces.end();
}

QVector<Registry::AnnouncedInterface> Registry::Private::interfaces(Interface interface) const
{
    QVector<Registry::AnnouncedInterface> retVal;
    for (auto it = m_interfaces.constBegin(); it != m_interfaces.constEnd(); ++it) {
        const auto &data = *it;
        if (data.interface == interface) {
            retVal << AnnouncedInterface{data.name, data.version};
        }
    }
    return retVal;
}

Registry::AnnouncedInterface Registry::Private::interface(Interface interface) const
{
    const auto all = interfaces(interface);
    if (!all.isEmpty()) {
        return all.last();
    }
    return AnnouncedInterface{0, 0};
}

bool Registry::hasInterface(Registry::Interface interface) const
{
    return d->hasInterface(interface);
}

QVector<Registry::AnnouncedInterface> Registry::interfaces(Interface interface) const
{
    return d->interfaces(interface);
}

Registry::AnnouncedInterface Registry::interface(Interface interface) const
{
    return d->interface(interface);
}

#define BIND2(__NAME__, __INAME__, __WL__) \
__WL__ *Registry::bind##__NAME__(uint32_t name, uint32_t version) const \
{ \
    return d->bind<__WL__>(Interface::__INAME__, name, qMin(maxVersion(Interface::__INAME__), version)); \
}

#define BIND(__NAME__, __WL__) BIND2(__NAME__, __NAME__, __WL__)

BIND(Compositor, wl_compositor)
BIND(Output, wl_output)
BIND(Seat, wl_seat)
BIND(Shell, wl_shell)
BIND(Shm, wl_shm)
BIND(SubCompositor, wl_subcompositor)
BIND(FullscreenShell, _wl_fullscreen_shell)
BIND(DataDeviceManager, wl_data_device_manager)
BIND(PlasmaShell, org_kde_plasma_shell)
BIND(PlasmaWindowManagement, org_kde_plasma_window_management)
BIND(Idle, org_kde_kwin_idle)
BIND(FakeInput, org_kde_kwin_fake_input)
BIND(OutputManagement, org_kde_kwin_outputmanagement)
BIND(OutputDevice, org_kde_kwin_outputdevice)
BIND2(ShadowManager, Shadow, org_kde_kwin_shadow_manager)
BIND2(BlurManager, Blur, org_kde_kwin_blur_manager)
BIND2(ContrastManager, Contrast, org_kde_kwin_contrast_manager)
BIND2(SlideManager, Slide, org_kde_kwin_slide_manager)
BIND2(DpmsManager, Dpms, org_kde_kwin_dpms_manager)

#undef BIND
#undef BIND2

template <class T, typename WL>
T *Registry::Private::create(quint32 name, quint32 version, QObject *parent, WL *(Registry::*bindMethod)(uint32_t, uint32_t) const)
{
    T *t = new T(parent);
    t->setEventQueue(queue);
    t->setup((q->*bindMethod)(name, version));
    QObject::connect(q, &Registry::interfaceRemoved, t,
        [t, name] (quint32 removed) {
            if (name == removed) {
                emit t->removed();
            }
        }
    );
    return t;
}

#define CREATE2(__NAME__, __BINDNAME__) \
__NAME__ *Registry::create##__NAME__(quint32 name, quint32 version, QObject *parent) \
{ \
    return d->create<__NAME__>(name, version, parent, &Registry::bind##__BINDNAME__); \
}

#define CREATE(__NAME__) CREATE2(__NAME__, __NAME__)

CREATE(Compositor)
CREATE(Seat)
CREATE(Shell)
CREATE(SubCompositor)
CREATE(FullscreenShell)
CREATE(Output)
CREATE(DataDeviceManager)
CREATE(PlasmaShell)
CREATE(PlasmaWindowManagement)
CREATE(Idle)
CREATE(FakeInput)
CREATE(OutputManagement)
CREATE(OutputDevice)
CREATE(ShadowManager)
CREATE(BlurManager)
CREATE(ContrastManager)
CREATE(SlideManager)
CREATE(DpmsManager)
CREATE2(ShmPool, Shm)

#undef CREATE
#undef CREATE2

static const wl_interface *wlInterface(Registry::Interface interface)
{
    auto it = s_interfaces.find(interface);
    if (it != s_interfaces.end()) {
        return it.value().interface;
    }
    return nullptr;
}

template <typename T>
T *Registry::Private::bind(Registry::Interface interface, uint32_t name, uint32_t version) const
{
    auto it = std::find_if(m_interfaces.begin(), m_interfaces.end(), [=](const InterfaceData &data) {
        return data.interface == interface && data.name == name && data.version >= version;
    });
    if (it == m_interfaces.end()) {
        qCDebug(KWAYLAND_CLIENT) << "Don't have interface " << int(interface) << "with name " << name << "and minimum version" << version;
        return nullptr;
    }
    auto t = reinterpret_cast<T*>(wl_registry_bind(registry, name, wlInterface(interface), version));
    if (queue) {
        queue->addProxy(t);
    }
    return t;
}

bool Registry::isValid() const
{
    return d->registry.isValid();
}

wl_registry *Registry::registry()
{
    return d->registry;
}

Registry::operator wl_registry*() const
{
    return d->registry;
}

Registry::operator wl_registry*()
{
    return d->registry;
}

}
}
