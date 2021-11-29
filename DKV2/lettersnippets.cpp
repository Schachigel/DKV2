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

letterSnippet letterSnippetFromInt(int i) {
    Q_ASSERT(i<=int(letterSnippet::maxValue));
    return static_cast<letterSnippet>(i);
}
snippetType snippetTypeFromInt( int i) {
    Q_ASSERT(i<=int(snippetType::maxValue));
    return static_cast<snippetType>(i);
}

snippet::snippet(letterSnippet ls, letterType lT /*=allLetters */, qlonglong creditor /*=0*/)
    : ls(ls), lType (lT), cId(creditor)
{
    // ensure consistency
    switch(type()){
    case snippetType::allLettersAllKreditors:
        if( lType not_eq (letterType)allLetters || cId not_eq 0) {
            qDebug() << "resetting snippet attributes" << snippetNames[int(ls)];
            lType = (letterType)allLetters;
            cId = 0;
        }
        break;
    case snippetType::allKreditors:
        if( cId not_eq 0) {
            qDebug() << "resetting snippet attributes" << snippetNames[int(ls)];
            cId = 0;
        }
        if(lT == (letterType)allLetters) // there is no generic "Betreff" for all letters ...
            qCritical() << "wrong letter type for this snippetType" << snippetNames[int(ls)];
        break;
    case snippetType::allLetters:
        if( lType not_eq (letterType)allLetters) {
            qDebug() << "resetting snippet attributes" << snippetNames[int(ls)];
            lType = (letterType)allLetters;
        }
    case snippetType::individual:
    case snippetType::maxValue:
        break;
    }
}

std::pair<QString, bool> snippet::read(const letterSnippet sId, const letterType lId, const qlonglong kId, QSqlDatabase db/*=QSqlDatabase::database ()*/)
{
    // generic read function for the db stuff
    QString where {qsl("%1=%4 AND %2=%5 AND %3%6").arg(fnSnippet, fnLetter, fnCreditor)};
    where =where.arg (QString::number(int(sId)), QString::number(int(lId)));
    where =where.arg(kId ? (qsl(" =") + QString::number(kId)) : qsl(" IS NULL"));

    QVariant result =executeSingleValueSql (getTableDef ()[fnText], where, db);
    bool success =result.isValid ();
    if( not success)
        qDebug() << "snippet not found";
    return std::make_pair(result.toString (), success);
}

bool snippet::write (const QString text, const letterSnippet sId, const letterType lId, const qlonglong kId, QSqlDatabase db/*=QSqlDatabase::database ()*/)
{
    // generic write function for the db stuff
    QString sql {qsl("REPLACE INTO %1 VALUES (?, ?, ?, ?) ").arg(tableName)};
    QVariant sKid = kId ? QVariant(kId) : QVariant(QVariant::LongLong);
    QVector<QVariant> vars {int(sId), int(lId), sKid, text};
    return executeSql_wNoRecords (sql, vars, db);
}

bool snippet::wInitDb (const QString text, QSqlDatabase db/*=QSqlDatabase::database ()*/) const
{
    QString sql {qsl("INSERT OR IGNORE INTO %1 VALUES (?, ?, ?, ?) ").arg(tableName)};
    QVariant sKid = cId ? QVariant(cId) : QVariant(QVariant::LongLong);
    QVector<QVariant> vars {int(ls), int(lType), sKid, text};
    return executeSql_wNoRecords (sql, vars, db);
}

std::pair<QString, bool> snippet::read(QSqlDatabase db) const
{
    QString text;
    bool result =false;
    switch( type()) {
    case snippetType::allLettersAllKreditors:
        // date, greeting, food
        return read(ls, (letterType)allLetters, cId_allKreditors, db);
    case snippetType::allKreditors:
        // table, about: different for each letter, same for each creditor
        return read(ls, lType, cId_allKreditors, db);
    case snippetType::allLetters:
        // address, salut: different for each kreditor, fallback o cId =0  possible
        std::tie(text, result) =read(ls, (letterType)allLetters, cId, db);
        if( result)
            return {text, result};
        else
            return read(ls, (letterType)allLetters, cId_allKreditors, db);
    case snippetType::individual:
        // text1, text2: different for each letter and kreditor
        std::tie(text, result) =read(ls, lType, cId, db);
        if( result)
            return {text, result};
        else
            return read(ls, lType, cId_allKreditors, db);
    default:
        Q_ASSERT(not "invalid snippet Type");
        return {QString(), false};
    }
}

bool snippet::write(const QString& text, QSqlDatabase db) const
{
    switch( type()) {
    case snippetType::allLettersAllKreditors:
        // date, greeting, food
        return write(text, ls, (letterType)allLetters, cId_allKreditors, db);
    case snippetType::allKreditors:
        // table, about: different for each letter
        return write(text, ls, lType, cId_allKreditors, db);
    case snippetType::allLetters:
        // address, salut: different for each kreditor
        return write(text, ls, (letterType)allLetters, cId, db);
    case snippetType::individual:
        // text1, text2: different for each letter and kreditor
        return write(text, ls, lType, cId, db);
    default:
        Q_ASSERT(not "writing invalid snippet Type");
        return false;
    }
}

