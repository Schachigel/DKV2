#!/bin/bash

# Stop at any error
set -e

# Location of the source tree
SOURCEDIR=`pwd`/DKV2
PROJECTFILE=DKV2.pro
# Location to build DKV2
BUILDDIR=`pwd`/build-dist-linux

# Prepare QT build enviornment: accept either QTDIR or Qt5_Dir env variable
# for QT location
if [ -z ${QTDIR+x} ]; then
	QTDIR=${Qt5_Dir}
fi
if [ -z ${QTDIR+x} ]; then
	echo "QTDIR not defined- please set it to the location containing the Qt version to build against. For example:"
	echo "  export QTDIR=~/Qt/5.15.0/gcc_64"
	exit 1
fi

QMAKE=${QTDIR}/bin/qmake
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

${QMAKE} ${SOURCEDIR}/${PROJECTFILE} \
    -spec linux-g++ \
    CONFIG+=qtquickcompiler

${MAKE} -j6

##### package with linuxdeployqt #####

mkdir -p app
pushd app
cp ${SOURCEDIR}/res/logo256.png dkv2.png
cp ${SOURCEDIR}/res/DKV2.desktop .
mv ${BUILDDIR}/DKV2 .
popd

unset LD_LIBRARY_PATH # Remove too old Qt from the search path
LINUXDEPLOYQT_OPTS=-unsupported-allow-new-glibc -unsupported-bundle-everything
# PATH=${QTDIR}/bin:${PATH} ${LINUXDEPLOYQT} app/DKV2 -bundle-non-qt-libs ${LINUXDEPLOYQT_OPTS}
EXTRA_PLUGINS="platforms/libqxcb.so,platformthemes/libqgtk3.so,styles/libqgtk3style.so"
PATH=${QTDIR}/bin:${PATH} ${LINUXDEPLOYQT} app/DKV2 \
	-extra-plugins=${EXTRA_PLUGINS} \
	-appimage ${LINUXDEPLOYQT_OPTS}

APPIMAGE_GIT_FILENAME="DKV2-${GIT_VERSION}-x86_64.AppImage"
APPIMAGE_RES_FILENAME="DKV2-${VERSION}-x86_64.AppImage"
ARTIFCACT_FILENAME="DKV2-${VERSION}-linux-x86_64.tar.gz"

mv ${APPIMAGE_GIT_FILENAME} ${APPIMAGE_RES_FILENAME} 2>/dev/null | true
tar -czf ${ARTIFCACT_FILENAME} ${APPIMAGE_RES_FILENAME}
chown -R $LOCAL_USER:$LOCAL_GROUP `pwd`
echo "Created ${BUILDDIR}/${ARTIFCACT_FILENAME}"
