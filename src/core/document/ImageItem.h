#pragma once

#include <QSize>
#include <QString>

namespace purrview::core {

enum class PlacementMode { Fit, Fill, Stretch };

struct ImageItem {
    QString source;
    QSize pixelSize;
    PlacementMode placementMode = PlacementMode::Fit;
    int rotationDegrees = 0;
};

} // namespace purrview::core
