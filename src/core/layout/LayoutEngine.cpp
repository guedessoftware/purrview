#include "core/layout/LayoutEngine.h"

namespace purrview::core {

PageLayout LayoutEngine::calculate(const DocumentModel& document) const {
    const PageModel& page = document.page();
    const GridLayout& grid = document.grid();

    PageLayout result;
    result.pageWidthMm = page.widthMm();
    result.pageHeightMm = page.heightMm();

    if (grid.rows() <= 0 || grid.columns() <= 0) {
        result.error = QStringLiteral("A grade precisa ter ao menos uma linha e uma coluna.");
        return result;
    }

    const double availableWidth = page.widthMm() - page.marginLeftMm() - page.marginRightMm() -
                                  (grid.columns() - 1) * grid.horizontalSpacingMm();
    const double availableHeight = page.heightMm() - page.marginTopMm() - page.marginBottomMm() -
                                   (grid.rows() - 1) * grid.verticalSpacingMm();

    if (availableWidth <= 0.0 || availableHeight <= 0.0) {
        result.error = QStringLiteral("Margens e espaçamentos não deixam área útil na página.");
        return result;
    }

    const double cellWidth = availableWidth / grid.columns();
    const double cellHeight = availableHeight / grid.rows();
    result.cellsMm.reserve(static_cast<std::size_t>(grid.rows() * grid.columns()));

    for (int row = 0; row < grid.rows(); ++row) {
        for (int column = 0; column < grid.columns(); ++column) {
            const double x =
                page.marginLeftMm() + column * (cellWidth + grid.horizontalSpacingMm());
            const double y = page.marginTopMm() + row * (cellHeight + grid.verticalSpacingMm());
            result.cellsMm.emplace_back(x, y, cellWidth, cellHeight);
        }
    }

    return result;
}

} // namespace purrview::core
