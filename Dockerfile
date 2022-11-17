FROM ubuntu:20.04
ENV TZ=Europe/Berlin
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
# FROM mhitza/linuxdeployqt:5.11.3

# Install basics
RUN apt update && \
      apt install --assume-yes wget fuse binutils libglib2.0-0 software-properties-common git curl qt5-default qt5-qmake make g++


# Install libraries
RUN apt-get update && \
      apt-get install -y icnsutils libxcb-xfixes0 libxcb-icccm4-dev libxcb-icccm4 freetds-dev libsybdb5 libsybdb5 libxcb-image0 libxcb-image0-dev libgl1-mesa-dev libxkbcommon-x11-0 libxcb-icccm4 libxcb-image0 libxcb-keysyms1 libxcb-keysyms1-dev libxcb-render-util0 libxcb-xinerama0 libzstd-dev libcurl4-openssl-dev


# Install linuxdeployqt
RUN wget https://github.com/probonopd/linuxdeployqt/releases/download/7/linuxdeployqt-7-x86_64.AppImage \
      --quiet --output-document=/usr/bin/linuxdeployqt && \
    chmod +x /usr/bin/linuxdeployqt
    
# Install more things
RUN apt-get install -y libfontconfig1 libegl1-mesa libcups2 libodbc1 libpq5 libgtk-3-0

# Set QTDIR variable
ENV QTDIR /qt
ENV QML_IMPORT_PATH="/qt/qml:/app/DKV2"
ENV QML2_IMPORT_PATH="/qt/qml:/app/DKV2"
# Run build script
WORKDIR /app
RUN git config --global --add safe.directory /app
CMD /app/dist_linux.sh
