#include "composer/ComposerSessionAdapter.h"

#include <algorithm>
#include <utility>

namespace impage::composer {

ComposerSessionAdapter::ComposerSessionAdapter(core::ImageSession& session,
                                               core::DocumentModel& document, QObject* parent)
    : QObject(parent), session_(session), document_(document) {
    connect(&session_, &QAbstractItemModel::rowsInserted, this, [this] { synchronize(); });
    connect(&session_, &QAbstractItemModel::rowsRemoved, this, [this] { synchronize(); });
    connect(&session_, &QAbstractItemModel::modelReset, this, [this] { synchronize(); });
    connect(&session_, &QAbstractItemModel::dataChanged, this,
            [this](const QModelIndex&, const QModelIndex&, const QList<int>& roles) {
                if (roles.isEmpty() || roles.contains(core::ImageSession::RotationRole)) {
                    synchronize();
                }
            });
    connect(&session_, &core::ImageSession::selectionChanged, this, [this] { synchronize(); });
    synchronize();
}

void ComposerSessionAdapter::useCompleteSession() {
    usesExplicitImages_ = false;
    explicitImageIds_.clear();
    synchronize();
}

void ComposerSessionAdapter::useExplicitImages(const QList<core::ImageId>& imageIds) {
    usesExplicitImages_ = true;
    explicitImageIds_ = imageIds;
    synchronize();
}

void ComposerSessionAdapter::freezeCurrentImages() {
    if (usesExplicitImages_) {
        return;
    }
    explicitImageIds_.clear();
    explicitImageIds_.reserve(session_.count());
    for (const core::ImageEntry& image : session_.images()) {
        explicitImageIds_.push_back(image.id);
    }
    usesExplicitImages_ = true;
}

bool ComposerSessionAdapter::usesExplicitImages() const {
    return usesExplicitImages_;
}

QList<core::ImageId> ComposerSessionAdapter::explicitImageIds() const {
    return explicitImageIds_;
}

void ComposerSessionAdapter::synchronize() {
    document_.clearImages();
    if (usesExplicitImages_) {
        QList<core::ImageId> availableImageIds;
        availableImageIds.reserve(explicitImageIds_.size());
        for (const core::ImageId& imageId : explicitImageIds_) {
            const auto entry = std::find_if(
                session_.images().cbegin(), session_.images().cend(),
                [&imageId](const core::ImageEntry& candidate) { return candidate.id == imageId; });
            if (entry != session_.images().cend()) {
                availableImageIds.push_back(imageId);
                document_.addImage({.source = entry->sourcePath,
                                    .pixelSize = entry->pixelSize,
                                    .rotationDegrees = entry->rotationDegrees});
            }
        }
        explicitImageIds_ = std::move(availableImageIds);
    } else {
        for (const core::ImageEntry& entry : session_.images()) {
            document_.addImage({.source = entry.sourcePath,
                                .pixelSize = entry.pixelSize,
                                .rotationDegrees = entry.rotationDegrees});
        }
    }
    emit documentImagesChanged();
}

} // namespace impage::composer
