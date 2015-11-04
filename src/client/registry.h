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
#ifndef WAYLAND_REGISTRY_H
#define WAYLAND_REGISTRY_H

#include <QHash>
#include <QObject>

#include <KWayland/Client/kwaylandclient_export.h>

struct wl_compositor;
struct wl_data_device_manager;
struct wl_display;
struct wl_output;
struct wl_registry;
struct wl_seat;
struct wl_shell;
struct wl_shm;
struct wl_subcompositor;
struct _wl_fullscreen_shell;
struct org_kde_kwin_outputmanagement;
struct org_kde_kwin_outputdevice;
struct org_kde_kwin_fake_input;
struct org_kde_kwin_idle;
struct org_kde_kwin_dpms_manager;
struct org_kde_kwin_shadow_manager;
struct org_kde_kwin_blur_manager;
struct org_kde_kwin_contrast_manager;
struct org_kde_kwin_slide_manager;
struct org_kde_plasma_shell;
struct org_kde_plasma_window_management;

namespace KWayland
{
namespace Client
{

class Compositor;
class ConnectionThread;
class DataDeviceManager;
class DpmsManager;
class EventQueue;
class FakeInput;
class FullscreenShell;
class OutputManagement;
class OutputDevice;
class Idle;
class Output;
class PlasmaShell;
class PlasmaWindowManagement;
class Seat;
class ShadowManager;
class BlurManager;
class ContrastManager;
class SlideManager;
class Shell;
class ShmPool;
class SubCompositor;

/**
 * @short Wrapper for the wl_registry interface.
 *
 * The purpose of this class is to manage the wl_registry interface.
 * This class supports some well-known interfaces and can create a
 * wrapper class for those.
 *
 * The main purpose is to emit signals whenever a new interface is
 * added or an existing interface is removed. For the well known interfaces
 * dedicated signals are emitted allowing a user to connect directly to the
 * signal announcing the interface it is interested in.
 *
 * To create and setup the Registry one needs to call create with either a
 * wl_display from an existing Wayland connection or a ConnectionThread instance:
 *
 * @code
 * ConnectionThread *connection; // existing connection
 * Registry registry;
 * registry.create(connection);
 * registry.setup();
 * @endcode
 *
 * The interfaces are announced in an asynchronous way by the Wayland server.
 * To initiate the announcing of the interfaces one needs to call setup.
 **/
class KWAYLANDCLIENT_EXPORT Registry : public QObject
{
    Q_OBJECT
public:
    /**
     * The well-known interfaces this Registry supports.
     * For each of the enum values the Registry is able to create a Wrapper
     * object.
     **/
    enum class Interface {
        Unknown,    ///< Refers to an Unknown interface
        Compositor, ///< Refers to the wl_compositor interface
        Shell,      ///< Refers to the wl_shell interface
        Seat,       ///< Refers to the wl_seat interface
        Shm,        ///< Refers to the wl_shm interface
        Output,     ///< Refers to the wl_output interface
        FullscreenShell, ///< Refers to the _wl_fullscreen_shell interface
        SubCompositor, ///< Refers to the wl_subcompositor interface;
        DataDeviceManager, ///< Refers to the wl_data_device_manager interface
        PlasmaShell, ///< Refers to org_kde_plasma_shell interface
        PlasmaWindowManagement, ///< Refers to org_kde_plasma_window_management interface
        Idle, ///< Refers to org_kde_kwin_idle_interface interface
        FakeInput, ///< Refers to org_kde_kwin_fake_input interface
        Shadow, ///< Refers to org_kde_kwin_shadow_manager interface
        Blur, ///< refers to org_kde_kwin_blur_manager interface
        Contrast, ///< refers to org_kde_kwin_contrast_manager interface
        Slide, ///< refers to org_kde_kwin_slide_manager
        Dpms, ///< Refers to org_kde_kwin_dpms_manager interface
        OutputManagement, ///< Refers to the wl_data_device_manager interface
        OutputDevice     ///< Refers to the org_kde_kwin_outputdevice interface
    };
    explicit Registry(QObject *parent = nullptr);
    virtual ~Registry();

