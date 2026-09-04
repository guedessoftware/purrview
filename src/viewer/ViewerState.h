#pragma once

#include <QObject>
#include <QString>
#include <QTimer>

namespace purrview::viewer {

class ViewerState : public QObject {
    Q_OBJECT
    Q_PROPERTY(ZoomMode zoomMode READ zoomMode NOTIFY zoomChanged)
    Q_PROPERTY(double zoomFactor READ zoomFactor NOTIFY zoomChanged)
    Q_PROPERTY(bool fitMode READ fitMode NOTIFY zoomChanged)
    Q_PROPERTY(bool filmstripVisible READ filmstripVisible WRITE setFilmstripVisible NOTIFY
                   filmstripVisibleChanged)
    Q_PROPERTY(bool infoPanelVisible READ infoPanelVisible WRITE setInfoPanelVisible NOTIFY
                   infoPanelVisibleChanged)
    Q_PROPERTY(bool toolbarPinned READ toolbarPinned WRITE setToolbarPinned NOTIFY
                   toolbarPinnedChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorChanged)
    Q_PROPERTY(bool fullScreen READ fullScreen WRITE setFullScreen NOTIFY fullScreenChanged)
    Q_PROPERTY(bool controlsVisible READ controlsVisible NOTIFY interactionChanged)
    Q_PROPERTY(bool cursorVisible READ cursorVisible NOTIFY interactionChanged)
    Q_PROPERTY(bool interactionBlocked READ interactionBlocked WRITE setInteractionBlocked NOTIFY
                   interactionChanged)

  public:
    enum class ZoomMode { Fit, ActualSize, Custom };
    Q_ENUM(ZoomMode)

    explicit ViewerState(QObject* parent = nullptr);

    [[nodiscard]] ZoomMode zoomMode() const;
    [[nodiscard]] double zoomFactor() const;
    [[nodiscard]] bool fitMode() const;
    [[nodiscard]] bool filmstripVisible() const;
    [[nodiscard]] bool infoPanelVisible() const;
    [[nodiscard]] bool toolbarPinned() const;
    [[nodiscard]] QString errorString() const;
    [[nodiscard]] bool fullScreen() const;
    [[nodiscard]] bool controlsVisible() const;
    [[nodiscard]] bool cursorVisible() const;
    [[nodiscard]] bool interactionBlocked() const;

    void updateFitScale(double scale);
    void setCustomZoom(double scale);
    void zoomIn();
    void zoomOut();
    void fitToWindow();
    void actualSize();
    void resetForImage();
    void requestPanReset();
    void setError(const QString& error);
    void clearError();
    void setFilmstripVisible(bool visible);
    void toggleFilmstrip();
    void setInfoPanelVisible(bool visible);
    void toggleInfoPanel();
    Q_INVOKABLE void setToolbarPinned(bool pinned);
    Q_INVOKABLE void toggleToolbarPinned();
    void restoreZoom(ZoomMode mode, double factor);
    Q_INVOKABLE void setFullScreen(bool fullScreen);
    Q_INVOKABLE void setInteractionBlocked(bool blocked);
    Q_INVOKABLE void notifyActivity();
    void setInactivityIntervalForTesting(int milliseconds);

  signals:
    void zoomChanged();
    void panResetRequested();
    void errorChanged();
    void filmstripVisibleChanged();
    void infoPanelVisibleChanged();
    void toolbarPinnedChanged();
    void fullScreenChanged();
    void interactionChanged();

  private:
    static constexpr double MinimumCustomZoom = 0.05;
    static constexpr double MaximumCustomZoom = 8.0;
    static constexpr double ZoomStep = 1.25;

    void setZoom(ZoomMode mode, double factor);

    ZoomMode zoomMode_ = ZoomMode::Fit;
    double zoomFactor_ = 1.0;
    QString errorString_;
    bool filmstripVisible_ = true;
    bool infoPanelVisible_ = false;
    bool toolbarPinned_ = false;
    bool fullScreen_ = false;
    bool controlsVisible_ = true;
    bool cursorVisible_ = true;
    bool interactionBlocked_ = false;
    QTimer inactivityTimer_;
};

} // namespace purrview::viewer
