#include <QDebug>
#include "helper.h"
#include "dbtable.h"
#include "dbfield.h"

dbfield dbtable::operator[](QString s) const
{   // LOG_CALL_W(s);
    for (auto f : fields) {
        if( f.name() == s)
            return f;
    }
    return dbfield();
}

dbtable dbtable::append(const dbfield& f)
{   // LOG_CALL;
    dbfield newf(f);
    newf.setTableName(Name());
    fields.append(newf);
    return *this;
}

void dbtable::setUnique( const QVector<dbfield>& fs)
{   // LOG_CALL;
    QString tmp;
    for( int i =0; i < fs.count(); i++)
    {
        if( i>0) tmp += ", ";
        tmp += fs[i].name();
    }
    unique = ", UNIQUE (" +tmp +")";
}

QString dbtable::createTableSql() const
{   LOG_CALL;
    QString sql("CREATE TABLE " + name + " (");
    for( int i = 0; i< Fields().count(); i++) {
        if( i>0) sql.append(", ");
        sql.append(Fields()[i].getCreateSqlSnippet());
    }
    if( !unique.isEmpty())
        sql.append(unique);
    sql.append(")");

    return sql;
}

bool dbtable::create(QSqlDatabase db) const
{   LOG_CALL_W(name);
    QSqlQuery q(db);
    q.prepare(createTableSql());
    if( !q.exec())
    {
        qCritical() << "dbtable::create failed" << q.lastError() << endl << "SQL: " << q.lastQuery();
        return false;
    }
    else
        qDebug() << "Successfully ceated Table (SQL: " << q.lastQuery() << ")";
    return true;
}

TableDataInserter::TableDataInserter(const dbtable& t)
{
    init(t);
}

void TableDataInserter::init(const dbtable& t)
{   LOG_CALL;
    tablename = t.Name();
    for (auto dbfield : t.Fields()) {
        QSqlField sqlField(dbfield.name(), dbfield.type(), tablename);
        if( dbfield.typeDetails().contains("AUTOINCREMENT", Qt::CaseInsensitive) )
            sqlField.setAutoValue(true);
        record.append(sqlField);
    }
}

void TableDataInserter::setValue(const QString& n, const QVariant& v)
{   // LOG_CALL_W(n);
    qInfo() << "tableinserter setValue: " << n << " -> " << v ;
    if( n.isEmpty()) return;
    if( record.contains(n)) {
        if( record.field(n).type() == v.type())
            record.setValue(n, v);
        else {
            qDebug() << "wrong field type for insertion -> converting" << v.type() << " -> " << record.field(n).type();
            QVariant vf (v); vf.convert(record.field(n).type());
            record.setValue(n, vf);
        }
    }
    else
        qCritical() << "wrong field name for insertion " << n;
}

QString format4SQL(QVariant v)
{
    if( v.isNull())
        return "NULL";
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
    case QVariant::String:
    default:
        s = v.toString();
    }
    return "'" + s +"'";
}

QString TableDataInserter::getInsertRecordSQL() const
{//   LOG_CALL;
    if( record.isEmpty()) return QString();
    QString sql("INSERT INTO " + tablename +" VALUES (");

    for( int i=0; i<record.count(); i++) {
        if( i>0) sql += ", ";
        if( record.field(i).isAutoValue())
            sql += "NULL";
        else {
            sql += format4SQL(record.field(i).value());
        }
    }
    sql +=")";
    return sql;
}

QString TableDataInserter::getInsertOrReplaceRecordSQL() const
{   LOG_CALL;
    if( record.isEmpty()) return QString();
    QString sql("INSERT OR REPLACE INTO " + tablename +" VALUES (");

    for( int i=0; i<record.count(); i++) {
        if( i>0) sql += ", ";
        if( record.field(i).isAutoValue())
            sql += "NULL";
        else
            sql += format4SQL(record.field(i).value());
    }
    sql +=")";
    return sql;
}

QString TableDataInserter::getUpdateRecordSQL() const
{   LOG_CALL;
    if( record.isEmpty()) return QString();
    QString sql("UPDATE " + tablename +" SET ");
    QString where(" WHERE ");

    int alreadySetFields = 0;
    for( int i=0; i<record.count(); i++) {
        if( alreadySetFields>0) sql += ", ";
        if( record.field(i).isAutoValue())
            where += record.field(i).name() + " = " + record.field(i).value().toString();
        else {
            sql += record.field(i).name() + " = " + format4SQL(record.field(i).value());
            alreadySetFields++;
        }
    }
    sql += where;
    return sql;
}

int TableDataInserter::InsertData(QSqlDatabase db) const
{   // LOG_CALL;
    if( record.isEmpty()) return false;
    QString insertSql = getInsertRecordSQL();
    qDebug() << "TableDataInserter using " << insertSql;
    QSqlQuery q(db);
    bool ret = q.exec(insertSql);
    qlonglong lastRecord = q.lastInsertId().toLongLong();
    if( !ret) {
        qCritical() << "Insert record failed: " << q.lastError() << endl << q.lastQuery() << endl;
        return -1;
    }
    qDebug() << "successfully inserted Data at index " << q.lastInsertId().toLongLong() << endl <<  q.lastQuery() << endl;
    return lastRecord;
}

int TableDataInserter::InsertOrReplaceData() const
{   LOG_CALL;
    if( record.isEmpty()) return false;
    QSqlQuery q;
    bool ret = q.exec(getInsertOrReplaceRecordSQL());
    qlonglong lastRecord = q.lastInsertId().toLongLong();
    if( !ret) {
        qCritical() << "Insert/replace record failed: " << q.lastError() << endl << q.lastQuery() << endl;
        return -1;
    }
    qDebug() << "successfully inserted Data at index " << q.lastInsertId().toLongLong() << endl <<  q.lastQuery() << endl;
    return lastRecord;
}

int TableDataInserter::UpdateData() const
{   LOG_CALL;
    if( record.isEmpty()) return false;
    QSqlQuery q;
    bool ret = q.exec(getUpdateRecordSQL());
    int lastRecord = q.lastInsertId().toInt();
    if( !ret) {
        qCritical() << "Update record failed: " << q.lastError() << endl << q.lastQuery() << endl;
        return -1;
    }
    qDebug() << "successfully updated Data at index " << q.lastInsertId().toInt() << endl <<  q.lastQuery() << endl;
    return lastRecord;
}

