#pragma once

#include <QRectF>
#include <QString>

#include <vector>

namespace impage::core {

struct PageLayout {
    double pageWidthMm = 0.0;
    double pageHeightMm = 0.0;
    std::vector<QRectF> cellsMm;
    QString error;

    [[nodiscard]] bool isValid() const {
        return error.isEmpty();
    }
};

} // namespace impage::core
