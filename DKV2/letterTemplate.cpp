#include "letterTemplate.h"
#include "dkdbhelper.h"
#include "sqlhelper.h"

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
                      "Vereinbarungsgemäß werden die Zinsen Deines Direktkredits in den nächsten Tagen ausgezahlt. Auf Wunsch erstellen wir eine gesonderte Zinsbescheinigung für die Steuererklärung.";
    html[tableHeaderKennung] = "<b> DK Kennung <b>";
    html[tableHeaderOldValue] = "<b> Vertragswert </b>";
    html[tableHeaderInterest] = "<b> Zinsen {{jahr}} </b>";
    html[tableHeaderNewValue] = "NOT used";
    html[mainText2] = "Wenn Du Fragen zu dieser Abrechnung hast, zögere bitte nicht, Dich bei uns per Post oder E-Mail zu melden<p>"
                      "Wir hoffen auch in diesem Jahr auf Deine Solidarität. Für weitere Umschuldungen benötigen wir weiterhin Direktkredite."
                      "Empfehle uns Deinen Freund*innen und Verwandthen.";
}

void letterTemplate::init_Kontoabschluss()
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

    html[about] = "Beendigung Deines Direktkredites <b> {{Vertraege.Kennung}} </b>";
    html[mainText1] = "wunschgemäß beenden wir Deinen Direktkredit zum {{Vertraege.Buchungsdatum}}.<p>"
                      "Der Kredit und die angelaufenen Zinsen werden in den kommenden Tagen auf das von Dir angegebene Konto mit der IBAN {{Kerditoren.IBAN}} ausbezahlt. "
                      "Auf Wunsch erstellen wir eine gesonderte Zinsbescheinigung für die Steuererklärung.";
    html[tableHeaderKennung] = "<b> DK Kennung <b>";
    html[tableHeaderOldValue] = "<b> Vertragswert </b>";
    html[tableHeaderInterest] = "<b> Zinsen </b>";
    html[tableHeaderNewValue] = "NOT used";
    html[mainText2] = "Wenn Du Fragen zu dieser Abrechnung hast, zögere bitte nicht, Dich bei uns per Post oder E-Mail zu melden<p>"
                      "Wir hoffen auch weiterhin auf Deine Solidarität und dass wir Dich bald wieder zu unseren Unterstüzern zählen können."
                      " Denn für weitere Umschuldungen benötigen wir weiterhin Direktkredite.  Empfehle uns auch Deinen Freund*innen und Verwandthen.";
}

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

    html[about] = "Bestätigung Deines Direktkredites <b> {{Vertraege.Kennung}} </b>";
    html[mainText1] = "Hiermit bestätigen wir den Geldeingang von {{Vertraege.Betrag}} zum {{Vertraege.Buchungsdatum}}.<p>"
                      "Herzlichen Dank für Deine Unterstützung. Wir werden Dich ab jetzt jährlich über die Zinsentwicklung Deines Kredits informieren.";
    html[tableHeaderKennung] = "NOT used";
    html[tableHeaderOldValue] = "NOT used";
    html[tableHeaderInterest] = "NOT used";
    html[tableHeaderNewValue] = "NOT used";
    html[mainText2] = "Wenn Du Fragen zu Deinem Kredit hast, zögere bitte nicht, Dich bei uns per Post oder E-Mail zu melden<p>"
                      "Wir hoffen auch weiterhin auf Deine Solidarität. Denn für weitere Umschuldungen benötigen wir auch weiterhin Direktkredite."
                      "Empfehle uns auch Deinen Freund*innen und Verwandthen.";
}

