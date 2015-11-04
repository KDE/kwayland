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
#include "outputdevice.h"
#include "wayland_pointer_p.h"
// Qt
#include <QPoint>
#include <QRect>
#include <QSize>
// wayland
#include "wayland-org_kde_kwin_outputdevice-client-protocol.h"
#include <wayland-client-protocol.h>
#include <QDebug>

namespace KWayland
{

namespace Client
{

typedef QList<OutputDevice::Mode> Modes;

class OutputDevice::Private
{
public:
    Private(OutputDevice *q);
    void setup(org_kde_kwin_outputdevice *o);

    WaylandPointer<org_kde_kwin_outputdevice, org_kde_kwin_outputdevice_destroy> output;
    EventQueue *queue = nullptr;
    QSize physicalSize;
    QPoint globalPosition;
    QString manufacturer;
    QString model;
    int scale = 1;
    SubPixel subPixel = SubPixel::Unknown;
    Transform transform = Transform::Normal;
    Modes modes;
    Modes::iterator currentMode = modes.end();

    QByteArray edid;
    OutputDevice::Enablement enabled = OutputDevice::Enablement::Enabled;
    QByteArray uuid;
    bool done = false;

private:
    static void geometryCallback(void *data, org_kde_kwin_outputdevice *output, int32_t x, int32_t y,
                                 int32_t physicalWidth, int32_t physicalHeight, int32_t subPixel,
                                 const char *make, const char *model, int32_t transform);
    static void modeCallback(void *data, org_kde_kwin_outputdevice *output, uint32_t flags, int32_t width, int32_t height, int32_t refresh, int32_t mode_id);
    static void doneCallback(void *data, org_kde_kwin_outputdevice *output);
    static void scaleCallback(void *data, org_kde_kwin_outputdevice *output, int32_t scale);

    static void edidCallback(void *data, org_kde_kwin_outputdevice *output, const char *raw);
    static void enabledCallback(void *data, org_kde_kwin_outputdevice *output, int32_t enabled);
    static void uuidCallback(void *data, org_kde_kwin_outputdevice *output, const char *uuid);

    void setPhysicalSize(const QSize &size);
    void setGlobalPosition(const QPoint &pos);
    void setManufacturer(const QString &manufacturer);
    void setModel(const QString &model);
    void setScale(int scale);
    void setSubPixel(SubPixel subPixel);
    void setTransform(Transform transform);
    void addMode(uint32_t flags, int32_t width, int32_t height, int32_t refresh, int32_t mode_id);

