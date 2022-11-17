FROM ubuntu:20.04
# FROM mhitza/linuxdeployqt:5.11.3

# Install basics
RUN apt update && \
      apt install --assume-yes wget fuse binutils libglib2.0-0 software-properties-common git

# Install python and aqtinstall
RUN add-apt-repository ppa:deadsnakes/ppa && apt update && \
      apt install --assume-yes python3.7 python3-pip && \
      update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.7 1 && \
      python3 -m pip install --upgrade pip
RUN pip3 install "aqtinstall==1.1.5"

# Install libraries
RUN apt-get update && \
      apt-get install -y icnsutils libxcb-xfixes0 libxcb-icccm4-dev libxcb-icccm4 freetds-dev libsybdb5 libsybdb5 libxcb-image0 libxcb-image0-dev libgl1-mesa-dev libxkbcommon-x11-0 libxcb-icccm4 libxcb-image0 libxcb-keysyms1 libxcb-keysyms1-dev libxcb-render-util0 libxcb-xinerama0 libzstd-dev libcurl4-openssl-dev

# Install QT
ARG QT_VERSION=5.15.2
ARG QT_MODULES="qtgui qtwidgets qtsql qtcharts qtprintsupport qtstyleplugins"
WORKDIR /tmp
RUN aqt install $QT_VERSION linux desktop \
      --modules $QT_MODULES \
      -b https://mirrors.ocf.berkeley.edu/qt \
    && mkdir /qt \
    && cp -R ./$QT_VERSION/gcc_64/* /qt \
    && rm -rf ./$QT_VERSION

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
CMD /app/dist_linux.sh
