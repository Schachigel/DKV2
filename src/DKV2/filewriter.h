#ifndef FILEWRITER_H
#define FILEWRITER_H

#include "helper_core.h"

// Filenames of the templates/assets embedded via resource.qrc (see res/ subfolder).
// These are extracted to disk by extractTemplateFileFromResource() and later
// re-opened by mustachReplace()/savePdfFromHtmlTemplate()/writeRenderedTemplate().
// A single source of truth avoids case mismatches between extraction and
// consumption, which go unnoticed on case-insensitive filesystems (Windows)
// but break template lookup on case-sensitive ones (Linux/macOS).
inline const QString tplFnBrieflogoPng{qsl("brieflogo.png")};
inline const QString tplFnZinsbriefCss{qsl("zinsbrief.css")};
inline const QString tplFnZinsbriefHtml{qsl("zinsbrief.html")};
inline const QString tplFnZinslisteHtml{qsl("zinsliste.html")};
inline const QString tplFnZinsmailsWinBat{qsl("zinsmails-win.bat")};
inline const QString tplFnZinsmailsLinuxBat{qsl("zinsmails-linux.bat")};
inline const QString tplFnZinsmailsBat{qsl("zinsmails.bat")};
inline const QString tplFnDkv2mailBat{qsl("dkv2mail.bat")};
inline const QString tplFnDkv2mail{qsl("dkv2mail")};
inline const QString tplFnEndabrechnungHtml{qsl("endabrechnung.html")};
inline const QString tplFnEndabrZinsnachwHtml{qsl("endabr-Zinsnachw.html")};
inline const QString tplFnEndabrechnungCsv{qsl("endabrechnung.csv")};

// Output-dir subfolder names, repeated across annual_letters.cpp/filewriter.cpp.
inline const QString dirVorlagen{qsl("vorlagen")};
inline const QString dirHtml{qsl("html")};

QString appendFilenameToOutputDir(QString filename);

bool savePdfFromHtmlTemplate(const QString &templateName, const QString &fileName, const QVariantMap &data);

#define showFolder true
#define showFile   false

QString saveStringToUtf8File(const QString& filename, const QString& content);

QString sqltableToCsvString(QString sql, QVector<QPair<QString, QVariant>> params);

bool extractTemplateFileFromResource(const QString& path, const QString& file, const QString& outname =QString());

QString renderTemplate(const QString &templateName, const QVariantMap &data);

bool writeRenderedTemplate(const QString &templateName, const QString &fileName, const QVariantMap &data);

inline double cm2Pt(double cm) { return cm * 28.3465; };

// QString mustachReplace(const QString &templateFileName, const QVariantMap &data);

// QString replaceExtension(const QString& pdfFileName, const QString& ext);

#endif // FILEWRITER_H
