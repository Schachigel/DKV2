#!/usr/bin/env bash
set -euo pipefail

APP_NAME="DKV2"
OUT_DIR="build-dist-linux"
LDQ_URL="https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"

mkdir -p "${OUT_DIR}"
cd "${OUT_DIR}"

# linuxdeployqt holen
if [ ! -f linuxdeployqt.AppImage ]; then
  wget -nv "${LDQ_URL}" -O linuxdeployqt.AppImage
  chmod +x linuxdeployqt.AppImage
fi

# ---- Binary automatisch finden ----
# Kandidaten (häufige Pfade) + generische Suche als Fallback
BIN_CANDIDATES=(
  "../build/${APP_NAME}"
  "../build/src/${APP_NAME}"
  "$(find ../build -maxdepth 3 -type f -executable -name "${APP_NAME}" 2>/dev/null | head -n1 || true)"
)
BIN_PATH=""
for p in "${BIN_CANDIDATES[@]}"; do
  if [ -n "${p}" ] && [ -f "${p}" ]; then BIN_PATH="${p}"; break; fi
done
if [ -z "${BIN_PATH}" ]; then
  echo "[!] Konnte Executable nicht finden. Build-Struktur:"
  ls -R ../build || true
  exit 1
fi
echo "[i] Gefundenes Binary: ${BIN_PATH}"

# ---- AppDir vorbereiten ----
APPDIR="${PWD}/AppDir"
rm -rf "${APPDIR}"
mkdir -p "${APPDIR}/usr/bin" "${APPDIR}/usr/share/applications" "${APPDIR}/usr/share/icons/hicolor/256x256/apps"

cp -a "${BIN_PATH}" "${APPDIR}/usr/bin/${APP_NAME}"
chmod +x "${APPDIR}/usr/bin/${APP_NAME}"

# Icon optional
if [ -f "../../resources/icons/${APP_NAME}.png" ]; then
  cp "../../resources/icons/${APP_NAME}.png" "${APPDIR}/usr/share/icons/hicolor/256x256/apps/${APP_NAME}.png"
else
  printf '\211PNG\r\n\032\n\000\000\000\rIHDR\000\000\000\001\000\000\000\001\010\006\000\000\000\031\221\215\036\000\000\000\012IDATx\234c\000\001\000\000\005\000\001\r\n-\262\000\000\000\000IEND\256B`\202' \
    > "${APPDIR}/usr/share/icons/hicolor/256x256/apps/${APP_NAME}.png"
fi

find "$QT_ROOT_DIR/plugins/sqldrivers" -type f -name 'libqsql*.so' ! -name 'libqsqlite.so' -print -delete

#for p in \
#  "$QT_ROOT_DIR/plugins/sqldrivers/libqsqlmimer.so" \
#  "$QT_ROOT_DIR/plugins/sqldrivers/libqsqlodbc.so" \
#  "$QT_ROOT_DIR/plugins/sqldrivers/libqsqlibase.so" \
#  "$QT_ROOT_DIR/plugins/sqldrivers/libqsqlmysql.so" \
#  "$QT_ROOT_DIR/plugins/sqldrivers/libqsqlpsql.so"
#do
#  [ -f "$p" ] && echo "[i] removing $p" && rm -f "$p"
#done

# .desktop
DESKTOP_FILE="${APPDIR}/usr/share/applications/${APP_NAME}.desktop"
cat > "${DESKTOP_FILE}" <<EOF
[Desktop Entry]
Type=Application
Name=${APP_NAME}
Exec=${APP_NAME}
Icon=${APP_NAME}
Categories=Utility;
Terminal=false
EOF

# ---- AppImage erzeugen ----
echo "[i] Running linuxdeployqt..."
set -x
./linuxdeployqt.AppImage "${DESKTOP_FILE}" -appimage -bundle-non-qt-libs \
-exclude-plugins=sqldrivers/libqsqlmimer.so,sqldrivers/libqsqlodbc.so,sqldrivers/libqsqlibase.so || {
  echo "[i] Fallback: extract-and-run"
  ./linuxdeployqt.AppImage --appimage-extract
  ./squashfs-root/usr/bin/linuxdeployqt "${DESKTOP_FILE}" -appimage -bundle-non-qt-libs
}
set +x

echo "[✓] AppImage fertig: $(ls -1 *.AppImage)"
