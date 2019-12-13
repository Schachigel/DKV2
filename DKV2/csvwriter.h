#ifndef CSVWRITER_H
#define CSVWRITER_H

#include <QString>
#include <QList>
#include <QStringList>

class csvwriter
{
public:
    csvwriter (){}
    void addColumn(QString header);
    void appendToRow(QString value);
    void addRow(QList<QString> cols);
    void save(QString filname);
    QString out();
private:
    QList<QString> headers;
    QList<QList<QString>> rows;
    QList<QString> currentRow;
};

#endif // CSVWRITER_H
