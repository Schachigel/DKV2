#include <QDebug>
#include "dbtable.h"
#include "dbfield.h"

dbfield dbtable::operator[](QString s)
{
    for (auto f : fields)
    {
        if( f.name == s)
            return f;
    }
    return dbfield();
}

dbtable dbtable::append(dbfield f)
{
    f.tablename = name;
    fields.append(f);
    return *this;
}

QString dbtable::CreateTableSQL()
{
    QString sql("CREATE TABLE [" + name + "] (");
    for( int i = 0; i< fields.count(); i++)
    {
        if( i>0) sql.append(", ");
        sql.append(fields[i].CreateSQL());
    }
    sql.append(")");
    return sql;
}

TableDataInserter::TableDataInserter(const dbtable& t) :tablename(t.name)
{
    for (auto f : t.fields)
    {
        QSqlField sqlf(f.name, f.vType, tablename);
        if( f.TypeInfo.contains("AUTOINCREMENT", Qt::CaseInsensitive) )
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
        qCritical() << "wrong field name for insertion";
}

QString singleQuoted(QString s)
{
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
            sql += singleQuoted(record.field(i).value().toString());
    }
    qDebug() << "InsertRecordSQL:\n" << sql << ")";
    return sql +")";
}

bool TableDataInserter::InsertData(QSqlDatabase db)
{
    QSqlQuery q(db);
    bool ret = q.exec(InsertRecordSQL());
    if( !ret)
    {
        qCritical() << "Insert record failed: " << q.lastError() << "\n" << q.lastQuery() << endl;
        return false;
    }
    return true;
}
