#include "pch.h"
#include "creditor.h"
#include "lettersnippets.h"

/*static*/ const QString Snippet::tableName  {qsl("BriefElemente")};
/*static*/ const QString Snippet::fnSnippet  {qsl("Element")};
/*static*/ const QString Snippet::fnLetter   {qsl("Brief")};
/*static*/ const QString Snippet::fnCreditor {qsl("Kid")};

/*static*/ const dbtable& Snippet::getTableDef()
{
    static dbtable snippetstable(tableName);
    if( 0 == snippetstable.Fields().size()) {
        snippetstable.append(dbfield(fnSnippet, QVariant::Int));
        snippetstable.append(dbfield(fnLetter,  QVariant::Int));
        snippetstable.append(dbfield(fnCreditor, QVariant::Int));
        snippetstable.append(dbForeignKey(snippetstable[fnCreditor],
                     creditor::getTableDef().Name(), qsl("id"), qsl("ON DELETE CASCADE")));
        QVector<dbfield> unique {snippetstable[fnSnippet], snippetstable[fnLetter], snippetstable[fnCreditor]};
        snippetstable.setUnique (unique);
    }
    return snippetstable;
}

QString Snippet::load  (const letterSnippet sId, const letterType lId, const creditor kId, QSqlDatabase db/*=QSqlDatabase::database ()*/)
{
    return qsl("");
}

bool    Snippet::write (const letterSnippet sId, const letterType lId, const creditor kId, QSqlDatabase db/*=QSqlDatabase::database ()*/)
{
    return false;
}