letterTemplate::letterTemplate(letterTemplate::templateId id)
{
    tid=id;
    html[sections::dateFormat] = "Mannheim, den {{datum}}";
    html[projectAddress] = "Esperanza Franklin GmbH<p>Turley-Platz 9<br><b>68169 </b> Mannheim<br><small>info@esperanza-mannheim.de</small>";
    html[projectUrl] = "<small>www.esperanza-mannheim.de</small>";
    html[Address] ="{{Kreditoren.Vorname}} {{Kreditoren.Nachname}} <p> {{Kreditoren.Strasse}} <br><b> {{Kreditoren.Plz}} </b> {{Kreditoren.Stadt}} <br><small> {{email}} </small>";
    html[salutation] = "Liebe(r) {{Kreditoren.Vorname}}, ";
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

QString letterTemplate::getNameFromId(templateId id)
{
    switch(id)
    {
    case geldeingang:
        return "Geldeingang";
        break;
    case JA_thesa:
        return "JA_thesaurierend";
        break;
    case JA_auszahlend:
        return "JA_auszahlend";
        break;
    case Kontoabschluss:
        return "Kontoabschluss";
        break;
    default:
        Q_ASSERT("false lettertemplate id");
        return QString();
        break;
    }
}

letterTemplate::templateId letterTemplate::getIdFromName(QString n)
{
    for( int i = 0; i < letterTemplate::templateId::maxTemplateId; i++)
    {
        letterTemplate::templateId id=(letterTemplate::templateId)i;
        if( getNameFromId(id) == n)
            return id;
    }
    return letterTemplate::templateId::maxTemplateId;
}

bool letterTemplate::saveTemplate(const QString& con) const
{
    QSqlDatabase db = QSqlDatabase::database(con);
    if( !ensureTable(dkdbAddtionalTables["Briefvorlagen"], db))
        return false;

    for( int i = 0; i < letterTemplate::sections::maxSection; i++)
    {
        TableDataInserter tdi(  dkdbAddtionalTables["Briefvorlagen"]);
        tdi.setValue("templateId", QVariant(tid) );
        tdi.setValue("Eigenschaft", QVariant(i));
        tdi.setValue("Wert", QVariant(html[i]));
        if( -1 == tdi.InsertOrReplaceData(db))
        {
            qDebug() << "failed to write template data: " << i << ": " << html[i];
            return false;
        }
    }
    for( int i = topmost; i < letterTemplate::distances::maxDistance; i++)
    {
        TableDataInserter tdi(  dkdbAddtionalTables["Briefvorlagen"]);
        tdi.setValue("templateId", QVariant(tid) );
        tdi.setValue("Eigenschaft", QVariant(i));
        tdi.setValue("Wert", QVariant(QString::number(length[i])));
        if( -1 == tdi.InsertOrReplaceData(db))
        {
            qDebug() << "failed to write template data: " << i << ": " << length[i];
            return false;
        }
    }
    return true;
}
bool letterTemplate::loadTemplate(letterTemplate::templateId id, const QString& con)
{
    if( !tableExists("Briefvorlagen", con))
    {
        qDebug() << "Tabelle mit Briefvorlagen existiert nicht, Template Daten können nicht gespeichert werden";
        return false;
    }
    tid = id;
    QString q = SelectQueryFromFields( dkdbAddtionalTables["Briefvorlagen"].Fields(), "[templateId] == '" + QString::number(tid) + "'");
    QSqlQuery query(QSqlDatabase::database(con));
    query.prepare(q);
    if( !query.exec())
    {
        qDebug() << "reading template from DB failed" << query.lastError();
        return false;
    }
    while(query.next())
    {
        int prop = query.value("Eigenschaft").toInt();
        QString sValue = query.value("Wert").toString();
        if( prop >999)
            length[prop] = sValue.toInt();
        else
            html[prop] = sValue;
    }
    return true;
}

bool letterTemplate::operator ==(const letterTemplate &b) const
{
    if(tid != b.tid) return false;
    if( length.count() != b.length.count()) return false;
    if( html.count()   != b.html.count()) return false;
    for( int i = 0; i < maxDistance; i++)
    {
        if( b.length[i] != length[i]) return false;
    }
    for( int i = 0; i < maxSection; i++)
    {
        if( b.html[i] != html[i]) return false;
    }
    return true;
}
