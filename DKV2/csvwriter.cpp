
#include "helper.h"
#include "helpersql.h"
#include "helperfile.h"
#include "appconfig.h"
#include "csvwriter.h"

void csvwriter::addColumn(const QString& header)
{   LOG_CALL_W(header);
    QString h(header);
    Q_ASSERT(rows.empty()); // no add. columns after adding data
    h.replace(separator, qsl(","));
    h.replace(lineBreak, qsl(" | "));
    headers.append(h.trimmed());
}

int csvwriter::addColumns(const QString& headers)
{   LOG_CALL_W(headers);
    QList<QString> list = headers.split(separator);
    return addColumns(list);
}

int csvwriter::addColumns(const QStringList headers)
{   LOG_CALL;
    for(auto& s : qAsConst(headers))
    {
        addColumn(s);
    }
    return headers.size();
}

void csvwriter::appendToRow(const QString& value)
{   //LOG_CALL_W(value);
    QString v(value);
    v.replace(separator, qsl(","));
    v.replace(lineBreak, qsl(" | "));
    currentRow.append(v.trimmed());
    if( currentRow.size() == headers.size()) {
        rows.append( currentRow);
        currentRow.clear();
    }
}

void csvwriter::addRow(const QStringList &cols)
{   //LOG_CALL;
    Q_ASSERT(cols.size() == headers.size());
    for( auto& s : qAsConst(cols)) {
        appendToRow(s);
    }
}

void csvwriter::addRow(const QString& row)
{   //LOG_CALL_W(row);
    QList<QString> list = row.split(separator);
    addRow(list);
}

QString csvwriter::appendCsvLine(const QString& line, const QString& appendix) const
{   //LOG_CALL;
    QString l(line);
    if( l.size()) l += separator + qsl(" ");
    return l + appendix;
}


QString csvwriter::toString() const
{   LOG_CALL;
    QString out;
    for( auto& i : qAsConst(headers)) {
        out = appendCsvLine(out, i);
    }
    for( auto& j : qAsConst(rows)) {
        QString line;
        for( auto& k : qAsConst(j))
        {
            line = appendCsvLine( line, k);
        }
        out += qsl("\n") + line;
    }
    return out;
}

bool csvwriter::saveAndShowInExplorer(const QString& filename) const
{   LOG_CALL_W(filename);
    QString path {appConfig::Outdir() + qsl("/") + filename};
    backupFile(path);

    QFile file(path);
    std::unique_ptr<QTemporaryFile> p_tf {nullptr};
    if( not file.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        qCritical() << "could not open csv file for writing: " << filename;
        // check if we could get another filename to work with...
        p_tf =std::make_unique<QTemporaryFile>(tempPathTemplateFromPath(path));
        if( p_tf->open())
            qDebug() << "falling back to temporary file: " << p_tf->fileName ();
        else{
            qCritical() << "could not open any file for writing csv";
            return false;
        }
        p_tf->setAutoRemove (false); // we want to keep the file
        path =p_tf->fileName ();
    }
    QTextStream s;
    if( p_tf)
        s.setDevice (p_tf.get ());
    else
        s.setDevice (&file);
    s.setCodec("UTF-8");
    s.setGenerateByteOrderMark(true);
    s << toString();

    showInExplorer(path);
    return true;
}

bool StringLists2csv(const QString& filename, const QStringList& header, const QVector<QStringList>& data)
{
    LOG_CALL;
    int numColumns =header.size();
    csvwriter csv (qsl(";"));
    qDebug() << header;
    csv.addColumns(header);
    for( auto& line : qAsConst(data)) {
        qDebug() << line;
        if(line.size() not_eq numColumns){
            qWarning() << "csv file not created due to wrong number of elements in " << line;
            return false;
        }
        csv.addRow(line);
    }
    return csv.saveAndShowInExplorer (filename);
}

bool table2csv(const QString& filename, const QVector<dbfield>& fields, const QVector<QVariant::Type>& types, const QString& where)
{    LOG_CALL;
    csvwriter csv (qsl(";"));
    for(auto& f : qAsConst(fields))
        csv.addColumn(f.name());
    QString sql = selectQueryFromFields(fields, QVector<dbForeignKey>(), where);
    QSqlQuery q;
    if( not q.exec(sql)) {
        qCritical() << "sql faild to execute" << q.lastError() << "\nSQL: " << q.lastQuery();
        return false;
    }
    QLocale locale;
    while( q.next()) {
        for (int i = 0; i < fields.count(); i++) {
            QVariant::Type t = types.isEmpty() ? fields[i].type() : types[i];
            switch(t) {
            case QVariant::Int: {
                int v = q.record().value(fields[i].name()).toInt();
                csv.appendToRow(QString::number(v));
                break;
            }
            case QVariant::String: {
                csv.appendToRow(q.record().value(fields[i].name()).toString());
                break;
            }
            case QVariant::Date: {
                QDate d = q.record().value(fields[i].name()).toDate();
                csv.appendToRow(d.toString(qsl("dd.MM.yyyy")));
                break;
            }
            case QVariant::Double: {
                double d = q.record().value(fields[i].name()).toDouble();
                csv.appendToRow(locale.toString(d, 'f', 2));
                break;
            }
            default:
                csv.appendToRow(q.record().value(fields[i].name()).toString());
            }
        }
    }
    return csv.saveAndShowInExplorer(filename);
}
