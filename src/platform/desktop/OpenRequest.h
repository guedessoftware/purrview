#pragma once

#include <QByteArray>
#include <QMetaType>
#include <QStringList>
#include <QVariantList>

namespace purrview::desktop {

inline constexpr qsizetype MaximumOpenRequestFiles = 4096;
inline constexpr qsizetype MaximumOpenRequestPathLength = qsizetype{32} * 1024;

enum class OpenMode { Auto, Viewer, Composer };

struct OpenRequest {
    OpenMode mode = OpenMode::Auto;
    QStringList files;
    bool activateWindow = true;
};

[[nodiscard]] QByteArray serializeOpenRequest(const OpenRequest& request);
[[nodiscard]] bool deserializeOpenRequest(const QByteArray& payload, OpenRequest* request,
                                          QString* error = nullptr);
[[nodiscard]] QStringList normalizeFileArguments(const QStringList& arguments,
                                                 const QString& workingDirectory);
[[nodiscard]] QVariantList fileUrls(const QStringList& paths);

} // namespace purrview::desktop

Q_DECLARE_METATYPE(purrview::desktop::OpenRequest)
