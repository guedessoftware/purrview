#pragma once

#include <QSet>
#include <QString>

namespace impage::core {

[[nodiscard]] const QSet<QString>& supportedImageSuffixes();
[[nodiscard]] bool isSupportedImageFile(const QString& path);

} // namespace impage::core
