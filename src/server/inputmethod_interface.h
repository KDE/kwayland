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
class InputPanelSurfaceInterface;
class InputMethodContextInterface;

class KWAYLANDSERVER_EXPORT InputMethodInterface : public QObject
{
    Q_OBJECT
public:
    InputMethodInterface(Display *d, QObject *parent);
    virtual ~InputMethodInterface();

    InputMethodContextInterface* sendActivate();
    void sendDeactivate();

private:
    class Private;
    QScopedPointer<Private> d;
};

class KWAYLANDSERVER_EXPORT InputMethodContextInterface : public QObject
{
    Q_OBJECT
public:
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
    void keysym(uint32_t serial, uint32_t time, uint32_t sym, bool pressed, Qt::KeyboardModifiers modifiers);
    void grabKeyboard(uint32_t keyboard);
    void key(uint32_t serial, uint32_t time, uint32_t key, bool pressed);
    void modifiers(uint32_t serial, Qt::KeyboardModifiers mods_depressed, Qt::KeyboardModifiers mods_latched, Qt::KeyboardModifiers mods_locked, uint32_t group);
    void language(uint32_t serial, const QString &language);
    void textDirection(uint32_t serial, Qt::LayoutDirection direction);

private:
    friend class InputMethodInterface;
    InputMethodContextInterface(InputMethodInterface *parent);
    class Private;
    QScopedPointer<Private> d;
};

class KWAYLANDSERVER_EXPORT InputPanelInterface : public QObject
{
    Q_OBJECT
public:
    InputPanelInterface(Display* d, QObject* parent);
    virtual ~InputPanelInterface();

    QHash<uint32_t, InputPanelSurfaceInterface*> surfaces() const;

Q_SIGNALS:
    void inputPanelSurfaceAdded(uint32_t id, InputPanelSurfaceInterface* surface);

private:
    class Private;
    QScopedPointer<Private> d;
};

class KWAYLANDSERVER_EXPORT InputPanelSurfaceInterface : public QObject
{
    Q_OBJECT
public:
    virtual ~InputPanelSurfaceInterface();

    enum Position {
        CenterBottom = 0
    };
    Q_ENUM(Position)

    SurfaceInterface* surface() const;

Q_SIGNALS:
    void topLevel(OutputInterface *output, Position position);
    void overlayPanel();

private:
    InputPanelSurfaceInterface(QObject* parent);
    friend class InputPanelInterface;
    class Private;
    QScopedPointer<Private> d;
};

}
}

Q_DECLARE_METATYPE(KWayland::Server::InputMethodInterface*)

#endif
