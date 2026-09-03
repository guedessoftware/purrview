#pragma once

#include "core/image/ImageEntry.h"

#include <QList>
#include <QMetaType>

namespace impage::core {

enum class ActivationSource { Viewer, DirectLaunch, FileOpen, FutureModule };

struct ComposerActivationContext {
    QList<ImageId> imageIds;
    ActivationSource source = ActivationSource::DirectLaunch;
};

} // namespace impage::core

Q_DECLARE_METATYPE(impage::core::ActivationSource)
Q_DECLARE_METATYPE(impage::core::ComposerActivationContext)
