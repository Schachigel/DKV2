#ifndef LETTERSNIPPETS_H
#define LETTERSNIPPETS_H

#include "helper.h"
#include "contract.h"


enum class letterType {
    all =0,
    annPayoutL        =int(interestModel::payout)   +1,
    annReinvestL      =int(interestModel::reinvest) +1,
    annInterestInfoL  =int(interestModel::fixed)    +1,
    annInfoL          =int(interestModel::zero)     +1,
    maxValue
};

enum snippetType {
    allLettersAllKreditors, // like Datum: same for all letters and all creditors
    allKreditors,           // like Betreff: different in each letter, same for all creditors
    allLetters,             // like Anrede: same for all letters, different for each creditor
    individual              // like "Text 1": different for each letter, might differ for each creditor
};

#define SNIPPETS \
    X(date,     "date",     snippetType::allLettersAllKreditors) \
    X(greeting, "greeting", snippetType::allLettersAllKreditors)  \
    X(food,     "food",     snippetType::allLettersAllKreditors)   \
    X(table,    "table",    snippetType::allLettersAllKreditors) \
    X(about,    "about",    snippetType::allKreditors)            \
    X(text1,    "text1",    snippetType::individual)               \
    X(text2,    "text2",    snippetType::individual)             \
    X(address,  "address",  snippetType::allLetters)              \
    X(salut,    "salut",    snippetType::allLetters)               \

#define X(a, b, c) a,
enum letterSnippet {
    SNIPPETS
};
#undef X

#define X(a,b,c) qsl(b),
const QStringList snippetNames {
    SNIPPETS
};
#undef X

#define X(a,b,c) {a, c},
const QMap<letterSnippet, snippetType> snippet_type {
    SNIPPETS
};
#undef X

struct Snippet
{
    Snippet(letterSnippet ls, letterType lType =letterType::all, qlonglong creditor =0) : ls(ls), lType (lType), cId(creditor) {};
    QString read(QSqlDatabase db=QSqlDatabase::database ());
    bool write(const QString& t, QSqlDatabase db=QSqlDatabase::database ());
    QString name() { return snippetNames[ls]; };

    static const QString tableName;
    static const QString fnSnippet;
    static const QString fnLetter;
    static const QString fnCreditor;
    static const dbtable& getTableDef();
private:
    QString load  (letterSnippet sId, letterType lId, creditor kId, QSqlDatabase db=QSqlDatabase::database ());
    bool    write (const letterSnippet sId, const letterType lId, const creditor kId, QSqlDatabase db=QSqlDatabase::database ());
    letterSnippet ls;
    letterType lType;
    qlonglong cId;
};

#endif // LETTERSNIPPETS_H
