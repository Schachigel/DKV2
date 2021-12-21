#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <QDateTime>
#include <QVariant>
#include <QString>
#include <QTextDocument>

bool extractTemplateFileFromResource(const QString& path, const QString& file);

bool pdfWrite(const QString& templateName, const QString& fileName, const QVariantMap& data);


#endif // FILEWRITER_H
