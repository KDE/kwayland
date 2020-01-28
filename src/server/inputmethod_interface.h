/********************************************************************
Copyright 2020 Aleix Pol Gonzalez <aleixpol@kde.org>

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
#ifndef WAYLAND_SERVER_INPUTMETHOD_INTERFACE_H
#define WAYLAND_SERVER_INPUTMETHOD_INTERFACE_H

#include <KWayland/Server/kwaylandserver_export.h>
#include <QVector>

#include "resource.h"

namespace KWayland
{
namespace Server
{
class OutputInterface;
class SurfaceInterface;
class Display;

class KWAYLANDSERVER_EXPORT InputMethodInterface : public QObject
{
    Q_OBJECT
public:
    InputMethodInterface(Display *d, QObject *parent);
    virtual ~InputMethodInterface();

    void sendActivate(struct ::wl_resource *id);
    void sendDeactivate(struct ::wl_resource *context);

private:
    friend class InputMethodInterface;
    class Private;
    QScopedPointer<Private> d;
};

class KWAYLANDSERVER_EXPORT InputMethodContextInterface : public QObject
{
    Q_OBJECT
public:
    InputMethodContextInterface();
    virtual ~InputMethodContextInterface();

    void sendSurroundingText(const QString &text, uint32_t cursor, uint32_t anchor);
    void sendReset();
    void sendContentType(uint32_t hint, uint32_t purpose);
    void sendInvokeAction(uint32_t button, uint32_t index);
    void sendCommitState(uint32_t serial);
    void sendPreferredLanguage(const QString &language);

Q_SIGNALS:
    void commitString(uint serial, const QString &text);
    void preeditString(uint32_t serial, const QString &text, const QString &commit);
    void preeditStyling(uint32_t index, uint32_t length, uint32_t style);
    void preeditCursor(int32_t index);
    void deleteSurroundingText(int32_t index, uint32_t length);
    void cursorPosition(int32_t index, int32_t anchor);
//     void modifiersMap(int mods);
    void keysym(uint32_t serial, uint32_t time, uint32_t sym, uint32_t state, uint32_t modifiers);
    void grabKeyboard(uint32_t keyboard);
    void key(uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
    void modifiers(uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group);
    void language(uint32_t serial, const QString &language);
    void textDirection(uint32_t serial, uint32_t direction);

private:
    friend class InputMethodContextInterface;
    class Private;
    QScopedPointer<Private> d;
};

class KWAYLANDSERVER_EXPORT InputPanelInterface : public QObject
{
    Q_OBJECT
public:
    InputPanelInterface(QObject* parent);
    virtual ~InputPanelInterface();

    SurfaceInterface* inputPanelSurface(uint32_t id) const;

private:
    friend class InputPanelInterface;
    class Private;
    QScopedPointer<Private> d;
};

class KWAYLANDSERVER_EXPORT InputPanelSurfaceInterface : public QObject
{
    Q_OBJECT
public:
    InputPanelSurfaceInterface();
    virtual ~InputPanelSurfaceInterface();

    enum Position {
        CenterBottom = 0
    };
    Q_ENUM(Position)

    void setTopLevel(OutputInterface *output, Position position);
    void setOverlayPanel();
private:
    friend class InputPanelSurfaceInterface;
    class Private;
    QScopedPointer<Private> d;
};

}
}

Q_DECLARE_METATYPE(KWayland::Server::InputMethodInterface*)

#endif
