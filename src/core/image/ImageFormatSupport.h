#pragma once

#include <QSet>
#include <QSize>
#include <QString>
#include <QtGlobal>

namespace purrview::core {

// QImage commonly expands decoded content to at least 32 bits per pixel. Keep a
// process-wide ceiling so malformed or unexpectedly large files cannot exhaust
// memory before the UI has a chance to recover.
inline constexpr int MaximumImageAllocationMiB = 256;
inline constexpr qint64 MaximumImagePixels = 64LL * 1024LL * 1024LL;
inline constexpr qint64 MaximumClipboardPixels = 32LL * 1024LL * 1024LL;

[[nodiscard]] const QSet<QString>& supportedImageSuffixes();
[[nodiscard]] bool isSupportedImageFile(const QString& path);
[[nodiscard]] bool isImageSizeWithinLimits(const QSize& size,
                                           qint64 maximumPixels = MaximumImagePixels);

} // namespace purrview::core
