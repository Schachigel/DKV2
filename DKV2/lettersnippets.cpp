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
letterType letterTypeFromInt( int i) {
    Q_ASSERT(i<=int(letterType::maxValue));
    return static_cast<letterType>(i);
}

snippet::snippet(letterSnippet ls, letterType lT /*=letterType::all*/, qlonglong creditor /*=0*/)
    : ls(ls), lType (lT), cId(creditor)
{
    // ensure consistency
    switch(type()){
    case snippetType::allLettersAllKreditors:
        if( lType not_eq letterType::all || cId not_eq 0) {
            qDebug() << "resetting snippet attributes" << snippetNames[int(ls)];
            lType = letterType::all;
            cId = 0;
        }
        break;
    case snippetType::allKreditors:
        if( cId not_eq 0) {
            qDebug() << "resetting snippet attributes" << snippetNames[int(ls)];
            cId = 0;
        }
        if(lT == letterType::all) // there is no generic "Betreff" for all letters ...
            qCritical() << "wrong letter type for this snippetType" << snippetNames[int(ls)];
        break;
    case snippetType::allLetters:
        if( lType not_eq letterType::all) {
            qDebug() << "resetting snippet attributes" << snippetNames[int(ls)];
            lType = letterType::all;
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
        return read(ls, letterType::all, allKreditors, db);
    case snippetType::allKreditors:
        // table, about: different for each letter, same for each creditor
        return read(ls, lType, allKreditors, db);
    case snippetType::allLetters:
        // address, salut: different for each kreditor, fallback o cId =0  possible
        std::tie(text, result) =read(ls, letterType::all, cId, db);
        if( result)
            return {text, result};
        else
            return read(ls, letterType::all, allKreditors, db);
    case snippetType::individual:
        // text1, text2: different for each letter and kreditor
        std::tie(text, result) =read(ls, lType, cId, db);
        if( result)
            return {text, result};
        else
            return read(ls, lType, allKreditors, db);
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
        return write(text, ls, letterType::all, allKreditors, db);
    case snippetType::allKreditors:
        // table, about: different for each letter
        return write(text, ls, lType, allKreditors, db);
    case snippetType::allLetters:
        // address, salut: different for each kreditor
        return write(text, ls, letterType::all, cId, db);
    case snippetType::individual:
        // text1, text2: different for each letter and kreditor
        return write(text, ls, lType, cId, db);
    default:
        Q_ASSERT(not "writing invalid snippet Type");
        return false;
    }
}

////////// //////////////////////
//////// ///////////////////////
////// ////////////////////////
//// /////////////////////////
int writeDefaultSnippets(QSqlDatabase db)
{
    QVector<QPair<snippet, QString>> defaultSnippets = {
         { snippet(letterSnippet::date), qsl("{{gmbh.adresse1}} den {{datum}}")}
        ,{ snippet(letterSnippet::greeting), qsl("Mit freundlichen Grüßen")}
        ,{ snippet(letterSnippet::foot), qsl("<table width=100%><tr><td width=33%><small>Geschäftsführer*innen:<br>{{gmbh.gefue1}}<br>{{gmbh.gefue2}}<br>{{gmbh.gefue3}}</small></td><td width=33%></td><td width=33%></td></tr></table>")}
        ,{ snippet(letterSnippet::about, letterType::annPayoutL), qsl("Jahreszinsinformation {{Jahr}}")}
        ,{ snippet(letterSnippet::about, letterType::annReinvestL), qsl("Jahreszinsinformation {{Jahr}}")}
        ,{ snippet(letterSnippet::about, letterType::annInterestInfoL), qsl("Jahreszinsinformation {{Jahr}}")}
        ,{ snippet(letterSnippet::about, letterType::annInfoL), qsl("Jährliche Kreditinformation {{Jahr}}")}

        ,{ snippet(letterSnippet::text1, letterType::annPayoutL), qsl("die Mitglieder des Wohnprojektes Esperanza wünschen ein schönes neues Jahr und bedanken sich herzlich für Deine Unterstützung.<p>"
           "Dies ist der Kontoauszug Deiner Direktkredite für das Jahr 2019 bei {{gmbh.adresse1}}. "
           "Vereinbarungsgemäß werden die Zinsen Deines Direktkredits in den nächsten Tagen ausgezahlt. Auf Wunsch erstellen wir eine gesonderte Zinsbescheinigung für die Steuererklärung.")}
        ,{ snippet(letterSnippet::text1, letterType::annReinvestL), qsl("die Mitglieder des Wohnprojektes Esperanza wünschen ein schönes neues Jahr und bedanken sich herzlich für Deine Unterstützung.<p>"
           "Dies ist der Kontoauszug Deiner Direktkredite für das Jahr {{Jahr}} bei {{gmbh.adresse1}}. "
           "Vereinbarungsgemäß wurden die Zinsen Deinem Direktkredit Konto gut geschrieben. Auf Wunsch erstellen wir eine gesonderte Zinsbescheinigung für die Steuererklärung.")}
        ,{ snippet(letterSnippet::text1, letterType::annInterestInfoL), qsl("die Mitglieder des Wohnprojektes Esperanza wünschen ein schönes neues Jahr und bedanken sich herzlich für Deine Unterstützung.<p>"
           "Vereinbarungsgemäß werden die Zinsen für Deinen Direktkredit bis zur Auszahlung des Kredits bei uns verwahrt.")}
        ,{ snippet(letterSnippet::text1, letterType::annInfoL), qsl("die Mitglieder des Wohnprojektes Esperanza wünschen ein schönes neues Jahr und bedanken sich herzlich für Deine Unterstützung.<p>")}

        ,{ snippet(letterSnippet::text2, letterType::annPayoutL), qsl("Soltest Du noch Fragen zu dieser Abrechnung haben, so zögere bitte nicht, Dich bei uns per Post oder E-Mail zu melden.<p>"
           "Wir hoffen auch in diesem Jahr auf Deine Solidarität. Für weitere Umschuldungen benötigen wir auch weiterhin Direktkredite. "
           "Empfehle uns Deinen Freund*innen und Verwandten.")}
        ,{ snippet(letterSnippet::text2, letterType::annReinvestL), qsl("Soltest Du noch Fragen zu dieser Abrechnung haben, so zögere bitte nicht, Dich bei uns per Post oder E-Mail zu melden.<p>"
           "Wir hoffen auch in diesem Jahr auf Deine Solidarität. Für weitere Umschuldungen benötigen wir auch weiterhin Direktkredite. "
           "Empfehle uns Deinen Freund*innen und Verwandten.")}
        ,{ snippet(letterSnippet::text2, letterType::annInterestInfoL), qsl("Soltest Du noch Fragen zu dieser Abrechnung haben, so zögere bitte nicht, Dich bei uns per Post oder E-Mail zu melden.<p>"
           "Wir hoffen auch in diesem Jahr auf Deine Solidarität. Für weitere Umschuldungen benötigen wir auch weiterhin Direktkredite. "
           "Empfehle uns Deinen Freund*innen und Verwandten.")}
        ,{ snippet(letterSnippet::text2, letterType::annInfoL), qsl("Soltest Du noch Fragen zu Deinem Kredit haben, so zögere bitte nicht, Dich bei uns per Post oder E-Mail zu melden.<p>"
           "Wir hoffen auch in diesem Jahr auf Deine Solidarität. Für weitere Umschuldungen benötigen wir auch weiterhin Direktkredite. "
           "Empfehle uns Deinen Freund*innen und Verwandten.")}

        ,{ snippet(letterSnippet::address, letterType::all), qsl("<small>{{gmbh.address1}} {{gmbh.address2}}<br>{{gmbh.strasse}}, <b>{{gmbh.plz}}</b> {{gmbh.stadt}}</small><p>"
           "{{kreditoren.vorname}} {{kreditoren.nachname}} <p> {{kreditoren.strasse}} <br><b> {{kreditoren.plz}} </b> {{kreditoren.stadt}} <br><small> {{kreditoren.email}} </small>")}
        ,{ snippet(letterSnippet::salut, letterType::all), qsl("Mit freundlichen Grüßen")}
    };
    int ret =0;
    for (const auto& pair: qAsConst(defaultSnippets)) {
        if( pair.first.wInitDb(pair.second, db))
            ret++;;
    }
    return ret;
}


QVector<snippet> randomSnippets(int count)
{
    QRandomGenerator* rand =QRandomGenerator::system ();
    QVector<snippet> v;
    for( int i =0; i< count; i++) {
        const letterSnippet sid =letterSnippetFromInt(rand->bounded (200)%int(letterSnippet::maxValue));
        letterType lid;
        qlonglong kid;
        switch(snippet_type[sid])
        {
        case snippetType::allLettersAllKreditors:
            lid =letterType::all;
            kid =0;
            break;
        case snippetType::allKreditors:
            kid =0;
            lid =letterTypeFromInt(rand->bounded (100)%int(letterType::maxValue));
            break;
        case snippetType::allLetters:
            lid =letterType::all;
            kid = rand->bounded (count) +1;
            break;
        case snippetType::individual:
            /* or */
        case snippetType::maxValue:
            lid =letterTypeFromInt(rand->bounded (100)%int(letterType::maxValue));
            kid = rand->bounded (count) +1;
            break;
        }
        v.push_back (snippet(sid, lid, kid));
    }
    return v;
}
