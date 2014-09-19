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
#ifndef WAYLAND_SHELL_H
#define WAYLAND_SHELL_H

#include <QObject>
#include <QSize>

#include <KWayland/Client/kwaylandclient_export.h>

struct wl_surface;
struct wl_shell;
struct wl_shell_surface;

namespace KWayland
{
namespace Client
{
class ShellSurface;
class Output;
class Surface;

class KWAYLANDCLIENT_EXPORT Shell : public QObject
{
    Q_OBJECT
public:
    explicit Shell(QObject *parent = nullptr);
    virtual ~Shell();

    bool isValid() const;
    void release();
    void destroy();
    void setup(wl_shell *shell);

    ShellSurface *createSurface(wl_surface *surface, QObject *parent = nullptr);
    ShellSurface *createSurface(Surface *surface, QObject *parent = nullptr);

    operator wl_shell*();
    operator wl_shell*() const;

Q_SIGNALS:
    void interfaceAboutToBeReleased();
    void interfaceAboutToBeDestroyed();

private:
    class Private;
    QScopedPointer<Private> d;
};

class KWAYLANDCLIENT_EXPORT ShellSurface : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QSize size READ size WRITE setSize NOTIFY sizeChanged)
public:
    explicit ShellSurface(QObject *parent);
    virtual ~ShellSurface();

    void release();
    void destroy();
    void setup(wl_shell_surface *surface);
    QSize size() const;
    void setSize(const QSize &size);

    void setFullscreen(Output *output = nullptr);

    bool isValid() const;
    operator wl_shell_surface*();
    operator wl_shell_surface*() const;

Q_SIGNALS:
    void pinged();
    void sizeChanged(const QSize &);

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
