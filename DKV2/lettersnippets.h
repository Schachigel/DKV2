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
letterType letterTypeFromInt( int i);

enum class snippetType {
    allLettersAllKreditors, // like Datum: same for all letters and all creditors
    allKreditors,           // like Betreff: different in each letter, same for all creditors
    allLetters,             // like Anrede: same for all letters, different for each creditor
    individual,     // like "Text 1": different for each letter, might differ for each creditor
    maxValue
};
snippetType snippetTypeFromInt( int i);

#define SNIPPETS \
    X(date,     "date",     snippetType::allLettersAllKreditors) \
    X(greeting, "greeting", snippetType::allLettersAllKreditors)  \
    X(food,     "food",     snippetType::allLettersAllKreditors)   \
    X(table,    "table",    snippetType::allKreditors) \
    X(about,    "about",    snippetType::allKreditors)            \
    X(text1,    "text1",    snippetType::individual)               \
    X(text2,    "text2",    snippetType::individual)             \
    X(address,  "address",  snippetType::allLetters)              \
    X(salut,    "salut",    snippetType::allLetters)               \
    X(maxValue, "n/a",      snippetType::allLettersAllKreditors) \

#define X(a, b, c) a,
enum class letterSnippet {
    SNIPPETS
};
#undef X

letterSnippet letterSnippetFromInt(int i);

#define X(a,b,c) qsl(b),
const QStringList snippetNames {
    SNIPPETS
};
#undef X

#define X(a,b,c) {letterSnippet::a, c},
const QMap<letterSnippet, snippetType> snippet_type {
    SNIPPETS
};
#undef X

struct snippet
{
    snippet(letterSnippet ls, letterType lType =letterType::all, qlonglong creditor =0) : ls(ls), lType (lType), cId(creditor) {};
    QString read(QSqlDatabase db=QSqlDatabase::database ());
    bool write(const QString& t, QSqlDatabase db=QSqlDatabase::database ());
    QString name() { return snippetNames[int(ls)]; };

    static const QString tableName;
    static const QString fnSnippet;
    static const QString fnLetter;
    static const QString fnCreditor;
    static const QString fnText;
    static const dbtable& getTableDef();
    static const QVector<QString> getIndexes();
private:
    static QString read  (const letterType lId, const letterSnippet sId, const qlonglong kId, QSqlDatabase db=QSqlDatabase::database ());
    static bool    write (const letterType lId, const letterSnippet sId, const qlonglong kId, const QString text, QSqlDatabase db=QSqlDatabase::database ());
    const letterSnippet ls;
    const letterType lType;
    const qlonglong cId;
};

#endif // LETTERSNIPPETS_H
