#include <QSqlQuery>
#include <QSqlError>

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

dbtable dbtable::append(const dbForeignKey& fk)
{
    foreignKeys.append(fk);
    return *this;
}

void dbtable::setUnique( const QVector<dbfield>& fs)
{   // LOG_CALL;
    QString tmp;
    for( auto f : fs)
    {
        if( ! tmp.isEmpty()) tmp += ", ";
        tmp += f.name();
    }
    unique = ", UNIQUE (" +tmp +")";
}

QString dbtable::createTableSql() const
{   LOG_CALL;
    QString sql("CREATE TABLE " + name + " (");
    for( int i = 0; i< Fields().count(); i++) {
        if( i>0) sql.append(", ");
        sql.append(Fields()[i].get_CreateSqlSnippet());
    }
    for( auto fk : foreignKeys) {
        sql.append(", ");
        sql.append(fk.get_CreateSqlSnippet());
    }
    if( !unique.isEmpty())
        sql.append(unique);
    sql.append(")");

    return sql;
}

bool dbtable::create(QSqlDatabase db) const
{   LOG_CALL_W(name);
    QSqlQuery q(db);
    if( !q.exec(createTableSql()))
    {
        qCritical() << "dbtable::create failed" << q.lastError() << endl << "SQL: " << q.lastQuery();
        return false;
    }
    else
        qDebug() << "Successfully ceated Table (SQL: " << q.lastQuery() << ")";
    return true;
}
