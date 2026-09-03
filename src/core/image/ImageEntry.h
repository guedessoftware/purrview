#pragma once

#include <QSize>
#include <QString>
#include <QUuid>

namespace impage::core {

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

} // namespace impage::core