    /**
     * Releases the wl_registry interface.
     * After the interface has been released the Registry instance is no
     * longer valid and can be setup with another wl_registry interface.
     **/
    void release();
    /**
     * Destroys the data held by this Registry.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new wl_registry interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, registry, &Registry::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();
    /**
     * Gets the registry from the @p display.
     **/
    void create(wl_display *display);
    /**
     * Gets the registry from the @p connection.
     **/
    void create(ConnectionThread *connection);
    /**
     * Finalizes the setup of the Registry.
     * After calling this method the interfaces will be announced in an asynchronous way.
     * The Registry must have been created when calling this method.
     * @see create
     **/
    void setup();

    /**
     * Sets the @p queue to use for this Registry.
     *
     * The EventQueue should be set before the Registry gets setup.
     * The EventQueue gets automatically added to all interfaces created by
     * this Registry. So that all objects are in teh same EventQueue.
     *
     * @param queue The event queue to use for this Registry.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The EventQueue used by this Registry
     **/
    EventQueue *eventQueue();

    /**
     * @returns @c true if managing a wl_registry.
     **/
    bool isValid() const;
    /**
     * @returns @c true if the Registry has an @p interface.
     **/
    bool hasInterface(Interface interface) const;

    /**
     * Representation of one announced interface.
     **/
    struct AnnouncedInterface {
        /**
         * The name of the announced interface.
         **/
        quint32 name;
        /**
         * The maximum supported version of the announced interface.
         **/
        quint32 version;
    };
    /**
     * Provides name and version for the @p interface.
     *
     * The first value of the returned pair is the "name", the second value is the "version".
     * If the @p interface has not been announced, both values are set to 0.
     * If there @p interface has been announced multiple times, the last announced is returned.
     * In case one is interested in all announced interfaces, one should prefer @link interfaces(Interface) @endlink.
     *
     * The returned information can be passed into the bind or create methods.
     *
     * @param interface The well-known interface for which the name and version should be retrieved
     * @returns name and version of the given interface
     * @since 5.5
     **/
    AnnouncedInterface interface(Interface interface) const;
    /**
     * Provides all pairs of name and version for the well-known @p interface.
     *
     * If the @p interface has not been announced, an empty vector is returned.
     *
     * The returned information can be passed into the bind or create methods.
     *
     * @param interface The well-known interface for which the name and version should be retrieved
     * @returns All pairs of name and version of the given interface
     * @since 5.5
     **/
    QVector<AnnouncedInterface> interfaces(Interface interface) const;

