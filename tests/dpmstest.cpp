/********************************************************************
Copyright 2015  Martin Gräßlin <mgraesslin@kde.org>

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
#include <QApplication>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include "../src/client/connection_thread.h"
#include "../src/client/dpms.h"
#include "../src/client/registry.h"
#include "../src/client/output.h"

using namespace KWayland::Client;

static QString modeToString(Dpms::Mode mode)
{
    switch (mode) {
    case Dpms::Mode::On:
        return QStringLiteral("On");
    case Dpms::Mode::Standby:
        return QStringLiteral("Standby");
    case Dpms::Mode::Suspend:
        return QStringLiteral("Suspend");
    case Dpms::Mode::Off:
        return QStringLiteral("Off");
    default:
        Q_UNREACHABLE();
    }
}

QString supportedToString(bool supported)
{
    return supported ? QStringLiteral("Yes") : QStringLiteral("No");
}

static QLayout *setupOutput(Registry::AnnouncedInterface outputInterface, Registry *registry, DpmsManager *manager)
{
    Output *output = registry->createOutput(outputInterface.name, outputInterface.version, registry);
    QLabel *label = new QLabel(output->model());
    QObject::connect(output, &Output::changed, label,
        [label, output] {
            label->setText(output->model());
        }, Qt::QueuedConnection
    );

    Dpms *dpms = nullptr;
    if (manager) {
        dpms = manager->getDpms(output, output);
    }

    QFormLayout *dpmsForm = new QFormLayout;
    bool supported = dpms ? dpms->isSupported() : false;
    QLabel *supportedLabel = new QLabel(supportedToString(supported));
    dpmsForm->addRow(QStringLiteral("Supported:"), supportedLabel);
    QLabel *modeLabel = new QLabel(modeToString(dpms ? dpms->mode() : Dpms::Mode::On));
    dpmsForm->addRow(QStringLiteral("Mode:"), modeLabel);

    QPushButton *standbyButton = new QPushButton(QStringLiteral("Standby"));
    QPushButton *suspendButton = new QPushButton(QStringLiteral("Suspend"));
    QPushButton *offButton = new QPushButton(QStringLiteral("Off"));
    standbyButton->setEnabled(supported);
    suspendButton->setEnabled(supported);
    offButton->setEnabled(supported);
    QDialogButtonBox *bg = new QDialogButtonBox;
    bg->addButton(standbyButton, QDialogButtonBox::ActionRole);
    bg->addButton(suspendButton, QDialogButtonBox::ActionRole);
    bg->addButton(offButton, QDialogButtonBox::ActionRole);

    if (dpms) {
        QObject::connect(dpms, &Dpms::supportedChanged, supportedLabel,
            [supportedLabel, dpms, standbyButton, suspendButton, offButton] {
                const bool supported = dpms->isSupported();
                supportedLabel->setText(supportedToString(supported));
                standbyButton->setEnabled(supported);
                suspendButton->setEnabled(supported);
                offButton->setEnabled(supported);
            }, Qt::QueuedConnection
        );
        QObject::connect(dpms, &Dpms::modeChanged, modeLabel,
            [modeLabel, dpms] {
                modeLabel->setText(modeToString(dpms->mode()));
            }, Qt::QueuedConnection
        );
        QObject::connect(standbyButton, &QPushButton::clicked, dpms, [dpms] { dpms->requestMode(Dpms::Mode::Standby);});
        QObject::connect(suspendButton, &QPushButton::clicked, dpms, [dpms] { dpms->requestMode(Dpms::Mode::Suspend);});
        QObject::connect(offButton, &QPushButton::clicked, dpms, [dpms] { dpms->requestMode(Dpms::Mode::Off);});
    }

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    layout->addLayout(dpmsForm);
    layout->addWidget(bg);
    return layout;
}

int main(int argc, char **argv)
{
    qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("wayland"));
    QApplication app(argc, argv);

    QWidget window;

    ConnectionThread *connection = ConnectionThread::fromApplication();
    Registry registry;
    registry.create(connection);
    QObject::connect(&registry, &Registry::interfacesAnnounced, &app,
        [&registry, &window] {
            const bool hasDpms = registry.hasInterface(Registry::Interface::Dpms);
            QLabel *hasDpmsLabel = new QLabel(&window);
            if (hasDpms) {
                hasDpmsLabel->setText(QStringLiteral("Compositor provides a DpmsManager"));
            } else {
                hasDpmsLabel->setText(QStringLiteral("Compositor does not provid a DpmsManager"));
            }

            QVBoxLayout *layout = new QVBoxLayout;
            layout->addWidget(hasDpmsLabel);
            QFrame *hline = new QFrame;
            hline->setFrameShape(QFrame::HLine);
            layout->addWidget(hline);

            DpmsManager *dpmsManager = nullptr;
            if (hasDpms) {
                const auto dpmsData = registry.interface(Registry::Interface::Dpms);
                dpmsManager = registry.createDpmsManager(dpmsData.name, dpmsData.version);
            }

            // get all Outputs
            const auto outputs = registry.interfaces(Registry::Interface::Output);
            for (auto o : outputs) {
                layout->addLayout(setupOutput(o, &registry, dpmsManager));
                QFrame *hline = new QFrame;
                hline->setFrameShape(QFrame::HLine);
                layout->addWidget(hline);
            }

            window.setLayout(layout);
            window.show();
        }, Qt::QueuedConnection
    );
    registry.setup();

    return app.exec();
}
