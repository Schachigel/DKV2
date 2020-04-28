#ifndef CSVWRITER_H
#define CSVWRITER_H

#include <QString>
#include <QList>
#include <QStringList>

#include "sqlhelper.h"
#include "dbfield.h"

class csvwriter
{
public:
    void addColumn(QString header);
    int  addColumns(QString headers);
    void appendToRow(QString value);
    void addRow(QList<QString> cols);
    void addRow(QString row);

    QString out();
    bool save(QString filname);
private:
    QList<QString> headers;
    QList<QList<QString>> rows;
    QList<QString> currentRow;
};

bool table2csv(QVector<dbfield>& fields, QString where, QString filename, QSqlDatabase db = defaultDb());
bool table2csv(QVector<dbfield>& fields, QVector<QVariant::Type>& types, QString where, QString filename, QSqlDatabase db = defaultDb());

#endif // CSVWRITER_H
