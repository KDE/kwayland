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
#ifndef WAYLAND_OUTPUT_H
#define WAYLAND_OUTPUT_H

#include <QObject>
#include <QPointer>
#include <QSize>

#include <KWayland/Client/kwaylandclient_export.h>

struct wl_output;
class QPoint;
class QRect;

namespace KWayland
{
namespace Client
{

class EventQueue;

/**
 * @short Wrapper for the wl_output interface.
 *
 * This class provides a convenient wrapper for the wl_output interface.
 * Its main purpose is to hold the information about one Output.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create an Output interface:
 * @code
 * Output *c = registry->createOutput(name, version);
 * @endcode
 *
 * This creates the Output and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * Output *c = new Output;
 * c->setup(registry->bindOutput(name, version));
 * @endcode
 *
 * The Output can be used as a drop-in replacement for any wl_output
 * pointer as it provides matching cast operators.
 *
 * Please note that all properties of Output are not valid until the
 * changed signal has been emitted. The wayland server is pushing the
 * information in an async way to the Output instance. By emitting changed
 * the Output indicates that all relevant information is available.
 *
 * @see Registry
 **/
class KWAYLANDCLIENT_EXPORT Output : public QObject
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
         * Current and/or Preferred Mode of the Output.
         **/
        Flags flags = Flag::None;
        /**
         * The Output to which this Mode belongs.
         **/
        QPointer<Output> output;

        bool operator==(const Mode &m) const;
    };
    explicit Output(QObject *parent = nullptr);
    virtual ~Output();

    /**
     * Setup this Compositor to manage the @p output.
     * When using Registry::createOutput there is no need to call this
     * method.
     **/
    void setup(wl_output *output);

    /**
     * @returns @c true if managing a wl_output.
     **/
    bool isValid() const;
    operator wl_output*();
    operator wl_output*() const;
    wl_output *output();
    /**
     * Size in millimeters.
     **/
    QSize physicalSize() const;
    /**
     * Position within the global compositor space.
     **/
    QPoint globalPosition() const;
    /**
     * Textual description of the manufacturer.
     **/
    QString manufacturer() const;
    /**
     * Textual description of the model.
     **/
    QString model() const;
    /**
     * Size in the current mode.
     **/
    QSize pixelSize() const;
    /**
     * The geometry of this Output in pixels.
     * Convenient for QRect(globalPosition(), pixelSize()).
     * @see globalPosition
     * @see pixelSize
     **/
    QRect geometry() const;
    /**
     * Refresh rate in mHz of the current mode.
     **/
    int refreshRate() const;
    /**
     * Scaling factor of this output.
     *
     * A scale larger than 1 means that the compositor will automatically scale surface buffers
     * by this amount when rendering. This is used for very high resolution displays where
     * applications rendering at the native resolution would be too small to be legible.
     **/
    int scale() const;
    /**
     * Subpixel orientation of this Output.
     **/
    SubPixel subPixel() const;
    /**
     * Transform that maps framebuffer to Output.
     *
     * The purpose is mainly to allow clients render accordingly and tell the compositor,
     * so that for fullscreen surfaces, the compositor will still be able to scan out
     * directly from client surfaces.
     **/
    Transform transform() const;

    /**
     * @returns The Modes of this Output.
     **/
    QList<Mode> modes() const;

    /**
     * Sets the @p queue to use for bound proxies.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for bound proxies.
     **/
    EventQueue *eventQueue() const;

Q_SIGNALS:
    /**
     * Emitted whenever at least one of the data changed.
     **/
    void changed();
    /**
     * Emitted whenever a new Mode is added.
     * This normally only happens during the initial promoting of modes.
     * Afterwards only modeChanged should be emitted.
     * @param mode The newly added Mode.
     * @see modeChanged
     **/
    void modeAdded(const KWayland::Client::Output::Mode &mode);
    /**
     * Emitted whenever a Mode changes.
     * This normally means that the @c Mode::Flag::Current is added or removed.
     * @param mode The changed Mode
     **/
    void modeChanged(const KWayland::Client::Output::Mode &mode);

    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the Compositor got created by
     * Registry::createOutput
     *
     * @since 5.5
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

Q_DECLARE_METATYPE(KWayland::Client::Output::SubPixel)
Q_DECLARE_METATYPE(KWayland::Client::Output::Transform)
Q_DECLARE_METATYPE(KWayland::Client::Output::Mode)
Q_DECLARE_OPERATORS_FOR_FLAGS(KWayland::Client::Output::Mode::Flags)

#endif
