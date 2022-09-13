#ifndef CSVWRITER_H
#define CSVWRITER_H

#include "pch.h"

#include "dbfield.h"

class csvwriter
{
public:
    csvwriter(const QString& sep =qsl(";")) : separator(sep){ Q_ASSERT(sep.length()==1);}
    void addColumn(const QString& header);
    int  addColumns(const QString& headers);
    int addColumns(const QStringList headers);
    void appendToRow(const QString& value);
    void addRow(const QStringList& cols);
    void addRow(const QString& row);

    QString toString() const;
    bool saveAndShowInExplorer(const QString& filname) const;
private:
    QString appendCsvLine( const QString& line, const QString& appendix) const;

    QString separator;
    QRegularExpression lineBreak = QRegularExpression(qsl("[\\n\\r]+")); // all \r\n combinations
    QList<QString> headers;
    QList<QList<QString>> rows;
    QList<QString> currentRow;
};

bool table2csv(const QString& filename, const QVector<dbfield>& fields, const QVector<QVariant::Type>& types =QVector<QVariant::Type>(), const QString& where =QString());
bool StringLists2csv(const QString& filename, const QStringList& header, const QVector<QStringList>& lists);

#endif // CSVWRITER_H
