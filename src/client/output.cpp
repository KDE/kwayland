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
#include "output.h"
// Qt
#include <QPoint>
#include <QRect>
#include <QSize>
// wayland
#include <wayland-client-protocol.h>

namespace KWayland
{

namespace Client
{

class Output::Private
{
public:
    Private(Output *q);
    void setup(wl_output *o);

    wl_output *output = nullptr;
    QSize physicalSize;
    QPoint globalPosition;
    QString manufacturer;
    QString model;
    QSize pixelSize;
    int refreshRate = 0;
    int scale = 1;
    SubPixel subPixel = SubPixel::Unknown;
    Transform transform = Transform::Normal;

private:
    static void geometryCallback(void *data, wl_output *output, int32_t x, int32_t y,
                                 int32_t physicalWidth, int32_t physicalHeight, int32_t subPixel,
                                 const char *make, const char *model, int32_t transform);
    static void modeCallback(void *data, wl_output *output, uint32_t flags, int32_t width, int32_t height, int32_t refresh);
    static void doneCallback(void *data, wl_output *output);
    static void scaleCallback(void *data, wl_output *output, int32_t scale);
    void setPhysicalSize(const QSize &size);
    void setGlobalPosition(const QPoint &pos);
    void setManufacturer(const QString &manufacturer);
    void setModel(const QString &model);
    void setPixelSize(const QSize &size);
    void setRefreshRate(int refreshRate);
    void setScale(int scale);
    void setSubPixel(SubPixel subPixel);
    void setTransform(Transform transform);

    Output *q;
    static struct wl_output_listener s_outputListener;
};

Output::Private::Private(Output *q)
    : q(q)
{
}

void Output::Private::setup(wl_output *o)
{
    Q_ASSERT(o);
    Q_ASSERT(!output);
    output = o;
    wl_output_add_listener(output, &s_outputListener, this);
}

Output::Output(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

Output::~Output()
{
    if (d->output) {
        wl_output_destroy(d->output);
    }
}

wl_output_listener Output::Private::s_outputListener = {
    geometryCallback,
    modeCallback,
    doneCallback,
    scaleCallback
};

void Output::Private::geometryCallback(void *data, wl_output *output,
                              int32_t x, int32_t y,
                              int32_t physicalWidth, int32_t physicalHeight,
                              int32_t subPixel, const char *make, const char *model, int32_t transform)
{
    Q_UNUSED(transform)
    auto o = reinterpret_cast<Output::Private*>(data);
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

void Output::Private::modeCallback(void *data, wl_output *output, uint32_t flags, int32_t width, int32_t height, int32_t refresh)
{
    if (!(flags & WL_OUTPUT_MODE_CURRENT)) {
        // ignore all non-current modes;
        return;
    }
    auto o = reinterpret_cast<Output::Private*>(data);
    Q_ASSERT(o->output == output);
    o->setPixelSize(QSize(width, height));
    o->setRefreshRate(refresh);
}

void Output::Private::scaleCallback(void *data, wl_output *output, int32_t scale)
{
    auto o = reinterpret_cast<Output::Private*>(data);
    Q_ASSERT(o->output == output);
    o->setScale(scale);
}

void Output::Private::doneCallback(void *data, wl_output *output)
{
    auto o = reinterpret_cast<Output::Private*>(data);
    Q_ASSERT(o->output == output);
    emit o->q->changed();
}

void Output::setup(wl_output *output)
{
    d->setup(output);
}

void Output::Private::setGlobalPosition(const QPoint &pos)
{
    globalPosition = pos;
}

void Output::Private::setManufacturer(const QString &m)
{
    manufacturer = m;
}

void Output::Private::setModel(const QString &m)
{
    model = m;
}

void Output::Private::setPhysicalSize(const QSize &size)
{
    physicalSize = size;
}

void Output::Private::setPixelSize(const QSize& size)
{
    pixelSize = size;
}

void Output::Private::setRefreshRate(int r)
{
    refreshRate = r;
}

void Output::Private::setScale(int s)
{
    scale = s;
}

QRect Output::geometry() const
{
    if (!d->pixelSize.isValid()) {
        return QRect();
    }
    return QRect(d->globalPosition, d->pixelSize);
}

void Output::Private::setSubPixel(Output::SubPixel s)
{
    subPixel = s;
}

void Output::Private::setTransform(Output::Transform t)
{
    transform = t;
}

QPoint Output::globalPosition() const
{
    return d->globalPosition;
}

QString Output::manufacturer() const
{
    return d->manufacturer;
}

QString Output::model() const
{
    return d->model;
}

wl_output *Output::output()
{
    return d->output;
}

QSize Output::physicalSize() const
{
    return d->physicalSize;
}

QSize Output::pixelSize() const
{
    return d->pixelSize;
}

int Output::refreshRate() const
{
    return d->refreshRate;
}

int Output::scale() const
{
    return d->scale;
}

bool Output::isValid() const
{
    return d->output != nullptr;
}

Output::SubPixel Output::subPixel() const
{
    return d->subPixel;
}

Output::Transform Output::transform() const
{
    return d->transform;
}

Output::operator wl_output*() {
    return d->output;
}

Output::operator wl_output*() const {
    return d->output;
}

}
}
