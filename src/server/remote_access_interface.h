/****************************************************************************
Copyright 2016  Oleg Chernovskiy <kanedias@xaker.ru>

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
****************************************************************************/
#ifndef KWAYLAND_SERVER_REMOTE_ACCESS_H
#define KWAYLAND_SERVER_REMOTE_ACCESS_H

#include "global.h"

namespace KWayland
{
namespace Server
{

class Display;
class OutputInterface;

/**
 * The structure server should fill to use this interface.
 * Lifecycle:
 *  1. BufferHandle is filled and passed to RemoteAccessManager
 *     (stored in manager's sent list)
 *  2. Clients confirm that they wants this buffer, the RemoteBuffer 
 *     interfaces are then created and wrapped around BufferHandle.
 *  3. Once all clients are done with buffer (or disconnected),
 *     RemoteBuffer notifies manager and release signal is emitted.
 * 
 *     It's the responsibility of your process to delete this BufferHandle
 *     and release its' fd afterwards.
 **/
class KWAYLANDSERVER_EXPORT BufferHandle
{
public:
    explicit BufferHandle();
    virtual ~BufferHandle();
    void setFd(qint32 fd);
    void setSize(quint32 width, quint32 height);
    void setStride(quint32 stride);
    void setFormat(quint32 format);
  
    qint32 fd() const;
    quint32 height() const;
    quint32 width() const;
    quint32 stride() const;
    quint32 format() const;
private:

    friend class RemoteAccessManagerInterface;
    friend class RemoteBufferInterface;
    class Private;
    QScopedPointer<Private> d;
};

class KWAYLANDSERVER_EXPORT RemoteAccessManagerInterface : public Global
{
    Q_OBJECT
public:
    virtual ~RemoteAccessManagerInterface() = default;

    /**
     * Store buffer in sent list and notify client that we have a buffer for it
     **/
    void sendBufferReady(const OutputInterface *output, const BufferHandle *buf);
    /**
     * Check whether interface has been bound
     **/
    bool isBound() const;

Q_SIGNALS:
    /**
     * Previously sent buffer has been released by client
     */
    void bufferReleased(const BufferHandle *buf);

private:
    explicit RemoteAccessManagerInterface(Display *display, QObject *parent = nullptr);
    friend class Display;
    class Private;
};

}
}

#endif
