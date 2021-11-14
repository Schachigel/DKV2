#include "pch.h"
#include "creditor.h"
#include "lettersnippets.h"

/*static*/ const QString snippet::tableName  {qsl("BriefElemente")};
/*static*/ const QString snippet::fnSnippet  {qsl("Element")};
/*static*/ const QString snippet::fnLetter   {qsl("Brief")};
/*static*/ const QString snippet::fnCreditor {qsl("Kid")};
/*static*/ const QString snippet::fnText     {qsl("Text")};

/*static*/ const dbtable& snippet::getTableDef()
{
    static dbtable snippetstable(tableName);
    if( 0 == snippetstable.Fields().size()) {
        snippetstable.append(dbfield(fnSnippet,  QVariant::Int));
        snippetstable.append(dbfield(fnLetter,   QVariant::Int));
        snippetstable.append(dbfield(fnCreditor, QVariant::Int));
        snippetstable.append(dbForeignKey(snippetstable[fnCreditor],
                     creditor::getTableDef().Name(), qsl("id"), qsl("ON DELETE CASCADE")));
        snippetstable.append (dbfield(fnText));
        QVector<dbfield> unique {snippetstable[fnSnippet], snippetstable[fnLetter], snippetstable[fnCreditor]};
        snippetstable.setUnique (unique);
    }
    return snippetstable;
}
/*static*/ const QVector<QString> snippet::getIndexes()
{
    QVector<QString>v;
    v.append( qsl("CREATE UNIQUE INDEX unique_default_snippets "
                  "ON %1 (%2, %3) WHERE %4 IS NULL").arg(
                  tableName, fnSnippet, fnLetter, fnCreditor));
    return v;
}

 letterType letterTypeFromInt( int i) {
    Q_ASSERT(i<=int(letterType::maxValue));
    return static_cast<letterType>(i);
}

 snippetType snippetTypeFromInt( int i) {
    Q_ASSERT(i<=int(snippetType::maxValue));
    return static_cast<snippetType>(i);
}

 letterSnippet letterSnippetFromInt(int i) {
    Q_ASSERT(i<=int(letterSnippet::maxValue));
    return static_cast<letterSnippet>(i);
}


QString snippet::read(const letterType lId, const letterSnippet sId, const qlonglong kId, QSqlDatabase db/*=QSqlDatabase::database ()*/)
{
    QString where {qsl("%1=%4 AND %2=%5 AND %3 ").arg(fnSnippet, fnLetter, fnCreditor,
                                        QString::number(int(sId)), QString::number(int(lId)))};
    QString sKid = kId ? (qsl(" =") + QString::number(kId)) : qsl(" IS NULL");

    QString text =executeSingleValueSql (getTableDef ()[fnText], where+sKid, db).toString ();
    return text;
}

bool    snippet::write (const letterType lId, const letterSnippet sId, const qlonglong kId, const QString text, QSqlDatabase db/*=QSqlDatabase::database ()*/)
{
    QString sql {qsl("REPLACE INTO %1 VALUES (?, ?, ?, ?) ").arg(tableName)};
    QVariant sKid = kId ? QVariant(kId) : QVariant(QVariant::LongLong);
    QVector<QVariant> vars {int(sId), int(lId), sKid, text};
    return executeSql_wNoRecords (sql, vars, db);
}

QString snippet::read(QSqlDatabase db)
{
    return read(lType, ls, cId, db);
}

bool snippet::write(const QString& text, QSqlDatabase db)
{
    return write(lType, ls, cId, text, db);
}
