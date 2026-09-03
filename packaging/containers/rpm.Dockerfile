FROM almalinux:9

RUN dnf -y -q install dnf-plugins-core epel-release \
    && dnf config-manager --set-enabled crb \
    && dnf -y -q install \
        appstream \
        cmake \
        cups-devel \
        desktop-file-utils \
        file \
        gcc-c++ \
        libxkbcommon-devel \
        ninja-build \
        rpm-build \
        qt6-qtbase-devel \
        qt6-qtdeclarative-devel \
        qt6-qtimageformats \
        qt6-qtsvg \
        qt6-qtwayland \
        xorg-x11-server-Xvfb \
        xorg-x11-xauth \
    && dnf clean all

ENTRYPOINT ["/bin/bash", "/src/packaging/containers/build-rpm.sh"]
