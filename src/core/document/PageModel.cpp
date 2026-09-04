#include "core/document/PageModel.h"

#include <algorithm>

namespace purrview::core {

double PageModel::widthMm() const {
    return orientation_ == Orientation::Portrait ? portraitWidthMm() : portraitHeightMm();
}

double PageModel::heightMm() const {
    return orientation_ == Orientation::Portrait ? portraitHeightMm() : portraitWidthMm();
}

PageModel::Orientation PageModel::orientation() const {
    return orientation_;
}

PageModel::PaperSize PageModel::paperSize() const {
    return paperSize_;
}

double PageModel::marginTopMm() const {
    return marginTopMm_;
}

double PageModel::marginBottomMm() const {
    return marginBottomMm_;
}

double PageModel::marginLeftMm() const {
    return marginLeftMm_;
}

double PageModel::marginRightMm() const {
    return marginRightMm_;
}

void PageModel::setOrientation(Orientation orientation) {
    orientation_ = orientation;
}

void PageModel::setPaperSize(PaperSize paperSize) {
    paperSize_ = paperSize;
}

double PageModel::portraitWidthMm() const {
    switch (paperSize_) {
    case PaperSize::A4:
        return 210.0;
    case PaperSize::A3:
        return 297.0;
    case PaperSize::A5:
        return 148.0;
    case PaperSize::Letter:
    case PaperSize::Legal:
        return 215.9;
    case PaperSize::Photo10x15:
        return 100.0;
    }
    return 210.0;
}

double PageModel::portraitHeightMm() const {
    switch (paperSize_) {
    case PaperSize::A4:
        return 297.0;
    case PaperSize::A3:
        return 420.0;
    case PaperSize::A5:
        return 210.0;
    case PaperSize::Letter:
        return 279.4;
    case PaperSize::Legal:
        return 355.6;
    case PaperSize::Photo10x15:
        return 150.0;
    }
    return 297.0;
}

void PageModel::setMarginsMm(double top, double right, double bottom, double left) {
    marginTopMm_ = std::max(0.0, top);
    marginRightMm_ = std::max(0.0, right);
    marginBottomMm_ = std::max(0.0, bottom);
    marginLeftMm_ = std::max(0.0, left);
}

} // namespace purrview::core
