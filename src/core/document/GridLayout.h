#pragma once

namespace impage::core {

class GridLayout {
  public:
    [[nodiscard]] int rows() const;
    [[nodiscard]] int columns() const;
    [[nodiscard]] double horizontalSpacingMm() const;
    [[nodiscard]] double verticalSpacingMm() const;

    void setRows(int rows);
    void setColumns(int columns);
    void setHorizontalSpacingMm(double spacing);
    void setVerticalSpacingMm(double spacing);

  private:
    int rows_ = 1;
    int columns_ = 1;
    double horizontalSpacingMm_ = 4.0;
    double verticalSpacingMm_ = 4.0;
};

} // namespace impage::core
