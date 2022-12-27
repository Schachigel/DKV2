#ifndef FILEWRITER_H
#define FILEWRITER_H



bool extractTemplateFileFromResource(const QString& path, const QString& file, const QString& outname =QString());

QString renderTemplate(const QString &templateName, const QVariantMap &data);

bool savePdfFromHtmlTemplate(const QString &templateName, const QString &fileName, const QVariantMap &data);

bool writeRenderedTemplate(const QString &templateName, const QString &fileName, const QVariantMap &data);

#endif // FILEWRITER_H
