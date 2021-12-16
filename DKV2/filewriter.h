#ifndef FILEWRITER_H
#define FILEWRITER_H

#include <QDateTime>
#include <QVariant>
#include <QString>
#include <QTextDocument>


bool pdfWrite(QString templateName, QString fileName, QVariantMap data);


#endif // FILEWRITER_H