    /**
     * @name Low-level bind methods for global interfaces.
     **/
    ///@{
    /**
     * Binds the wl_compositor with @p name and @p version.
     * If the @p name does not exist or is not for the compositor interface,
     * @c null will be returned.
     *
     * Prefer using createCompositor instead.
     * @see createCompositor
     **/
    wl_compositor *bindCompositor(uint32_t name, uint32_t version) const;
    /**
     * Binds the wl_shell with @p name and @p version.
     * If the @p name does not exist or is not for the shell interface,
     * @c null will be returned.
     *
     * Prefer using createShell instead.
     * @see createShell
     **/
    wl_shell *bindShell(uint32_t name, uint32_t version) const;
    /**
     * Binds the wl_seat with @p name and @p version.
     * If the @p name does not exist or is not for the seat interface,
     * @c null will be returned.
     *
     * Prefer using createSeat instead.
     * @see createSeat
     **/
    wl_seat *bindSeat(uint32_t name, uint32_t version) const;
    /**
     * Binds the wl_shm with @p name and @p version.
     * If the @p name does not exist or is not for the shm interface,
     * @c null will be returned.
     *
     * Prefer using createShmPool instead.
     * @see createShmPool
     **/
    wl_shm *bindShm(uint32_t name, uint32_t version) const;
    /**
     * Binds the org_kde_kwin_outputmanagement with @p name and @p version.
     * If the @p name does not exist or is not for the outputmanagement interface,
     * @c null will be returned.
     *
     * Prefer using createOutputManagement instead.
     * @see createOutputManagement
     **/
    org_kde_kwin_outputmanagement *bindOutputManagement(uint32_t name, uint32_t version) const;
    /**
     * Binds the org_kde_kwin_outputdevice with @p name and @p version.
     * If the @p name does not exist or is not for the outputdevice interface,
     * @c null will be returned.
     *
     * Prefer using createOutputDevice instead.
     * @see createOutputDevice
     **/
    wl_output *bindOutput(uint32_t name, uint32_t version) const;
    /**
     * Binds the wl_subcompositor with @p name and @p version.
     * If the @p name does not exist or is not for the subcompositor interface,
     * @c null will be returned.
     *
     * Prefer using createSubCompositor instead.
     * @see createSubCompositor
     **/
    wl_subcompositor *bindSubCompositor(uint32_t name, uint32_t version) const;
    /**
     * Binds the wl_output with @p name and @p version.
     * If the @p name does not exist or is not for the output interface,
     * @c null will be returned.
     *
     * Prefer using createOutput instead.
     * @see createOutput
     * @since 5.5
     **/
    org_kde_kwin_outputdevice *bindOutputDevice(uint32_t name, uint32_t version) const;

    /**
     * Binds the _wl_fullscreen_shell with @p name and @p version.
     * If the @p name does not exist or is not for the fullscreen shell interface,
     * @c null will be returned.
     *
     * Prefer using createFullscreenShell instead.
     * @see createFullscreenShell
     **/
    _wl_fullscreen_shell *bindFullscreenShell(uint32_t name, uint32_t version) const;
    /**
     * Binds the wl_data_device_manager with @p name and @p version.
     * If the @p name does not exist or is not for the data device manager interface,
     * @c null will be returned.
     *
     * Prefer using createDataDeviceManager instead.
     * @see createDataDeviceManager
     **/
    wl_data_device_manager *bindDataDeviceManager(uint32_t name, uint32_t version) const;
    /**
     * Binds the org_kde_plasma_shell with @p name and @p version.
     * If the @p name does not exist or is not for the Plasma shell interface,
     * @c null will be returned.
     *
     * Prefer using createPlasmaShell instead.
     * @see createPlasmaShell
     * @since 5.4
     **/
    org_kde_plasma_shell *bindPlasmaShell(uint32_t name, uint32_t version) const;
    /**
     * Binds the org_kde_plasma_window_management with @p name and @p version.
     * If the @p name does not exist or is not for the Plasma window management interface,
     * @c null will be returned.
     *
     * Prefer using createPlasmaWindowManagement instead.
     * @see createPlasmaWindowManagement
     * @since 5.4
     **/
    org_kde_plasma_window_management *bindPlasmaWindowManagement(uint32_t name, uint32_t version) const;
    /**
     * Binds the org_kde_kwin_idle with @p name and @p version.
     * If the @p name does not exist or is not for the idle interface,
     * @c null will be returned.
     *
     * Prefer using createIdle instead.
     * @see createIdle
     * @since 5.4
     **/
    org_kde_kwin_idle *bindIdle(uint32_t name, uint32_t version) const;
    /**
     * Binds the org_kde_kwin_fake_input with @p name and @p version.
     * If the @p name does not exist or is not for the fake input interface,
     * @c null will be returned.
     *
     * Prefer using createFakeInput instead.
     * @see createFakeInput
     * @since 5.4
     **/
    org_kde_kwin_fake_input *bindFakeInput(uint32_t name, uint32_t version) const;
    /**
     * Binds the org_kde_kwin_shadow_manager with @p name and @p version.
     * If the @p name does not exist or is not for the shadow manager interface,
     * @c null will be returned.
     *
     * Prefer using createShadowManager instead.
     * @see createShadowManager
     * @since 5.4
     **/
    org_kde_kwin_shadow_manager *bindShadowManager(uint32_t name, uint32_t version) const;
    /**
     * Binds the org_kde_kwin_blur_manager with @p name and @p version.
     * If the @p name does not exist or is not for the blur manager interface,
     * @c null will be returned.
     *
     * Prefer using createBlurManager instead.
     * @see createBlurManager
     * @since 5.5
     **/
    org_kde_kwin_blur_manager *bindBlurManager(uint32_t name, uint32_t version) const;
    /**
     * Binds the org_kde_kwin_contrast_manager with @p name and @p version.
     * If the @p name does not exist or is not for the contrast manager interface,
     * @c null will be returned.
     *
     * Prefer using createContrastManager instead.
     * @see createContrastManager
     * @since 5.5
     **/
    org_kde_kwin_contrast_manager *bindContrastManager(uint32_t name, uint32_t version) const;
    /**
     * Binds the org_kde_kwin_slide_manager with @p name and @p version.
     * If the @p name does not exist or is not for the slide manager interface,
     * @c null will be returned.
     *
     * Prefer using createSlideManager instead.
     * @see createSlideManager
     * @since 5.5
     **/
    org_kde_kwin_slide_manager * bindSlideManager(uint32_t name, uint32_t version) const;
    /**
     * Binds the org_kde_kwin_dpms_manager with @p name and @p version.
     * If the @p name does not exist or is not for the dpms manager interface,
     * @c null will be returned.
     *
     * Prefer using createDpmsManager instead.
     * @see createDpmsManager
     * @since 5.5
     **/
    org_kde_kwin_dpms_manager *bindDpmsManager(uint32_t name, uint32_t version) const;
    ///@}

