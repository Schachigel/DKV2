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
