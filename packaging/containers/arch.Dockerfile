FROM archlinux:base-devel

RUN pacman -Syu --noconfirm \
    && pacman -S --needed --noconfirm \
        appstream \
        cmake \
        desktop-file-utils \
        exiv2 \
        kimageformats \
        ninja \
        qt6-base \
        qt6-declarative \
        qt6-imageformats \
        qt6-svg \
        qt6-wayland \
    && useradd --create-home --shell /bin/bash builder \
    && pacman -Scc --noconfirm

ENTRYPOINT ["/bin/bash", "/src/packaging/containers/build-arch.sh"]
