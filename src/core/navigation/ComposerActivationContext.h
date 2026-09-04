#pragma once

#include "core/image/ImageEntry.h"

#include <QList>
#include <QMetaType>

namespace purrview::core {

enum class ActivationSource { Viewer, DirectLaunch, FileOpen, FutureModule };

struct ComposerActivationContext {
    QList<ImageId> imageIds;
    ActivationSource source = ActivationSource::DirectLaunch;
};

} // namespace purrview::core

Q_DECLARE_METATYPE(purrview::core::ActivationSource)
Q_DECLARE_METATYPE(purrview::core::ComposerActivationContext)
