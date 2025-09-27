#!/bin/bash

# Stop at any error
set -e

# Location of the source tree
SOURCEDIR=`pwd`/src/DKV2
PROJECTFILE=DKV2.pro.user
# Location to build DKV2
BUILDDIR=`pwd`/build-dist-linux

# Prepare QT build environment: accept either QTDIR or Qt6_Dir env variable for QT location,
# fallback to `/usr`.  `qmake6` should be in `$QTDIR/bin`
if [ -z ${QTDIR+x} ]; then
  QTDIR="${Qt5_Dir:-/usr}"
fi

QMAKE=${QTDIR}/bin/qmake6
MAKE=make

GIT_VERSION=`git rev-parse --short HEAD`
VERSION=`git describe --tags 2> /dev/null || echo $GIT_VERSION`

LINUXDEPLOYQT="linuxdeployqt"
if ! command -v $LINUXDEPLOYQT &> /dev/null; then
  LINUXDEPLOYQT=`pwd`/linuxdeployqt-continuous-x86_64.AppImage
fi

echo "Building with QTDIR: \`${QTDIR}\`"

##### download linuxdeployqt

if [ ! -f $LINUXDEPLOYQT ]; then
  curl -o "$LINUXDEPLOYQT" "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
fi

##### build #####

mkdir -p ${BUILDDIR}
pushd ${BUILDDIR}

cmake ..

# ${QMAKE} ${SOURCEDIR}/${PROJECTFILE} \
#     -spec linux-g++ \
#     CONFIG+=qtquickcompiler

${MAKE} -j

##### package with linuxdeployqt #####

mkdir -p app
pushd app
cp ${SOURCEDIR}/res/logo256.png dkv2.png
cp ${SOURCEDIR}/res/DKV2.desktop .
cp ${BUILDDIR}/src/DKV2/DKV2 .
popd

unset LD_LIBRARY_PATH # Remove too old Qt from the search path

LINUXDEPLOYQT_OPTS=-unsupported-allow-new-glibc
# EXTRA_PLUGINS="platforms/libqxcb.so,platformthemes/libqgtk3.so,styles/libqgtk3style.so"
EXTRA_PLUGINS="platforms/libqxcb.so,platformthemes/libqgtk3.so"

# linuxdeployqt bugs which needed workarounds:
# - https://github.com/probonopd/linuxdeployqt/issues/612
# - https://github.com/probonopd/linuxdeployqt/issues/608
PATH=${QTDIR}/bin:${PATH} ${LINUXDEPLOYQT} app/DKV2 \
  -extra-plugins=${EXTRA_PLUGINS} \
  -appimage \
  ${LINUXDEPLOYQT_OPTS} \
  -bundle-non-qt-libs \
  -qmake="${QMAKE}" \
  -no-strip

APPIMAGE_GIT_FILENAME="DKV2-${GIT_VERSION}-x86_64.AppImage"
APPIMAGE_RES_FILENAME="DKV2-${VERSION}-x86_64.AppImage"
ARTIFCACT_FILENAME="DKV2-${VERSION}-linux-x86_64.tar.gz"

mv ${APPIMAGE_GIT_FILENAME} ${APPIMAGE_RES_FILENAME} 2>/dev/null | true
tar -czf ${ARTIFCACT_FILENAME} ${APPIMAGE_RES_FILENAME}
chown -R $LOCAL_USER:$LOCAL_GROUP `pwd`
echo "Created ${BUILDDIR}/${ARTIFCACT_FILENAME}"

