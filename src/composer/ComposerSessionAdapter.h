#pragma once

#include "core/document/DocumentModel.h"
#include "core/image/ImageSession.h"

#include <QList>
#include <QObject>

namespace impage::composer {

class ComposerSessionAdapter : public QObject {
    Q_OBJECT

  public:
    ComposerSessionAdapter(core::ImageSession& session, core::DocumentModel& document,
                           QObject* parent = nullptr);

    void useCompleteSession();
    void useExplicitImages(const QList<core::ImageId>& imageIds);
    void freezeCurrentImages();
    [[nodiscard]] bool usesExplicitImages() const;
    [[nodiscard]] QList<core::ImageId> explicitImageIds() const;
    void synchronize();

  signals:
    void documentImagesChanged();

  private:
    core::ImageSession& session_;
    core::DocumentModel& document_;
    QList<core::ImageId> explicitImageIds_;
    bool usesExplicitImages_ = false;
};

} // namespace impage::composer
