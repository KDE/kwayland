
#include "outputchangeset.h"

namespace KWayland
{
    namespace Server
    {

        class OutputChangeSet::Private
        {
        public:
            Private(OutputDeviceInterface *outputdevice, OutputChangeSet *parent);
            ~Private();

            OutputChangeSet *q;
            OutputDeviceInterface *o;

            OutputDeviceInterface::Enablement enabled;
            int modeId;
            OutputDeviceInterface::Transform transform;
            QPoint position;
            int scale;
        };
    }
}