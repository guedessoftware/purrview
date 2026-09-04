#pragma once

#include "core/image/ImageEntry.h"

#include <QAbstractListModel>
#include <QUrl>

#include <optional>
#include <vector>

namespace purrview::core {

// ImageSession is intentionally confined to the application's UI thread.
class ImageSession : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentImageChanged)
    Q_PROPERTY(QString currentImageId READ currentImageId NOTIFY currentImageChanged)
    Q_PROPERTY(QUrl currentImageSource READ currentImageSource NOTIFY currentImageChanged)
    Q_PROPERTY(QUrl sourceFolder READ sourceFolder NOTIFY sourceFolderChanged)
    Q_PROPERTY(int selectedCount READ selectedCount NOTIFY selectionChanged)

  public:
    enum Role {
        IdRole = Qt::UserRole + 1,
        SourceRole,
        FileNameRole,
        SelectedRole,
        CurrentRole,
        WidthRole,
        HeightRole,
        RotationRole,
        ValidRole
    };
    Q_ENUM(Role)

    explicit ImageSession(QObject* parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] int count() const;
    [[nodiscard]] int currentIndex() const;
    [[nodiscard]] QString currentImageId() const;
    [[nodiscard]] QUrl currentImageSource() const;
    [[nodiscard]] QUrl sourceFolder() const;
    [[nodiscard]] int selectedCount() const;
    [[nodiscard]] const std::vector<ImageEntry>& images() const;
    [[nodiscard]] std::vector<ImageEntry> selectedImages() const;
    [[nodiscard]] const ImageEntry* currentImage() const;

    [[nodiscard]] std::optional<ImageId> addImage(const QString& path, QString* error = nullptr);
    [[nodiscard]] std::optional<ImageId> addImageReference(const QString& path,
                                                            QString* error = nullptr);
    [[nodiscard]] QList<ImageId> addImageReferences(const QStringList& paths,
                                                    bool inspectImages = false,
                                                    QStringList* errors = nullptr);
    [[nodiscard]] QList<ImageId> addImages(const QStringList& paths, QStringList* errors = nullptr);
    [[nodiscard]] bool removeImage(const ImageId& id);
    Q_INVOKABLE bool removeImageById(const QString& id);
    Q_INVOKABLE void clear();

    void setCurrentIndex(int index);
    [[nodiscard]] bool setCurrentImage(const ImageId& id);
    Q_INVOKABLE bool setCurrentImageById(const QString& id);

    [[nodiscard]] bool selectImage(const ImageId& id);
    [[nodiscard]] bool deselectImage(const ImageId& id);
    [[nodiscard]] bool toggleSelection(const ImageId& id);
    [[nodiscard]] bool selectImages(const QList<ImageId>& ids);
    [[nodiscard]] bool replaceSelection(const QList<ImageId>& ids);
    Q_INVOKABLE bool toggleSelectionById(const QString& id);
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE void selectAll();
    [[nodiscard]] bool setRotation(const ImageId& id, int degrees);

  signals:
    void countChanged();
    void currentImageChanged();
    void selectionChanged();
    void sourceFolderChanged();
    void sessionCleared();

  private:
    [[nodiscard]] static std::optional<ImageEntry> validateImage(const QString& path,
                                                                 QString* error);
    [[nodiscard]] int indexOf(const ImageId& id) const;
    [[nodiscard]] bool setSelected(const ImageId& id, bool selected);
    void emitCurrentRowsChanged(int previousIndex, int nextIndex);
    void recomputeSourceFolder();

    std::vector<ImageEntry> images_;
    int currentIndex_ = -1;
    QString sourceFolderPath_;
};

} // namespace purrview::core
