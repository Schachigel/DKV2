#ifndef FILEWRITER_H
#define FILEWRITER_H

bool savePdfFromHtmlTemplate(const QString &templateName, const QString &fileName, const QVariantMap &data);


bool extractTemplateFileFromResource(const QString& path, const QString& file, const QString& outname =QString());

QString renderTemplate(const QString &templateName, const QVariantMap &data);

bool writeRenderedTemplate(const QString &templateName, const QString &fileName, const QVariantMap &data);

inline double cm2Pt(double cm) { return cm * 28.3465; };

// QString mustachReplace(const QString &templateFileName, const QVariantMap &data);

// QString replaceExtension(const QString& pdfFileName, const QString& ext);

#endif // FILEWRITER_H
