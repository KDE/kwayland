# KWayland

KWayland is a Qt-style API to interact with the wayland-client and wayland-server API.

## Introduction

KWayland provides two libraries:

- KWayland::Client
- KWayland::Server

As the names suggest, KWayland implements a Client and a Server API for the Wayland protocol. 
The API is Qt-style, removing the need for a Qt developer to interact with a low-level C-API, 
were they uncomfortable with one. For example, the callback mechanism from the Wayland API is
replaced by signals; data types are adjusted to be close to what a Qt developer would expect: 
two arguments of int that are represented by a QPoint or a QSize.

## KWayland Server

### Head-less API

The server library can be used to implement a Wayland server with Qt. The API is head-less,
meaning it does not perform any output and is not restrictive on different rendering methods.
This allows to easily integrate in existing rendering code based on things like OpenGL or
QPainter. Applications built on top of the KWayland Server render the graphics using the
following technologies:

- OpenGL over DRM/KMS
- OpenGL over X11
- OpenGL over Wayland
- OpenGL over Android's hwcomposer enabled through libhybris
- QPainter over DRM/KMs
- QPainter over fbdev
- QPainter over X11
- QPainter over Wayland
- QWidget
- QtQuick

Although the library does not perform any output, it makes it very easy to enable rendering.
The representation for a [Buffer](@ref KWayland::Server::BufferInterface) allows easy conversion
to a (memory-shared) QImage in case the buffer represents a shared memory buffer. This QImage
can be used for rendering in a QPainter based API or to generate an OpenGL texture.

### Easy usage of Wayland API

The library hides many Wayland implementation details. For all Wayland interfaces which have a
double buffered state, the classes always only provide access to the committed state. The pending
state is an internal detail. On commit of the pending state, Qt signals are emitted about what
changed.

Buffers are ref-counted and automatically released if it is no longer referenced allowing the
client to reuse it. This happens fully automatically when a surface no longer references a buffer.
As long as a buffer is attached to a surface, the surface has it referenced and the user of the API
can access the buffer without needing to care about referencing it.

The API of KWayland is hand-crafted to make usage easier. The representation of a
[Surface](@ref KWayland::Server::SurfaceInterface) combines multiple aspects about a Surface even
if it is added to other elements in the Wayland API. As a result, a Surface contains all
[SubSurfaces](@ref KWayland::Server::SubSurfaceInterface) attached to it so that the user
doesn't have to monitor for which Surface a SubSurface was created.

Similarly, the representation of a [Seat](@ref KWayland::Server::SeatInterface) combines all aspects of
the Seat. A user of the API only needs to interact with the Seat, there is no need to track all the
created [keyboards](@ref KWayland::Server::KeyboardInterface), [pointers](@ref KWayland::Server::PointerInterface), etc. The
representation of Seat tracks which keyboards are generated and is able to forward events to the
proper focus surface, send enter and leave notifications when needed without the user of the API
having to care about it.

### Handling input events

Just like with the output, the server API is not restrictive on input events. This allows easier
integration with existing input handlers and it also allows one to easily filter the input before
it is passed to the server, and is then delegated to the client. As a result, one can filter out
things like global touch gestures or keyboard shortcuts without having to implement handlers inside
KWayland. The SeatInterface provides a very easy to use API that forward events which can be easily
integrated with Qt's existing input event system, there is, for example, a mapping from Qt::MouseButto
to the Linux input code.

Applications can be built on top of the KWayland Server's integrated input events with the following technologies:

- libinput
- X11
- Wayland
- Android's inputstack enabled through libhybris
- QInputEvent

### Private IPC with child processes

The KWayland Server is well suited for having a private IPC with child processes. The [Display](@ref KWayland::Server::Display) can be
setup in a way so that it doesn't create a public socket, but only allows connections through socket
pairs. This allows to create a socketpair, pass one file descriptor to the KWayland server and the other
to the forked process, e.g. through the WAYLAND_SOCKET environment variable. Thus a dedicated IPC
is created which can be used even to run your own custom protocol. For example, KDE Plasma uses
such a dedicated parent-child Wayland server in its screen locker architecture.

Of course private sockets can be added at any time in addition to a publicly available socket. This
can be used to recognize specific clients and to restrict access to interfaces for only some dedicated
clients.

## KWayland Client

The idea around the KWayland Client is to provide a drop-in API for the Wayland client library which at
the same time provides convenience Qt-style API. It is not intended to be used as a replacement for
the QtWayland QPA plugin, but rather as a way to interact with Wayland in case one needs Qt to use
a different QPA plugin or in combination with QtWayland to allow a lower-level interaction without
requiring to write C code.

### Convenience API

The convenience API in the KWayland Client provides one class wrapping a Wayland object. Each class can
be casted into the wrapped Wayland type. The API represents events as signals and provides simple
method calls for requests.

Classes representing global Wayland resources can be created through the [Registry](@ref KWayland::Client::Registry). This class eases
the interaction with the Wayland registry and emits signals whenever a new global is announced or gets
removed. The Registry has a list of known interfaces (e.g. common Wayland protocols like `wl_compositor`
or `wl_shell`) which have dedicated announce/removed signals and objects which can be factored by the Registry
for those globals.

Many globals function as a factory for further resources. For example, the Compositor has a factory 
method for surfaces. All objects can also be created in a low-level way, interacting directly with 
the Wayland API and also providing convenience factory methods. This allows the user to control the
Wayland API more easily and at a lower level should they need it.

### Integration with QtWayland QPA

KWayland integrates well with QGuiApplications using the QtWayland QPA. As a result, it is not
necessary to create a new connection to the Wayland server, since the one used by Qt can be reused.
If a Wayland object can be taken from Qt, the respective class provides a static method normally
called `fromApplication`. In addition, the API can get the Surface from a QWindow.

## Using KWayland in your application

### With CMake

KWayland installs a CMake Config file which allows to use KWayland as imported targets. There is
one library for the Client and one for the Server.

To find the package use for example:

    find_package(KF5Wayland CONFIG)
    set_package_properties(KF5Wayland PROPERTIES TYPE OPTIONAL )
    add_feature_info("KF5Wayland" KF5Wayland_FOUND "Required for the awesome Wayland on Qt demo")

Now to link against the Client library use:

    add_executable(exampleApp example.cpp)
    target_link_libraries(exampleApp KF5::WaylandClient)

To link against the Server library use:

    add_executable(exampleServer exampleServer.cpp)
    target_link_libraries(exampleServer KF5::WaylandServer)

### With QMake

KWayland installs .pri files for the Client and Server library allowing easy usage in QMake based
applications.

Just use:

    QT += KWaylandClient

Respectively:

    QT += KWaylandServer

Please make sure that your project is configured with C++11 support:

    CONFIG += c++11
