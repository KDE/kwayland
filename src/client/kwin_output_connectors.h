/********************************************************************
Copyright 2013  Martin Gräßlin <mgraesslin@kde.org>

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
#ifndef WAYLAND_KWIN_OUTPUT_CONNECTORS_H
#define WAYLAND_KWIN_OUTPUT_CONNECTORS_H

#include <QObject>
#include <QPointer>
#include <QSize>

#include <KWayland/Client/kwaylandclient_export.h>

struct org_kde_kwin_output_connectors;
class QPoint;
class QRect;

namespace KWayland
{
namespace Client
{

/**
 * @short Wrapper for the org_kde_kwin_output_connectors interface.
 *
 * This class provides a convenient wrapper for the org_kde_kwin_output_connectors interface.
 * Its main purpose is to hold the information about one KWinOutputConnectors.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create an KWinOutputConnectors interface:
 * @code
 * KWinOutputConnectors *c = registry->createKWinOutputConnectors(name, version);
 * @endcode
 *
 * This creates the KWinOutputConnectors and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * KWinOutputConnectors *c = new KWinOutputConnectors;
 * c->setup(registry->bindKWinOutputConnectors(name, version));
 * @endcode
 *
 * The KWinOutputConnectors can be used as a drop-in replacement for any org_kde_kwin_output_connectors
 * pointer as it provides matching cast operators.
 *
 * Please note that all properties of KWinOutputConnectors are not valid until the
 * changed signal has been emitted. The wayland server is pushing the
 * information in an async way to the KWinOutputConnectors instance. By emitting changed
 * the KWinOutputConnectors indicates that all relevant information is available.
 *
 * @see Registry
 **/
class KWAYLANDCLIENT_EXPORT KWinOutputConnectors : public QObject
{
    Q_OBJECT
public:
    explicit KWinOutputConnectors(QObject *parent = nullptr);
    virtual ~KWinOutputConnectors();

    /**
     * Setup this Compositor to manage the @p output.
     * When using Registry::createKWinOutputConnectors there is no need to call this
     * method.
     **/
    void setup(org_kde_kwin_output_connectors *output);

    /**
     * @returns @c true if managing a org_kde_kwin_output_connectors.
     **/
    bool isValid() const;
    operator org_kde_kwin_output_connectors*();
    operator org_kde_kwin_output_connectors*() const;
    org_kde_kwin_output_connectors *output();

    void getDisabledOutputs();

Q_SIGNALS:
    /**
     * Emitted whenever at least one of the data changed.
     **/
    void sync();
    void outputAppeared(const QString &name, const QString &connector);
    void outputDisppeared(const QString &name, const QString &connector);

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
