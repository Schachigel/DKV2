FROM ubuntu:24.04
ENV TZ=Europe/Berlin
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Install basics
RUN apt-get update
RUN apt-get install -y \
    binutils file fuse wget software-properties-common \
    git curl \
    cmake make g++ libglib2.0-0 \
    qt6-base-dev qt6-l10n-tools

# Install libraries
RUN apt-get install -y \
    icnsutils libxcb-icccm4-dev freetds-dev libsybdb5 \
    libsybdb5 libxcb-image0-dev libgl1-mesa-dev \
    libxcb-keysyms1-dev \
    libxcb-xinerama0 libzstd-dev libcurl4-openssl-dev \
    libodbc2
    # libxcb-icccm4 \
    # libxcb-image0 \

# Install linuxdeployqt
RUN wget \
    https://github.com/probonopd/linuxdeployqt/releases/download/10/linuxdeployqt-continuous-x86_64.AppImage \
    --quiet --output-document=/usr/bin/linuxdeployqt && \
    chmod +x /usr/bin/linuxdeployqt

ENV QML_IMPORT_PATH="/qt/qml:/app/DKV2"
RUN git config --global --add safe.directory /app
# Run build script
WORKDIR /app
CMD /app/dist_linux.sh
