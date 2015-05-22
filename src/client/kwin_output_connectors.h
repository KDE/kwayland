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
    enum class SubPixel {
        Unknown,
        None,
        HorizontalRGB,
        HorizontalBGR,
        VerticalRGB,
        VerticalBGR
    };
    enum class Transform {
        Normal,
        Rotated90,
        Rotated180,
        Rotated270,
        Flipped,
        Flipped90,
        Flipped180,
        Flipped270
    };
    struct Mode {
        enum class Flag {
            None = 0,
            Current = 1 << 0,
            Preferred = 1 << 1
        };
        Q_DECLARE_FLAGS(Flags, Flag)
        /**
         * The size of this Mode in pixel space.
         **/
        QSize size;
        /**
         * The refresh rate in mHz of this Mode.
         **/
        int refreshRate = 0;
        /**
         * The flags of this Mode, that is whether it's the
         * Current and/or Preferred Mode of the KWinOutputConnectors.
         **/
        Flags flags = Flag::None;
        /**
         * The KWinOutputConnectors to which this Mode belongs.
         **/
        QPointer<KWinOutputConnectors> output;

        bool operator==(const Mode &m) const;
    };
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
    /**
     * Emitted whenever a new Mode is added.
     * This normally only happens during the initial promoting of modes.
     * Afterwards only modeChanged should be emitted.
     * @param mode The newly added Mode.
     * @see modeChanged
     **/
    void outputAppeared(const QString &name, const QString &connector);
    /**
     * Emitted whenever a Mode changes.
     * This normally means that the @c Mode::Flag::Current is added or removed.
     * @param mode The changed Mode
     **/
    void outputDisppeared(const QString &name, const QString &connector);

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

#endif
