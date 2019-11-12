#include <QDebug>
#include "dbtable.h"
#include "dbfield.h"

dbfield dbtable::operator[](QString s) const
{
    for (auto f : fields)
    {
        if( f.name() == s)
            return f;
    }
    return dbfield();
}

dbtable dbtable::append(const dbfield& f)
{
    dbfield newf(f);
    newf.setTableName(Name());
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

TableDataInserter::TableDataInserter(const dbtable& t)
{
    init(t);
}

void TableDataInserter::init(const dbtable& t)
{
    tablename = t.Name();
    for (auto dbfield : t.Fields())
    {
        QSqlField sqlField(dbfield.name(), dbfield.type(), tablename);
        if( dbfield.typeDetails().contains("AUTOINCREMENT", Qt::CaseInsensitive) )
            sqlField.setAutoValue(true);
        record.append(sqlField);
    }
}

void TableDataInserter::setValue(const QString& n, const QVariant& v)
{
    if( n.isEmpty()) return;
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

QString TableDataInserter::getInsertRecordSQL() const
{
    if( record.isEmpty()) return QString();
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
    sql +=")";
    return sql;
}

QString TableDataInserter::getUpdateRecordSQL() const
{
//"UPDATE names SET firstname = 'Nisse', lastname = 'Svensson' WHERE id = 7"
    if( record.isEmpty()) return QString();
    QString sql("UPDATE " + tablename +" SET ");
    QString where(" WHERE ");

    int alreadySetFields = 0;
    for( int i=0; i<record.count(); i++)
    {
        if( alreadySetFields>0) sql += ", ";
        if( record.field(i).isAutoValue())
            where += record.field(i).name() + " = " + record.field(i).value().toString();
        else
        {
            sql += record.field(i).name() + " = " + format4SQL(record.field(i).value());
            alreadySetFields++;
        }
    }
    sql += where;
    return sql;
}

int TableDataInserter::InsertData(QSqlDatabase db) const
{
    if( record.isEmpty()) return false;
    QSqlQuery q(db);
    bool ret = q.exec(getInsertRecordSQL());
    int lastRecord = q.lastInsertId().toInt();
    if( !ret)
    {
        qCritical() << "Insert record failed: " << q.lastError() << endl << q.lastQuery() << endl;
        return -1;
    }
    qDebug() << "successfully inserted Data at index " << q.lastInsertId().toInt() << endl <<  q.lastQuery() << endl;
    return lastRecord;
}

int TableDataInserter::UpdateData(QSqlDatabase db) const
{
    if( record.isEmpty()) return false;
    QSqlQuery q(db);
    bool ret = q.exec(getUpdateRecordSQL());
    int lastRecord = q.lastInsertId().toInt();
    if( !ret)
    {
        qCritical() << "Insert record failed: " << q.lastError() << endl << q.lastQuery() << endl;
        return -1;
    }
    qDebug() << "successfully updated Data at index " << q.lastInsertId().toInt() << endl <<  q.lastQuery() << endl;
    return lastRecord;
}

