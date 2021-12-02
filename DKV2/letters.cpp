#include "letters.h"

int fromLetterType(letterType lt)
{
    return (int)lt;
}

letterType letterTypeFromInt(int lt)
{
    if (lt < fromLetterType(letterType::maxValue) && lt >= 0)
    {
        return (letterType)lt;
    }
    Q_ASSERT("invalid letterType");
    return letterType::maxValue;
}

letters::letters()
{
}

////////// //////////////////////
//////// ///////////////////////
////// ////////////////////////
//// /////////////////////////
int writeDefaultSnippets(QSqlDatabase db)
{
    QVector<QPair<snippet, QString>> defaultSnippets = {
        {snippet(letterSnippet::date), qsl("{{gmbh.adresse1}} den {{datum}}")}, {snippet(letterSnippet::greeting), qsl("Mit freundlichen Grüßen")}, {snippet(letterSnippet::foot), qsl("<table width=100%><tr><td width=33%><small>Geschäftsführer*innen:<br>{{gmbh.gefue1}}<br>{{gmbh.gefue2}}<br>{{gmbh.gefue3}}</small></td><td width=33%></td><td width=33%></td></tr></table>")}, {snippet(letterSnippet::about, letterType::annPayoutL), qsl("Jahreszinsinformation {{Jahr}}")}, {snippet(letterSnippet::about, letterType::annReinvestL), qsl("Jahreszinsinformation {{Jahr}}")}, {snippet(letterSnippet::about, letterType::annInterestInfoL), qsl("Jahreszinsinformation {{Jahr}}")}, {snippet(letterSnippet::about, letterType::annInfoL), qsl("Jährliche Kreditinformation {{Jahr}}")}

        ,
        {snippet(letterSnippet::text1, letterType::annPayoutL), qsl("die Mitglieder des Wohnprojektes Esperanza wünschen ein schönes neues Jahr und bedanken sich herzlich für Deine Unterstützung.<p>"
                                                                    "Dies ist der Kontoauszug Deiner Direktkredite für das Jahr 2019 bei {{gmbh.adresse1}}. "
                                                                    "Vereinbarungsgemäß werden die Zinsen Deines Direktkredits in den nächsten Tagen ausgezahlt. Auf Wunsch erstellen wir eine gesonderte Zinsbescheinigung für die Steuererklärung.")},
        {snippet(letterSnippet::text1, letterType::annReinvestL), qsl("die Mitglieder des Wohnprojektes Esperanza wünschen ein schönes neues Jahr und bedanken sich herzlich für Deine Unterstützung.<p>"
                                                                      "Dies ist der Kontoauszug Deiner Direktkredite für das Jahr {{Jahr}} bei {{gmbh.adresse1}}. "
                                                                      "Vereinbarungsgemäß wurden die Zinsen Deinem Direktkredit Konto gut geschrieben. Auf Wunsch erstellen wir eine gesonderte Zinsbescheinigung für die Steuererklärung.")},
        {snippet(letterSnippet::text1, letterType::annInterestInfoL), qsl("die Mitglieder des Wohnprojektes Esperanza wünschen ein schönes neues Jahr und bedanken sich herzlich für Deine Unterstützung.<p>"
                                                                          "Vereinbarungsgemäß werden die Zinsen für Deinen Direktkredit bis zur Auszahlung des Kredits bei uns verwahrt.")},
        {snippet(letterSnippet::text1, letterType::annInfoL), qsl("die Mitglieder des Wohnprojektes Esperanza wünschen ein schönes neues Jahr und bedanken sich herzlich für Deine Unterstützung.<p>")}

        ,
        {snippet(letterSnippet::text2, letterType::annPayoutL), qsl("Soltest Du noch Fragen zu dieser Abrechnung haben, so zögere bitte nicht, Dich bei uns per Post oder E-Mail zu melden.<p>"
                                                                    "Wir hoffen auch in diesem Jahr auf Deine Solidarität. Für weitere Umschuldungen benötigen wir auch weiterhin Direktkredite. "
                                                                    "Empfehle uns Deinen Freund*innen und Verwandten.")},
        {snippet(letterSnippet::text2, letterType::annReinvestL), qsl("Soltest Du noch Fragen zu dieser Abrechnung haben, so zögere bitte nicht, Dich bei uns per Post oder E-Mail zu melden.<p>"
                                                                      "Wir hoffen auch in diesem Jahr auf Deine Solidarität. Für weitere Umschuldungen benötigen wir auch weiterhin Direktkredite. "
                                                                      "Empfehle uns Deinen Freund*innen und Verwandten.")},
        {snippet(letterSnippet::text2, letterType::annInterestInfoL), qsl("Soltest Du noch Fragen zu dieser Abrechnung haben, so zögere bitte nicht, Dich bei uns per Post oder E-Mail zu melden.<p>"
                                                                          "Wir hoffen auch in diesem Jahr auf Deine Solidarität. Für weitere Umschuldungen benötigen wir auch weiterhin Direktkredite. "
                                                                          "Empfehle uns Deinen Freund*innen und Verwandten.")},
        {snippet(letterSnippet::text2, letterType::annInfoL), qsl("Soltest Du noch Fragen zu Deinem Kredit haben, so zögere bitte nicht, Dich bei uns per Post oder E-Mail zu melden.<p>"
                                                                  "Wir hoffen auch in diesem Jahr auf Deine Solidarität. Für weitere Umschuldungen benötigen wir auch weiterhin Direktkredite. "
                                                                  "Empfehle uns Deinen Freund*innen und Verwandten.")}

        ,
        {snippet(letterSnippet::address, letterType::all), qsl("<small>{{gmbh.address1}} {{gmbh.address2}}<br>{{gmbh.strasse}}, <b>{{gmbh.plz}}</b> {{gmbh.stadt}}</small><p>"
                                                               "{{kreditoren.vorname}} {{kreditoren.nachname}} <p> {{kreditoren.strasse}} <br><b> {{kreditoren.plz}} </b> {{kreditoren.stadt}} <br><small> {{kreditoren.email}} </small>")},
        {snippet(letterSnippet::salut, letterType::all), qsl("Mit freundlichen Grüßen")}};
    int ret = 0;
    for (const auto &pair : qAsConst(defaultSnippets))
    {
        if (pair.first.wInitDb(pair.second, db))
            ret++;
        ;
    }
    return ret;
}

QVector<snippet> randomSnippets(int count)
{
    QRandomGenerator *rand = QRandomGenerator::system();
    QVector<snippet> v;
    for (int i = 0; i < count; i++)
    {
        const letterSnippet sid = letterSnippetFromInt(rand->bounded(200) % int(letterSnippet::maxValue));
        letterType lid;
        qlonglong kid;
        switch (snippet_type[sid])
        {
        case snippetType::allLettersAllKreditors:
            lid = letterType::all;
            kid = 0;
            break;
        case snippetType::allKreditors:
            kid = 0;
            lid = letterTypeFromInt(rand->bounded(100) % int(letterType::maxValue));
            break;
        case snippetType::allLetters:
            lid = letterType::all;
            kid = rand->bounded(count) + 1;
            break;
        case snippetType::individual:
            /* or */
        case snippetType::maxValue:
            lid = letterTypeFromInt(rand->bounded(100) % int(letterType::maxValue));
            kid = rand->bounded(count) + 1;
            break;
        }
        v.push_back(snippet(sid, lid, kid));
    }
    return v;
}
