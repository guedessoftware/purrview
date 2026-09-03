FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        appstream \
        build-essential \
        ca-certificates \
        cmake \
        desktop-file-utils \
        dpkg-dev \
        file \
        libgl1-mesa-dev \
        libqt6svg6 \
        libxkbcommon-dev \
        ninja-build \
        qml6-module-qtquick \
        qml6-module-qtquick-controls \
        qml6-module-qtquick-dialogs \
        qml6-module-qtquick-layouts \
        qml6-module-qtquick-templates \
        qml6-module-qtquick-window \
        qml6-module-qtqml \
        qml6-module-qtqml-models \
        qml6-module-qtqml-workerscript \
        qt6-base-dev \
        qt6-base-dev-tools \
        qt6-declarative-dev \
        qt6-declarative-dev-tools \
        qt6-image-formats-plugins \
    && rm -rf /var/lib/apt/lists/*

ENTRYPOINT ["/bin/bash", "/src/packaging/containers/build-deb.sh"]
