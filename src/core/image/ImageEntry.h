#pragma once

#include <QSize>
#include <QString>
#include <QUuid>

namespace purrview::core {

using ImageId = QUuid;

struct ImageEntry {
    ImageId id;
    QString sourcePath;
    QString fileName;
    QSize pixelSize;
    int rotationDegrees = 0;
    bool selected = false;
    bool valid = true;
};

} // namespace purrview::core
