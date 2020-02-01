#include "letterTemplate.h"
#include "sqlhelper.h"

//enum sections{
//    dateFormat, projectAddress, projectUrl, Address, about, salutation,
//    mainText1, tableHeaderKennung, tableHeaderOldValue, tableHeaderInterest,
//    tableHeaderNewValue, mainText2, greeting, signee
//};
//enum distances{
//    topmost, overProjectAddress, projectAddressHeight, logoWidth, overAbout,
//    overSalutation, overText, tableMargin, overGreeting, overSignee


void letterTemplate::init_geldeingang()
{
    length[topmost] = 1.;
    length[overProjectAddress] = 10.;
    length[projectAddressHeight] = 42.;
    length[logoWidth] = 80.;
    length[overAbout] = 10.;
    length[overSalutation] = 5.;
    length[overText] = 5.;
    length[tableMargin] = 4.;
    length[overGreeting] = 10.;
    length[overSignee] = 10.;

    html[about] = "Bestätigung des Geldeingangs für den Direktkredit <b> {{Kennung}} </b>";
    html[mainText1] = "die Mitglieder des Wohnprojektes Esperanza bedanken sich herzlich für Deine Unterstützung.<p>"
                      "Mit diesem Schreiben bestätigen wir dir den Eingang Deiner Überweisung auf unserem Konto am {{Buchungsdatum}}. Damit beginnt die Zinsberechnung."
                      "Vereinbarungsgemäß werden die Zinsen {{Zinsbehandlung}}. {{Vertragsende}};"
                      "Wir werden Dich ab jetzt zum Begin jeden Jahres mit einem Kontoauszug/ Zinsbestätigung über die Entwicklung des DK auf dem Laufenden halten. ";
    html[tableHeaderKennung]  = "NOT used";
    html[tableHeaderOldValue] = "NOT used";
    html[tableHeaderInterest] = "NOT used";
    html[tableHeaderNewValue] = "NOT used";
    html[mainText2] = "Wenn Du Fragen zu Deinem Vertrag hast, zögere bitte nicht, Dich bei uns per Post oder E-Mail zu melden<p>"
                      "Wir hoffen auch in Zukunft auf Deine Solidarität, denn für weitere Umschuldungen benötigen wir weiterhin Direktkredite."
                      "Bitte empfehle uns Deinen Freund*innen und Verwandthen.";
}

void letterTemplate::init_JA_thesa()
{
    length[topmost] = 1.;
    length[overProjectAddress] = 10.;
    length[projectAddressHeight] = 42.;
    length[logoWidth] = 80.;
    length[overAbout] = 10.;
    length[overSalutation] = 5.;
    length[overText] = 5.;
    length[tableMargin] = 4.;
    length[overGreeting] = 10.;
    length[overSignee] = 10.;

    html[about] = "Kontoauszug Direktkredit(e) <b>Jahresabschluß {{jahr}} </b>";
    html[mainText1] = "die Mitglieder des Wohnprojektes Esperanza wünschen ein schönes neues Jahr und bedanken sich herzlich für Deine Unterstützung.<p>"
                      "Dies ist der Kontoauszug Deiner Direktkredite für das Jahr 2019 bei Esperanza Franklin GmbH. "
                      "Vereinbarungsgemäß wurden die Zinsen Deinem Direktkredit Konto gut geschrieben. Auf Wunsch erstellen wir eine gesonderte Zinsbescheinigung für die Steuererklärung.";
    html[tableHeaderKennung] = "<b> DK Kennung <b>";
    html[tableHeaderOldValue] = "<b> Vertragswert </b>";
    html[tableHeaderInterest] = "<b> Zinsen {{jahr}} </b>";
    html[tableHeaderNewValue] = "<b> Neuer Vertragswert </b>";
    html[mainText2] = "Wenn Du Fragen zu dieser Abrechnung hast, zögere bitte nicht, Dich bei uns per Post oder E-Mail zu melden<p>"
                      "Wir hoffen auch in diesem Jahr auf Deine Solidarität. Für weitere Umschuldungen benötigen wir weiterhin Direktkredite."
                      "Empfehle uns Deinen Freund*innen und Verwandthen.";
}

void letterTemplate::init_JA_auszahlend()
{
    length[topmost] = 1.;
    length[overProjectAddress] = 10.;
    length[projectAddressHeight] = 42.;
    length[logoWidth] = 80.;
    length[overAbout] = 10.;
    length[overSalutation] = 5.;
    length[overText] = 5.;
    length[tableMargin] = 4.;
    length[overGreeting] = 10.;
    length[overSignee] = 10.;

    html[about] = "Kontoauszug Direktkredit(e) <b>Jahresabschluß {{jahr}} </b>";
    html[mainText1] = "die Mitglieder des Wohnprojektes Esperanza wünschen ein schönes neues Jahr und bedanken sich herzlich für Deine Unterstützung.<p>"
                      "Dies ist der Kontoauszug Deiner Direktkredite für das Jahr 2019 bei Esperanza Franklin GmbH. "
                      "Vereinbarungsgemäß wurden die Zinsen Deinem Direktkredit Konto gut geschrieben. Auf Wunsch erstellen wir eine gesonderte Zinsbescheinigung für die Steuererklärung.";
    html[tableHeaderKennung] = "<b> DK Kennung <b>";
    html[tableHeaderOldValue] = "<b> Vertragswert </b>";
    html[tableHeaderInterest] = "<b> Zinsen {{jahr}} </b>";
    html[tableHeaderNewValue] = "<b> Neuer Vertragswert </b>";
    html[mainText2] = "Wenn Du Fragen zu dieser Abrechnung hast, zögere bitte nicht, Dich bei uns per Post oder E-Mail zu melden<p>"
                      "Wir hoffen auch in diesem Jahr auf Deine Solidarität. Für weitere Umschuldungen benötigen wir weiterhin Direktkredite."
                      "Empfehle uns Deinen Freund*innen und Verwandthen.";
}

void letterTemplate::init_Kontoabschluss()
{

}

letterTemplate::letterTemplate(letterTemplate::templateId id)
{
    html[dateFormat] = "Mannheim, den {{datum}}";
    html[projectAddress] = "Esperanza Franklin GmbH<p>Turley-Platz 9<br><b>68169 </b> Mannheim<br><small>info@esperanza-mannheim.de</small>";
    html[projectUrl] = "<small>www.esperanza-mannheim.de</small>";
    html[Address] ="{{vorname}} {{nachname}} <p> {{strasse}} <br><b> {{plz}} </b> {{stadt}} <br><small> {{email}} </small>";
    html[salutation] = "Liebe(r) {{vorname}}, ";
    html[greeting] = "Mit freundlichen Grüßen";
    html[signee] =  "Jutta Sichau<br><small>Direktkreditverwaltung</small>";

    switch(id)
    {
    case geldeingang:
        init_geldeingang();
        break;
    case JA_thesa:
        init_JA_thesa();
        break;
    case JA_auszahlend:
        init_JA_auszahlend();
        break;
    case Kontoabschluss:
        init_Kontoabschluss();
        break;
    default:
        break;
    }
}

bool letterTemplate::saveTemplate(const QString& templatename, const QString& con) const
{
    if( !sqlEnsureTable("Briefe"))
        return false;

    return true;
}
void letterTemplate::readTemplate(const QString& templateName, const QString& con)
{

}
