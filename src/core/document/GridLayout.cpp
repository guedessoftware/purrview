#include "core/document/GridLayout.h"

#include <algorithm>

namespace impage::core {

int GridLayout::rows() const {
    return rows_;
}

int GridLayout::columns() const {
    return columns_;
}

double GridLayout::horizontalSpacingMm() const {
    return horizontalSpacingMm_;
}

double GridLayout::verticalSpacingMm() const {
    return verticalSpacingMm_;
}

void GridLayout::setRows(int rows) {
    rows_ = rows;
}

void GridLayout::setColumns(int columns) {
    columns_ = columns;
}

void GridLayout::setHorizontalSpacingMm(double spacing) {
    horizontalSpacingMm_ = std::max(0.0, spacing);
}

void GridLayout::setVerticalSpacingMm(double spacing) {
    verticalSpacingMm_ = std::max(0.0, spacing);
}

} // namespace impage::core
