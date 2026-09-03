#pragma once

#include "core/image/ImageEntry.h"

#include <QList>
#include <QPointF>

namespace impage::viewer {

struct ViewerNavigationState {
    core::ImageId currentImageId;
    QList<core::ImageId> selectedImageIds;
    double zoomFactor = 1.0;
    int zoomMode = 0;
    QPointF panOffset;
    int rotationDegrees = 0;
    bool infoPanelVisible = false;
    bool filmstripVisible = true;
    int filmstripCurrentIndex = -1;
    double filmstripContentX = 0.0;
};

} // namespace impage::viewer
