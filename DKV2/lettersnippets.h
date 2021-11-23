#ifndef LETTERSNIPPETS_H
#define LETTERSNIPPETS_H

#include "helper.h"
#include "contract.h"


static const qlonglong allKreditors =0;

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
    X(foot,     "foot",     snippetType::allLettersAllKreditors)   \
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
    snippet(letterSnippet ls, letterType lType =letterType::all, qlonglong creditor =0);

    QString name()     const { return snippetNames[int(ls)]; };
    snippetType type() const { return snippet_type[ls]; };

    std::pair<QString, bool> read(QSqlDatabase db=QSqlDatabase::database ()) const;
    bool write(const QString& t, QSqlDatabase db=QSqlDatabase::database ()) const;
    bool wInitDb (const QString text, QSqlDatabase db=QSqlDatabase::database ()) const;

    static const QString tableName, fnSnippet, fnLetter, fnCreditor, fnText;
    static const dbtable& getTableDef();
    static const QVector<QString> getIndexes();
private:
    static std::pair<QString, bool> read  (const letterSnippet sId, const letterType lId, const qlonglong kId, QSqlDatabase db=QSqlDatabase::database ());
    static bool    write (const QString text, const letterSnippet sId, const letterType lId, const qlonglong kId, QSqlDatabase db=QSqlDatabase::database ());
    const letterSnippet ls;
    letterType lType;
    qlonglong cId;
};

/// init for new db
int writeDefaultSnippets(QSqlDatabase db =QSqlDatabase::database ());

// for testing
QVector<snippet> randomSnippets(int count);


#endif // LETTERSNIPPETS_H
