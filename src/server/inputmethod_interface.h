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

    void sendSurroundingText(const QString &text, quint32 cursor, quint32 anchor);
    void sendReset();
    void sendContentType(quint32 hint, quint32 purpose);
    void sendInvokeAction(quint32 button, quint32 index);
    void sendCommitState(quint32 serial);
    void sendPreferredLanguage(const QString &language);

Q_SIGNALS:
    void commitString(quint32 serial, const QString &text);
    void preeditString(quint32 serial, const QString &text, const QString &commit);
    void preeditStyling(quint32 index, quint32 length, quint32 style);
    void preeditCursor(qint32 index);
    void deleteSurroundingText(qint32 index, quint32 length);
    void cursorPosition(qint32 index, qint32 anchor);
    void keysym(quint32 serial, quint32 time, quint32 sym, bool pressed, Qt::KeyboardModifiers modifiers);
    void grabKeyboard(quint32 keyboard);
    void key(quint32 serial, quint32 time, quint32 key, bool pressed);
    void modifiers(quint32 serial, Qt::KeyboardModifiers mods_depressed, Qt::KeyboardModifiers mods_latched, Qt::KeyboardModifiers mods_locked, quint32 group);
    void language(quint32 serial, const QString &language);
    void textDirection(quint32 serial, Qt::LayoutDirection direction);

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

    QHash<quint32, InputPanelSurfaceInterface*> surfaces() const;

Q_SIGNALS:
    void inputPanelSurfaceAdded(quint32 id, InputPanelSurfaceInterface* surface);

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
