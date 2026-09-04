#pragma once

namespace purrview::core {

class PageModel {
  public:
    enum class Orientation { Portrait, Landscape };
    enum class PaperSize { A4, A3, A5, Letter, Legal, Photo10x15 };

    [[nodiscard]] double widthMm() const;
    [[nodiscard]] double heightMm() const;
    [[nodiscard]] Orientation orientation() const;
    [[nodiscard]] PaperSize paperSize() const;
    [[nodiscard]] double marginTopMm() const;
    [[nodiscard]] double marginBottomMm() const;
    [[nodiscard]] double marginLeftMm() const;
    [[nodiscard]] double marginRightMm() const;

    void setOrientation(Orientation orientation);
    void setPaperSize(PaperSize paperSize);
    void setMarginsMm(double top, double right, double bottom, double left);

  private:
    [[nodiscard]] double portraitWidthMm() const;
    [[nodiscard]] double portraitHeightMm() const;

    Orientation orientation_ = Orientation::Portrait;
    PaperSize paperSize_ = PaperSize::A4;
    double marginTopMm_ = 10.0;
    double marginBottomMm_ = 10.0;
    double marginLeftMm_ = 10.0;
    double marginRightMm_ = 10.0;
};

} // namespace purrview::core