    /**
     * @name Convenient factory methods for global objects.
     **/
    ///@{
    /**
     * Creates a Compositor and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the wl_compositor interface,
     * the returned Compositor will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the wl_compositor interface to bind
     * @param version The version or the wl_compositor interface to use
     * @param parent The parent for Compositor
     *
     * @returns The created Compositor.
     **/
    Compositor *createCompositor(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a Seat and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the wl_seat interface,
     * the returned Seat will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the wl_seat interface to bind
     * @param version The version or the wl_seat interface to use
     * @param parent The parent for Seat
     *
     * @returns The created Seat.
     **/
    Shell *createShell(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a Compositor and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the wl_compositor interface,
     * the returned Compositor will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the wl_compositor interface to bind
     * @param version The version or the wl_compositor interface to use
     * @param parent The parent for Compositor
     *
     * @returns The created Compositor.
     **/
    Seat *createSeat(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a ShmPool and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the wl_shm interface,
     * the returned ShmPool will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the wl_shm interface to bind
     * @param version The version or the wl_shm interface to use
     * @param parent The parent for ShmPool
     *
     * @returns The created ShmPool.
     **/
    ShmPool *createShmPool(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a SubCompositor and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the wl_subcompositor interface,
     * the returned SubCompositor will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the wl_subcompositor interface to bind
     * @param version The version or the wl_subcompositor interface to use
     * @param parent The parent for SubCompositor
     *
     * @returns The created SubCompositor.
     **/
    SubCompositor *createSubCompositor(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates an Output and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the wl_output interface,
     * the returned Output will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the wl_output interface to bind
     * @param version The version or the wl_output interface to use
     * @param parent The parent for Output
     *
     * @returns The created Output.
     **/
    Output *createOutput(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates an KWinOutputManagement and sets it up to manage the interface identified
     * by @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the wl_output interface,
     * the returned KWinConnectors will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the org_kde_kwin_outputmanagement interface to bind
     * @param version The version or the org_kde_kwin_outputmanagement interface to use
     * @param parent The parent for KWinOutputManagement
     *
     * @returns The created KWinOutputManagement.
     * @since 5.5
     **/
    OutputManagement *createOutputManagement(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates an OutputDevice and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the org_kde_kwin_outputdevice interface,
     * the returned OutputDevice will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the org_kde_kwin_outputdevice interface to bind
     * @param version The version or the org_kde_kwin_outputdevice interface to use
     * @param parent The parent for OutputDevice
     *
     * @returns The created Output.
     * @since 5.5
     **/
    OutputDevice *createOutputDevice(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a FullscreenShell and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the _wl_fullscreen_shell interface,
     * the returned FullscreenShell will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the _wl_fullscreen_shell interface to bind
     * @param version The version or the _wl_fullscreen_shell interface to use
     * @param parent The parent for FullscreenShell
     *
     * @returns The created FullscreenShell.
     * @since 5.5
     **/
    FullscreenShell *createFullscreenShell(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a DataDeviceManager and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the wl_data_device_manager interface,
     * the returned DataDeviceManager will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the wl_data_device_manager interface to bind
     * @param version The version or the wl_data_device_manager interface to use
     * @param parent The parent for DataDeviceManager
     *
     * @returns The created DataDeviceManager.
     **/
    DataDeviceManager *createDataDeviceManager(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a PlasmaShell and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the org_kde_plasma_shell interface,
     * the returned PlasmaShell will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the org_kde_plasma_shell interface to bind
     * @param version The version or the org_kde_plasma_shell interface to use
     * @param parent The parent for PlasmaShell
     *
     * @returns The created PlasmaShell.
     * @since 5.4
     **/
    PlasmaShell *createPlasmaShell(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a PlasmaWindowManagement and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the org_kde_plasma_window_management interface,
     * the returned PlasmaWindowManagement will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the org_kde_plasma_window_management interface to bind
     * @param version The version or the org_kde_plasma_window_management interface to use
     * @param parent The parent for PlasmaWindowManagement
     *
     * @returns The created PlasmaWindowManagement.
     * @since 5.4
     **/
    PlasmaWindowManagement *createPlasmaWindowManagement(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates an Idle and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the org_kde_kwin_idle interface,
     * the returned Idle will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the org_kde_kwin_idle interface to bind
     * @param version The version or the org_kde_kwin_idle interface to use
     * @param parent The parent for Idle
     *
     * @returns The created Idle.
     * @since 5.4
     **/
    Idle *createIdle(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a FakeInput and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the org_kde_kwin_fake_input interface,
     * the returned FakeInput will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the org_kde_kwin_fake_input interface to bind
     * @param version The version or the org_kde_kwin_fake_input interface to use
     * @param parent The parent for FakeInput
     *
     * @returns The created FakeInput.
     * @since 5.4
     **/
    FakeInput *createFakeInput(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a ShadowManager and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the org_kde_kwin_shadow_manager interface,
     * the returned ShadowManager will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the org_kde_kwin_shadow_manager interface to bind
     * @param version The version or the org_kde_kwin_shadow_manager interface to use
     * @param parent The parent for ShadowManager
     *
     * @returns The created ShadowManager.
     * @since 5.4
     **/
    ShadowManager *createShadowManager(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a BlurManager and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the org_kde_kwin_blur_manager interface,
     * the returned BlurManager will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the org_kde_kwin_blur_manager interface to bind
     * @param version The version or the org_kde_kwin_blur_manager interface to use
     * @param parent The parent for BlurManager
     *
     * @returns The created BlurManager.
     * @since 5.5
     **/
    BlurManager *createBlurManager(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a ContrastManager and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the org_kde_kwin_contrast_manager interface,
     * the returned ContrastManager will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the org_kde_kwin_contrast_manager interface to bind
     * @param version The version or the org_kde_kwin_contrast_manager interface to use
     * @param parent The parent for ContrastManager
     *
     * @returns The created ContrastManager.
     * @since 5.5
     **/
    ContrastManager *createContrastManager(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a SlideManager and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the org_kde_kwin_slide_manager interface,
     * the returned SlideManager will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the org_kde_kwin_slide_manager interface to bind
     * @param version The version or the org_kde_kwin_slide_manager interface to use
     * @param parent The parent for SlideManager
     *
     * @returns The created SlideManager.
     * @since 5.5
     **/
    SlideManager *createSlideManager(quint32 name, quint32 version, QObject *parent = nullptr);
    /**
     * Creates a DpmsManager and sets it up to manage the interface identified by
     * @p name and @p version.
     *
     * Note: in case @p name is invalid or isn't for the org_kde_kwin_dpms_manager interface,
     * the returned DpmsManager will not be valid. Therefore it's recommended to call
     * isValid on the created instance.
     *
     * @param name The name of the org_kde_kwin_dpms_manager interface to bind
     * @param version The version or the org_kde_kwin_dpms_manager interface to use
     * @param parent The parent for DpmsManager
     *
     * @returns The created DpmsManager.
     * @since 5.5
     **/
    DpmsManager *createDpmsManager(quint32 name, quint32 version, QObject *parent = nullptr);
    ///@}

    /**
     * cast operator to the low-level Wayland @c wl_registry
     **/
    operator wl_registry*();
    /**
     * cast operator to the low-level Wayland @c wl_registry
     **/
    operator wl_registry*() const;
    /**
     * @returns access to the low-level Wayland @c wl_registry
     **/
    wl_registry *registry();

Q_SIGNALS:
    /**
     * @name Interface announced signals.
     **/
    ///@{
    /**
     * Emitted whenever a wl_compositor interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     **/
    void compositorAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a wl_shell interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     **/
    void shellAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a wl_seat interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     **/
    void seatAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a wl_shm interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     **/
    void shmAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a wl_subcompositor interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     **/
    void subCompositorAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a wl_output interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     **/
    void outputAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a _wl_fullscreen_shell interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     **/
    void fullscreenShellAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a wl_data_device_manager interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     **/
    void dataDeviceManagerAnnounced(quint32 name, quint32 version);

    void outputManagementAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a org_kde_kwin_outputdevice interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.5
     **/
    void outputDeviceAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a org_kde_plasma_shell interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.4
     **/
    void plasmaShellAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a org_kde_plasma_window_management interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.4
     **/
    void plasmaWindowManagementAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a org_kde_kwin_idle interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.4
     **/
    void idleAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a org_kde_kwin_fake_input interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.4
     **/
    void fakeInputAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a org_kde_kwin_shadow_manager interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.4
     **/
    void shadowAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a org_kde_kwin_blur_manager interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.5
     **/
    void blurAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a org_kde_kwin_contrast_manager interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.5
     **/
    void contrastAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a org_kde_kwin_slide_manager interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.5
     **/
    void slideAnnounced(quint32 name, quint32 version);
    /**
     * Emitted whenever a org_kde_kwin_dpms_manager interface gets announced.
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     * @since 5.5
     **/
    void dpmsAnnounced(quint32 name, quint32 version);
    ///@}
    /**
     * @name Interface removed signals.
     **/
    ///@{
    /**
     * Emitted whenever a wl_compositor interface gets removed.
     * @param name The name for the removed interface
     **/
    void compositorRemoved(quint32 name);
    /**
     * Emitted whenever a wl_shell interface gets removed.
     * @param name The name for the removed interface
     **/
    void shellRemoved(quint32 name);
    /**
     * Emitted whenever a wl_seat interface gets removed.
     * @param name The name for the removed interface
     **/
    void seatRemoved(quint32 name);
    /**
     * Emitted whenever a wl_shm interface gets removed.
     * @param name The name for the removed interface
     **/
    void shmRemoved(quint32 name);
    /**
     * Emitted whenever a wl_subcompositor interface gets removed.
     * @param name The name for the removed interface
     **/
    void subCompositorRemoved(quint32 name);
    /**
     * Emitted whenever a wl_output interface gets removed.
     * @param name The name for the removed interface
     **/
    void outputRemoved(quint32 name);
    /**
     * Emitted whenever a _wl_fullscreen_shell interface gets removed.
     * @param name The name for the removed interface
     **/
    void fullscreenShellRemoved(quint32 name);
    /**
     * Emitted whenever a wl_data_device_manager interface gets removed.
     * @param name The name for the removed interface
     **/
    void dataDeviceManagerRemoved(quint32 name);
    /**
     * Emitted whenever a org_kde_kwin_outputmanagement interface gets removed.
     * @param name The name for the removed interface
     * @since 5.5
     **/
    void outputManagementRemoved(quint32 name);
    /**
     * Emitted whenever a org_kde_kwin_outputdevice interface gets removed.
     * @param name The name for the removed interface
     * @since 5.5
     **/
    void outputDeviceRemoved(quint32 name);
    /**
     * Emitted whenever a org_kde_plasma_shell interface gets removed.
     * @param name The name for the removed interface
     * @since 5.4
     **/
    void plasmaShellRemoved(quint32 name);
    /**
     * Emitted whenever a org_kde_plasma_window_management interface gets removed.
     * @param name The name for the removed interface
     * @since 5.4
     **/
    void plasmaWindowManagementRemoved(quint32 name);
    /**
     * Emitted whenever a org_kde_kwin_idle interface gets removed.
     * @param name The name for the removed interface
     * @since 5.4
     **/
    void idleRemoved(quint32 name);
    /**
     * Emitted whenever a org_kde_kwin_fake_input interface gets removed.
     * @param name The name for the removed interface
     * @since 5.4
     **/
    void fakeInputRemoved(quint32 name);
    /**
     * Emitted whenever a org_kde_kwin_shadow_manager interface gets removed.
     * @param name The name for the removed interface
     * @since 5.4
     **/
    void shadowRemoved(quint32 name);
    /**
     * Emitted whenever a org_kde_kwin_blur_manager interface gets removed.
     * @param name The name for the removed interface
     * @since 5.5
     **/
    void blurRemoved(quint32 name);
    /**
     * Emitted whenever a org_kde_kwin_contrast_manager interface gets removed.
     * @param name The name for the removed interface
     * @since 5.5
     **/
    void contrastRemoved(quint32 name);
    /**
     * Emitted whenever a org_kde_kwin_slide_manager interface gets removed.
     * @param name The name for the removed interface
     * @since 5.5
     **/
    void slideRemoved(quint32 name);
    /**
     * Emitted whenever a org_kde_kwin_dpms_manager interface gets removed.
     * @param name The name for the removed interface
     * @since 5.5
     **/
    void dpmsRemoved(quint32 name);
    ///@}
    /**
     * Generic announced signal which gets emitted whenever an interface gets
     * announced.
     *
     * This signal is emitted before the dedicated signals are handled. If one
     * wants to know about one of the well-known interfaces use the dedicated
     * signals instead. Especially the bind methods might fail before the dedicated
     * signals are emitted.
     *
     * @param interface The interface (e.g. wl_compositor) which is announced
     * @param name The name for the announced interface
     * @param version The maximum supported version of the announced interface
     **/
    void interfaceAnnounced(QByteArray interface, quint32 name, quint32 version);
    /**
     * Generic removal signal which gets emitted whenever an interface gets removed.
     *
     * This signal is emitted after the dedicated signals are handled.
     *
     * @param name The name for the removed interface
     **/
    void interfaceRemoved(quint32 name);
    /**
     * Emitted when the Wayland display is done flushing the initial interface
     * callbacks, announcing wl_display properties. This can be used to compress
     * events. Note that this signal is emitted only after announcing interfaces,
     * such as outputs, but not after receiving callbacks of interface properties,
     * such as the output's geometry, modes, etc..
     * This signal is emitted from the wl_display_sync callback.
     **/
    void interfacesAnnounced();

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
