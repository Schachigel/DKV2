#include <QDebug>
#include "dbtable.h"
#include "dbfield.h"

dbfield dbtable::operator[](QString s)
{
    for (auto f : fields)
    {
        if( f.Name() == s)
            return f;
    }
    return dbfield();
}

dbtable dbtable::append(const dbfield& f)
{
    dbfield newf(f);
    newf.setTablename(Name());
    fields.append(newf);
    return *this;
}

QString dbtable::CreateTableSQL() const
{
    QString sql("CREATE TABLE [" + name + "] (");
    for( int i = 0; i< Fields().count(); i++)
    {
        if( i>0) sql.append(", ");
        sql.append(Fields()[i].getCreateSqlSnippet());
    }
    sql.append(")");
    return sql;
}

TableDataInserter::TableDataInserter(const dbtable& t) :tablename(t.Name()), lastRecord(-1)
{
    for (auto f : t.Fields())
    {
        QSqlField sqlf(f.Name(), f.VType(), tablename);
        if( f.TypeInfo().contains("AUTOINCREMENT", Qt::CaseInsensitive) )
            sqlf.setAutoValue(true);
        record.append(sqlf);
    }
}

void TableDataInserter::setValue(const QString& n, const QVariant& v)
{
    if( record.contains(n))
    {
        if( record.field(n).type() == v.type())
            record.setValue(n, v);
        else
            qCritical() << "wrong field type for insertion";
    }
    else
        qCritical() << "wrong field name for insertion " << n;
}

QString format4SQL(QVariant v)
{
    QString s;
    switch(v.type())
    {
    case QVariant::Double:
    {   double d(v.toDouble());
        s = QString::number(d, 'f', 2);
        break;
    }
    case QVariant::Bool:
        s = (v.toBool())? "1" : "0";
        break;
    default:
        s = v.toString();
    }

    return "'" + s +"'";
}

QString TableDataInserter::InsertRecordSQL()
{
    QString sql("INSERT INTO " + tablename +" VALUES (");

    for( int i=0; i<record.count(); i++)
    {
        if( i>0) sql += ", ";
        if( record.field(i).isAutoValue())
            sql += "NULL";
        else
        {
            sql += format4SQL(record.field(i).value());
        }
    }
    qDebug() << "InsertRecordSQL:\n" << sql << ")";
    return sql +")";
}

bool TableDataInserter::InsertData(QSqlDatabase db)
{
    QSqlQuery q(db);
    bool ret = q.exec(InsertRecordSQL());
    lastRecord = q.lastInsertId().toInt();
    if( !ret)
    {
        qCritical() << "Insert record failed: " << q.lastError() << "\n" << q.lastQuery() << endl;
        return false;
    }
    return true;
}
