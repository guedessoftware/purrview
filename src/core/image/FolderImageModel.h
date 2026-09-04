#pragma once

#include "core/image/ImageSession.h"
#include "core/image/ThumbnailCache.h"

#include <QAbstractListModel>
#include <QFileSystemWatcher>
#include <QHash>
#include <QSet>
#include <QSize>
#include <QStringList>
#include <QThreadPool>
#include <QTimer>
#include <QUrl>

#include <vector>

namespace purrview::core {

class FolderImageModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int currentIndex READ currentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QUrl directoryUrl READ directoryUrl NOTIFY directoryChanged)
    Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)
    Q_PROPERTY(int selectedCount READ selectedCount NOTIFY selectedCountChanged)

  public:
    enum Role {
        FileNameRole = Qt::UserRole + 1,
        FilePathRole,
        SourceUrlRole,
        ThumbnailUrlRole,
        CurrentRole,
        SelectedRole,
        ValidRole,
        WidthRole,
        HeightRole
    };
    Q_ENUM(Role)

    explicit FolderImageModel(ImageSession& session, ThumbnailCache& thumbnailCache,
                              QObject* parent = nullptr);
    ~FolderImageModel() override;

    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] int count() const;
    [[nodiscard]] int currentIndex() const;
    [[nodiscard]] QUrl directoryUrl() const;
    [[nodiscard]] bool scanning() const;
    [[nodiscard]] int selectedCount() const;
    [[nodiscard]] QStringList selectedPaths() const;
    [[nodiscard]] QString pathAt(int index) const;
    [[nodiscard]] QUrl sourceAt(int index) const;
    [[nodiscard]] int indexOfPath(const QString& path) const;

    void openFromImage(const QString& imagePath);
    [[nodiscard]] bool scanFromImageSynchronously(const QString& imagePath);
    void refresh();
    [[nodiscard]] bool refreshSynchronously();
    void setWatchingEnabled(bool enabled);

    Q_INVOKABLE void requestThumbnail(int index);
    Q_INVOKABLE void requestAround(int centerIndex, int radius = 10);

  signals:
    void countChanged();
    void currentIndexChanged();
    void directoryChanged();
    void scanningChanged();
    void selectedCountChanged();
    void scanCompleted(const QString& requestedCurrentPath, bool currentPresent,
                       int previousCurrentIndex);

  private:
    struct FolderItem {
        QString fileName;
        QString filePath;
        QString thumbnailKey;
        QSize pixelSize;
        bool thumbnailRequested = false;
        bool valid = true;
    };

    [[nodiscard]] static std::vector<FolderItem> enumerate(const QString& directoryPath);
    void beginScan(const QString& directoryPath, const QString& requestedCurrentPath);
    void applyScan(const QString& directoryPath, const QString& requestedCurrentPath,
                   int previousCurrentIndex, quint64 generation, std::vector<FolderItem> items);
    void updateSessionCurrentRole();
    void updateWatcher();
    void rebuildPathIndex();
    void refreshSelectionCache();
    [[nodiscard]] bool isPathSelected(const QString& path) const;

    ImageSession& session_;
    ThumbnailCache& thumbnailCache_;
    std::vector<FolderItem> items_;
    QHash<QString, int> pathIndex_;
    QSet<QString> selectedPaths_;
    QString directoryPath_;
    int lastCurrentIndex_ = -1;
    bool scanning_ = false;
    bool watchingEnabled_ = true;
    quint64 scanGeneration_ = 0;
    QThreadPool scanPool_;
    QFileSystemWatcher watcher_;
    QTimer refreshTimer_;
};

} // namespace purrview::core