    OutputDevice *q;
    static struct org_kde_kwin_outputdevice_listener s_outputListener;
};

OutputDevice::Private::Private(OutputDevice *q)
    : q(q)
{
}

void OutputDevice::Private::setup(org_kde_kwin_outputdevice *o)
{
    Q_ASSERT(o);
    Q_ASSERT(!output);
    output.setup(o);
    org_kde_kwin_outputdevice_add_listener(output, &s_outputListener, this);
}

bool OutputDevice::Mode::operator==(const OutputDevice::Mode &m) const
{
    return size == m.size
           && refreshRate == m.refreshRate
           && flags == m.flags
           && output == m.output;
}

OutputDevice::OutputDevice(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

OutputDevice::~OutputDevice()
{
    d->output.release();
}

org_kde_kwin_outputdevice_listener OutputDevice::Private::s_outputListener = {
    geometryCallback,
    modeCallback,
    doneCallback,
    scaleCallback,
    edidCallback,
    enabledCallback,
    uuidCallback
};

void OutputDevice::Private::geometryCallback(void *data, org_kde_kwin_outputdevice *output,
                              int32_t x, int32_t y,
                              int32_t physicalWidth, int32_t physicalHeight,
                              int32_t subPixel, const char *make, const char *model, int32_t transform)
{
    Q_UNUSED(transform)
    auto o = reinterpret_cast<OutputDevice::Private*>(data);
    Q_ASSERT(o->output == output);
    o->setGlobalPosition(QPoint(x, y));
    o->setManufacturer(make);
    o->setModel(model);
    o->setPhysicalSize(QSize(physicalWidth, physicalHeight));
    auto toSubPixel = [subPixel]() {
        switch (subPixel) {
        case WL_OUTPUT_SUBPIXEL_NONE:
            return SubPixel::None;
        case WL_OUTPUT_SUBPIXEL_HORIZONTAL_RGB:
            return SubPixel::HorizontalRGB;
        case WL_OUTPUT_SUBPIXEL_HORIZONTAL_BGR:
            return SubPixel::HorizontalBGR;
        case WL_OUTPUT_SUBPIXEL_VERTICAL_RGB:
            return SubPixel::VerticalRGB;
        case WL_OUTPUT_SUBPIXEL_VERTICAL_BGR:
            return SubPixel::VerticalBGR;
        case WL_OUTPUT_SUBPIXEL_UNKNOWN:
        default:
            return SubPixel::Unknown;
        }
    };
    o->setSubPixel(toSubPixel());
    auto toTransform = [transform]() {
        switch (transform) {
        case WL_OUTPUT_TRANSFORM_90:
            return Transform::Rotated90;
        case WL_OUTPUT_TRANSFORM_180:
            return Transform::Rotated180;
        case WL_OUTPUT_TRANSFORM_270:
            return Transform::Rotated270;
        case WL_OUTPUT_TRANSFORM_FLIPPED:
            return Transform::Flipped;
        case WL_OUTPUT_TRANSFORM_FLIPPED_90:
            return Transform::Flipped90;
        case WL_OUTPUT_TRANSFORM_FLIPPED_180:
            return Transform::Flipped180;
        case WL_OUTPUT_TRANSFORM_FLIPPED_270:
            return Transform::Flipped270;
        case WL_OUTPUT_TRANSFORM_NORMAL:
        default:
            return Transform::Normal;
        }
    };
    o->setTransform(toTransform());
}

void OutputDevice::Private::modeCallback(void *data, org_kde_kwin_outputdevice *output, uint32_t flags, int32_t width, int32_t height, int32_t refresh, int32_t mode_id)
{
    auto o = reinterpret_cast<OutputDevice::Private*>(data);
    Q_ASSERT(o->output == output);
    o->addMode(flags, width, height, refresh, mode_id);
}

void OutputDevice::Private::addMode(uint32_t flags, int32_t width, int32_t height, int32_t refresh, int32_t mode_id)
{
    Mode mode;
    mode.output = QPointer<OutputDevice>(q);
    mode.refreshRate = refresh;
    mode.size = QSize(width, height);
    mode.id = mode_id;
    if (flags & WL_OUTPUT_MODE_CURRENT) {
        mode.flags |= Mode::Flag::Current;
    }
    if (flags & WL_OUTPUT_MODE_PREFERRED) {
        mode.flags |= Mode::Flag::Preferred;
    }
    auto currentIt = modes.insert(modes.end(), mode);
    bool existing = false;
    if (flags & WL_OUTPUT_MODE_CURRENT) {
        auto it = modes.begin();
        while (it != currentIt) {
            auto &m = (*it);
            if (m.flags.testFlag(Mode::Flag::Current)) {
                m.flags &= ~Mode::Flags(Mode::Flag::Current);
                emit q->modeChanged(m);
            }
            if (m.refreshRate == mode.refreshRate && m.size == mode.size) {
                it = modes.erase(it);
                existing = true;
            } else {
                it++;
            }
        }
        currentMode = currentIt;
    }
    if (existing) {
        emit q->modeChanged(mode);
    } else {
        emit q->modeAdded(mode);
    }
}

KWayland::Client::OutputDevice::Mode OutputDevice::currentMode() const
{
    for (const auto &m: modes()) {
        if (m.flags.testFlag(KWayland::Client::OutputDevice::Mode::Flag::Current)) {
            return m;
        }
    }
    qWarning() << "current mode not found";
    return Mode();
}

void OutputDevice::Private::scaleCallback(void *data, org_kde_kwin_outputdevice *output, int32_t scale)
{
    auto o = reinterpret_cast<OutputDevice::Private*>(data);
    Q_ASSERT(o->output == output);
    o->setScale(scale);
}

void OutputDevice::Private::doneCallback(void *data, org_kde_kwin_outputdevice *output)
{
    auto o = reinterpret_cast<OutputDevice::Private*>(data);
    Q_ASSERT(o->output == output);
    o->done = true;
    emit o->q->changed();
    emit o->q->done();
}

void OutputDevice::Private::edidCallback(void* data, org_kde_kwin_outputdevice* output, const char* raw)
{
    auto o = reinterpret_cast<OutputDevice::Private*>(data);
    o->edid = raw;
}

void OutputDevice::Private::enabledCallback(void* data, org_kde_kwin_outputdevice* output, int32_t enabled)
{
    auto o = reinterpret_cast<OutputDevice::Private*>(data);

    OutputDevice::Enablement _enabled = OutputDevice::Enablement::Disabled;
    if (enabled == ORG_KDE_KWIN_OUTPUTDEVICE_ENABLEMENT_ENABLED) {
        _enabled = OutputDevice::Enablement::Enabled;
    }
    if (o->enabled != _enabled) {
        o->enabled = _enabled;
        emit o->q->enabledChanged(o->enabled);
        if (o->done) {
            emit o->q->changed();
        }
    }
}

void OutputDevice::Private::uuidCallback(void* data, org_kde_kwin_outputdevice* output, const char *uuid)
{
    auto o = reinterpret_cast<OutputDevice::Private*>(data);
    if (o->uuid != uuid) {
        o->uuid = uuid;
        emit o->q->uuidChanged(o->uuid);
        if (o->done) {
            emit o->q->changed();
        }
    }
}

void OutputDevice::setup(org_kde_kwin_outputdevice *output)
{
    d->setup(output);
}

EventQueue *OutputDevice::eventQueue() const
{
    return d->queue;
}

void OutputDevice::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

void OutputDevice::Private::setGlobalPosition(const QPoint &pos)
{
    globalPosition = pos;
}

void OutputDevice::Private::setManufacturer(const QString &m)
{
    manufacturer = m;
}

void OutputDevice::Private::setModel(const QString &m)
{
    model = m;
}

void OutputDevice::Private::setPhysicalSize(const QSize &size)
{
    physicalSize = size;
}

void OutputDevice::Private::setScale(int s)
{
    scale = s;
}

QRect OutputDevice::geometry() const
{
    if (d->currentMode == d->modes.end()) {
        return QRect();
    }
    return QRect(d->globalPosition, pixelSize());
}

void OutputDevice::Private::setSubPixel(OutputDevice::SubPixel s)
{
    subPixel = s;
}

void OutputDevice::Private::setTransform(OutputDevice::Transform t)
{
    transform = t;
}

QPoint OutputDevice::globalPosition() const
{
    return d->globalPosition;
}

QString OutputDevice::manufacturer() const
{
    return d->manufacturer;
}

QString OutputDevice::model() const
{
    return d->model;
}

org_kde_kwin_outputdevice *OutputDevice::output()
{
    return d->output;
}

QSize OutputDevice::physicalSize() const
{
    return d->physicalSize;
}

QSize OutputDevice::pixelSize() const
{
    if (d->currentMode == d->modes.end()) {
        return QSize();
    }
    return (*d->currentMode).size;
}

int OutputDevice::refreshRate() const
{
    if (d->currentMode == d->modes.end()) {
        return 0;
    }
    return (*d->currentMode).refreshRate;
}

int OutputDevice::scale() const
{
    return d->scale;
}

bool OutputDevice::isValid() const
{
    return d->output.isValid();
}

OutputDevice::SubPixel OutputDevice::subPixel() const
{
    return d->subPixel;
}

OutputDevice::Transform OutputDevice::transform() const
{
    return d->transform;
}

QList< OutputDevice::Mode > OutputDevice::modes() const
{
    return d->modes;
}

OutputDevice::operator org_kde_kwin_outputdevice*() {
    return d->output;
}

OutputDevice::operator org_kde_kwin_outputdevice*() const {
    return d->output;
}

QByteArray OutputDevice::edid() const
{
    return d->edid;
}

OutputDevice::Enablement OutputDevice::enabled() const
{
    return d->enabled;
}

QByteArray OutputDevice::uuid() const
{
    return d->uuid;
}


}
}
