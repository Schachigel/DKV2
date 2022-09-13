
#include "helper.h"
#include "helpersql.h"
#include "dbtable.h"
#include "dbfield.h"

dbfield dbtable::operator[](const QString& s) const
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
    for( auto& f : qAsConst(fs))
    {
        if( tmp.size()) tmp = tmp + qsl(", ");
        tmp =tmp + f.name();
    }
    unique = qsl(", UNIQUE (") +tmp +qsl(")");
}

QString dbtable::createTableSql() const
{   //LOG_CALL;
    QString sql(qsl("CREATE TABLE ") + name + qsl(" ("));
    for( int i = 0; i< Fields().count(); i++) {
        if( i>0) sql.append(qsl(", "));
        sql.append(Fields().at(i).get_CreateSqlSnippet());
    }
    for( auto fk : foreignKeys) {
        sql.append(", ");
        sql.append(fk.get_CreateSqlSnippet());
    }
    if( unique.size())
        sql.append(unique);
    sql.append(")");

    return sql;
}

bool dbtable::create(const QSqlDatabase& db) const
{   // LOG_CALL_W(name);
    if( executeSql_wNoRecords( createTableSql(), db)) {
        qDebug() << "Successfully created Table " << name;
        return true;
    } else {
        qCritical() << "dbtable::create " << name << " failed";
        return false;
    }
}
