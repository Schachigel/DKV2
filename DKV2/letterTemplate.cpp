
#include "helper.h"
#include "contract.h"
#include "dbstructure.h"
#include "appconfig.h"
#include "tabledatainserter.h"
#include "letterTemplate.h"

QMap<letterTemplate::elementType, QString> letterTemplate::all_elementTypes
{   { elementType::Adresse,            qsl("Adresse")},
    { elementType::Logo,               qsl("Logo")},
    { elementType::Datum,              qsl("Datum")},
    { elementType::Betreff,            qsl("Betreff")},
    { elementType::Anrede,             qsl("Anrede")},
    { elementType::Text1,              qsl("Text1")},
    { elementType::Abrechnungstabelle, qsl("Abrechnungsttabelle")},
    { elementType::Text2,              qsl("Text2")},
    { elementType::Gruss,              qsl("Gruss")},
    { elementType::Fuss,               qsl("Fuss")},
//    { elementType::maxId,              qsl("")}
};

QMap<letterTemplate::templId, QString> letterTemplate::all_templates
{   { templId::generic,                qsl("(alle)")},
    { templId::contractConclusion,     qsl("Vertragsabschluss")},
    { templId::activation,             qsl("Erster Geldeingang")},
    { templId::annualInterestReinvest, qsl("Jahreszinsanrechnung")},
    { templId::annualInterestPayout,   qsl("Jahreszinsauszahlung")},
    { templId::deposit,                qsl("Einzahlung")},
    { templId::payout,                 qsl("Auszahlung")},
    { templId::termination,            qsl("Kündigung")},
    { templId::accountClosing,         qsl("Vertragsende")},
//    { templId::maxId,                  qsl("")}
};

/* static */ dbtable letterTemplate::getTableDef_letterTypes()
{
    static dbtable letterTypesTable(qsl("Brieftypen"));
    if (0 == letterTypesTable.Fields().size()) {
        letterTypesTable.append(dbfield(qsl("id"), QVariant::Int).setNotNull().setPrimaryKey());
        letterTypesTable.append(dbfield(qsl("Brieftyp"), QVariant::String).setNotNull().setUnique());
    }
    return letterTypesTable;
}

/* static */ dbtable letterTemplate::getTableDef_elementTypes()
{
    static dbtable sectionsTable(qsl("BriefElementTypen"));
    if (0 == sectionsTable.Fields().size()) {
        sectionsTable.append(dbfield(qsl("id"), QVariant::Int).setNotNull().setPrimaryKey());
        sectionsTable.append(dbfield(qsl("ElementName"), QVariant::String).setNotNull().setUnique());
    }
    return sectionsTable;
}

/* static */ dbtable letterTemplate::getTableDef_letterElements()
{
    static dbtable letterTable("BriefElemente");
    if (0 == letterTable.Fields().size()) {
        letterTable.append(dbfield(contract::fnKreditorId, QVariant::LongLong)); // NOT setNotNull !! ...
        // ... is NULL for letter elements which are the same for all creditors
        letterTable.append(dbForeignKey(letterTable[contract::fnKreditorId],
                                        dkdbstructur[qsl("Kreditoren")][qsl("id")], ODOU_Action::CASCADE));

        letterTable.append(dbfield(qsl("BriefTypenId"), QVariant::Int).setNotNull());
        letterTable.append(dbForeignKey(letterTable[qsl("BriefTypenId")],
                                        dkdbstructur[qsl("Brieftypen")][qsl("id")]));

        letterTable.append(dbfield(qsl("BriefElementTypenId"), QVariant::Int).setNotNull());
        letterTable.append(dbForeignKey(letterTable[qsl("BriefElementTypenId")],
                                        dkdbstructur[qsl("BriefElementTypen")][qsl("id")]));

        letterTable.append(dbfield(qsl("Texte"), QVariant::String));

        QVector<dbfield> unique;
        unique.append(letterTable[contract::fnKreditorId]);
        unique.append(letterTable[qsl("BriefTypenId")]);
        unique.append(letterTable[qsl("BriefElementTypenId")]);
        letterTable.setUnique(unique);
    }
    return letterTable;
}

/* static */ bool letterTemplate::insert_letterTypes(QSqlDatabase db)
{ LOG_CALL;
    TableDataInserter tdi(dkdbstructur["Brieftypen"]);
    auto all_templates_keys =all_templates.keys();
    for (int i =0; i < all_templates_keys.size (); i++) {
        letterTemplate::templId key =all_templates_keys[i];
        QString value =all_templates.value (key);
        tdi.setValue(qsl("id"), int(key));
        tdi.setValue(qsl("Brieftyp"), value);
        if (-1 == tdi.InsertRecord(db))
            return false;
    }
    return true;
}

/* static */ bool letterTemplate::insert_elementTypes(QSqlDatabase db)
{ LOG_CALL;
    TableDataInserter tdi(dkdbstructur[qsl("BriefElementTypen")]);
    auto all_elementTypes_keys =all_elementTypes.keys();
    for (int i =0; i < all_elementTypes_keys.size (); i++) {
        letterTemplate::elementType key =all_elementTypes_keys[i];
        QString value =all_elementTypes.value(key);
        tdi.setValue(qsl("id"), int(key));
        tdi.setValue(qsl("ElementName"), value);
        if (-1 == tdi.InsertOrReplaceData(db))
            return false;
    }
    return true;
}

bool insertLetterElementFromMap(qlonglong kreditor, letterTemplate::templId briefTyp, QMap<letterTemplate::elementType, QString> map, QSqlDatabase db)
{ LOG_CALL;
    TableDataInserter tdi(dkdbstructur[qsl("BriefElemente")]);
    kreditor<=0 ? tdi.setValueToDefault(contract::fnKreditorId) : tdi.setValue(contract::fnKreditorId, kreditor);
    tdi.setValue(qsl("BriefTypenId"), QVariant(int(briefTyp)));
    bool res =true;
    auto map_keys =map.keys();
    for( int i =0; i < map_keys.size (); i++) {
        letterTemplate::elementType key =map_keys[i];
        QString value =map.value(key);
        tdi.setValue(qsl("BriefElementTypenId"), int(key));
        tdi.setValue(qsl("Texte"), value);
        if( -1 == tdi.InsertRecord(db)) {
            qDebug() << "Failded to insert generic letter Element " << map.value (letterTemplate::elementType(i));
            res = false;
        }
    }
    return res;

}

/* static */ bool letterTemplate::insert_letterElements(QSqlDatabase db)
{
    bool ret =true;
    QMap<elementType, QString> genericLetterElements {
        {elementType::Adresse, qsl("<small>{{gmbh.address1}} {{gmbh.address2}}<br>{{gmbh.strasse}}, <b>{{gmbh.plz}}</b> {{gmbh.stadt}}</small><p>"
                                   "{{kreditoren.vorname}} {{kreditoren.nachname}} <p> {{kreditoren.strasse}} <br><b> {{kreditoren.plz}} </b> {{kreditoren.stadt}} <br><small> {{kreditoren.email}} </small>")},
        {elementType::Logo,    qsl("<small>{{LOGO}}{{gmbh.url}}</small>")},
        {elementType::Datum,   qsl("{{gmbh.stadt}}, den {{datum}}")},
        {elementType::Anrede,  qsl("Liebe(r) {{Vorname}},")},
        {elementType::Gruss,   qsl("Mit freundlichen Grüßen<p>{{gmbh.dkKontakt}}")},
        {elementType::Fuss,    qsl("<table width=100%><tr>"
                                "<td width=33%><small>Geschäftsführer*innen:<br>{{gmbh.gefue1}}<br>{{gmbh.gefue2}}<br>{{gmbh.gefue3}}</small></td>"
                                "<td width=33%></td>"
                                "<td width=33%></td></tr></table>")}
    };
    ret = insertLetterElementFromMap(0, templId::generic, genericLetterElements, db);

    QMap<elementType, QString> interestReinvest_letterElements {
        {elementType::Betreff, qsl("Jahreszinsbescheid {{Jahr}}")},
        {elementType::Text1, qsl("Danke für Dein Vertrauen")},
        {elementType::Text2, qsl("gerne wieder, gerne mehr")}
    };
    ret &= insertLetterElementFromMap(0, templId::annualInterestReinvest, interestReinvest_letterElements, db);




    QMap<elementType, QString> interestPayout_letterElements {
        {elementType::Betreff, qsl("Jahreszinsauszahlung {{Jahr}}")},
        {elementType::Text1, qsl("Danke für Dein Vertrauen")},
        {elementType::Text2, qsl("gerne wieder, gerne mehr")}};
    ret &= insertLetterElementFromMap(0, templId::annualInterestPayout, interestPayout_letterElements, db);


    // insert letter parts which are different in different letter types
    // -> Betreff, Text1, Abrechnungstabelle, Text2

    return ret;
}

void letterTemplate::init_JA_thesa()
{   LOG_CALL;
    //html[about] = "Kontoauszug Direktkredit(e) <b>Jahresabschluß {{abrechnungsjahr}} </b>";
    //html[mainText1] = "die Mitglieder des Wohnprojektes Esperanza wünschen ein schönes neues Jahr und bedanken sich herzlich für Deine Unterstützung.<p>"
    //                  "Dies ist der Kontoauszug Deiner Direktkredite für das Jahr 2019 bei Esperanza Franklin GmbH. "
    //                  "Vereinbarungsgemäß wurden die Zinsen Deinem Direktkredit Konto gut geschrieben. Auf Wunsch erstellen wir eine gesonderte Zinsbescheinigung für die Steuererklärung.";
    //html[tableHeaderKennung] = "<b> {{tbh.kennung}} </b>";
    //html[tableHeaderOldValue] = "<b> {{tbh.old}} </b>";
    //html[tableHeaderInterest] = "<b> {{tbh.zins}} {{abrechnungsjahr}} </b>";
    //html[tableHeaderNewValue] = "<b> {{tbh.new}} </b>";
    //html[mainText2] = "Wenn Du Fragen zu dieser Abrechnung hast, zögere bitte nicht, Dich bei uns per Post oder E-Mail zu melden.<p>"
    //                  "Wir hoffen auch in diesem Jahr auf Deine Solidarität. Für weitere Umschuldungen benötigen wir weiterhin Direktkredite. "
    //                  "Empfehle uns Deinen Freund*innen und Verwandten.";
}

void letterTemplate::init_JA_auszahlend()
{   LOG_CALL;
 /*   html[about] = "Kontoauszug Direktkredit(e) <b>Jahresabschluß {{abrechnungsjahr}} </b>";
    html[mainText1] = "die Mitglieder des Wohnprojektes Esperanza wünschen ein schönes neues Jahr und bedanken sich herzlich für Deine Unterstützung.<p>"
                      "Dies ist der Kontoauszug Deiner Direktkredite für das Jahr 2019 bei Esperanza Franklin GmbH. "
                      "Vereinbarungsgemäß werden die Zinsen Deines Direktkredits in den nächsten Tagen ausgezahlt. Auf Wunsch erstellen wir eine gesonderte Zinsbescheinigung für die Steuererklärung.";
    html[tableHeaderKennung] = "<b> {{tbh.kennung}} <b>";
    html[tableHeaderOldValue] = "<b> {{tbh.old}} </b>";
    html[tableHeaderInterest] = "<b> {{tbh.Zins}} {{abrechnungsjahr}} </b>";
    html[tableHeaderNewValue] = "NOT used";
    html[mainText2] = "Wenn Du Fragen zu dieser Abrechnung hast, zögere bitte nicht, Dich bei uns per Post oder E-Mail zu melden.<p>"
                      "Wir hoffen auch in diesem Jahr auf Deine Solidarität. Für weitere Umschuldungen benötigen wir weiterhin Direktkredite. "
                      "Empfehle uns Deinen Freund*innen und Verwandten.";*/
}

void letterTemplate::init_Kontoabschluss()
{   LOG_CALL;
    //html[about] = "Beendigung Deines Direktkredites <b> {{vertraege.kennung}} </b>";
    //html[mainText1] = "wunschgemäß beenden wir Deinen Direktkredit zum {{vertraege.buchungsdatum}}.<p>"
    //                  "Der Kredit und die angelaufenen Zinsen werden in den kommenden Tagen auf das von Dir angegebene Konto mit der IBAN {{kreditoren.iban}} ausbezahlt. "
    //                  "Auf Wunsch erstellen wir eine gesonderte Zinsbescheinigung für die Steuererklärung.";
    //html[tableHeaderKennung]  = "<b> {{tbh.kennung}} </b>";
    //html[tableHeaderOldValue] = "<b> {{tbh.old}} </b>";
    //html[tableHeaderInterest] = "<b> {{tbh.zins}} </b>";
    //html[tableHeaderNewValue] = "<b> {{tbh.new}} </b>";
    //html[mainText2] = "Wenn Du Fragen zu dieser Abrechnung hast, zögere bitte nicht, Dich bei uns per Post oder E-Mail zu melden.<p>"
    //                  "Wir hoffen auch weiterhin auf Deine Solidarität und dass wir Dich bald wieder zu unseren Unterstüzern zählen können. "
    //                  "Denn für weitere Umschuldungen benötigen wir weiterhin Direktkredite.  Empfehle uns auch Deinen Freund*innen und Verwandten.";
}

void letterTemplate::init_Geldeingang()
{   LOG_CALL;
    //html[about] = "Bestätigung Deines Direktkredites <b> {{vertraege.kennung}} </b>";
    //html[mainText1] = "Hiermit bestätigen wir den Geldeingang von {{vertraege.betrag}} zum {{vertraege.buchungsdatum}}. Dein Vertrag wird unter der Kennung {{vertraege.kennung}} geführt.<p>"
    //                  "Herzlichen Dank für Deine Unterstützung. Wir werden Dich ab jetzt jährlich über die Zinsentwicklung Deines Kredits informieren.";
    //html[tableHeaderKennung] = "NOT used";
    //html[tableHeaderOldValue] = "NOT used";
    //html[tableHeaderInterest] = "NOT used";
    //html[tableHeaderNewValue] = "NOT used";
    //html[mainText2] = "Wenn Du Fragen zu Deinem Kredit hast, zögere bitte nicht, Dich bei uns per Post oder E-Mail zu melden.<p>"
    //                  "Wir hoffen auch weiterhin auf Deine Solidarität. Denn für weitere Umschuldungen benötigen wir auch weiterhin Direktkredite. "
    //                  "Empfehle uns auch Deinen Freund*innen und Verwandten.";
}

void letterTemplate::init_Kuendigung()
{   LOG_CALL;
    //html[about] = "Bestätigung der Kündigung Deines Direktkredites <b> {{vertraege.kennung}} </b>";
    //html[mainText1] = "Hiermit bestätigen wir den Eingang der Kündigung des Vertrags {{vertraege.betrag}} zum {{kuendigungsdatum}}.<p>"
    //                  "Das Vertragsende ergibt sich aus der Kündigungsfrist von {{vertraege.kfrist}} Monaten zum {{vertraege.laufzeitende}}";
    //html[tableHeaderKennung] = "NOT used";
    //html[tableHeaderOldValue] = "NOT used";
    //html[tableHeaderInterest] = "NOT used";
    //html[tableHeaderNewValue] = "NOT used";
    //html[mainText2] = "Wenn Du Fragen zu Deinem Kredit hast, zögere bitte nicht, Dich bei uns per Post oder E-Mail zu melden.<p>"
    //                  "Wir hoffen auch weiterhin auf Deine Solidarität. Denn für weitere Umschuldungen benötigen wir auch weiterhin Direktkredite. "
    //                  "Empfehle uns auch Deinen Freund*innen und Verwandten.";
}

void letterTemplate::init_defaults()
{   LOG_CALL;
    //html[sections::dateFormat] = "Mannheim, den {{datum}}";
    //html[projectAddress] = "{{gmbh.address1}}<p>{{gmbh.address2}}<br>{{gmbh.strasse}}<br><b>{{gmbh.plz}}</b> {{gmbh.stadt}}<br><small>{{gmbh.email}}</small>";
    //html[projectUrl] = "<small>{{gmbh.url}}</small>";
    //html[Address] ="{{kreditoren.vorname}} {{kreditoren.nachname}} <p> {{kreditoren.strasse}} <br><b> {{kreditoren.plz}} </b> {{kreditoren.stadt}} <br><small> {{kreditoren.email}} </small>";
    //html[salutation] = "Liebe(r) {{kreditoren.vorname}}, ";
    //html[greeting] = "Mit freundlichen Grüßen";
    //html[signee] =  "{{gmbh.dkKontakt}}<br><small>Direktkreditverwaltung</small>";

    switch(tid)
    {
    //case templateId::Geldeingang:
    //    init_Geldeingang();
    //    break;
    case templId::annualInterestReinvest:
        init_JA_thesa();
        break;
    case templId::annualInterestPayout:
        init_JA_auszahlend();
        break;
    //case templateId::Kontoabschluss:
    //    init_Kontoabschluss();
    //    break;
    //case templateId::Kuendigung:
    //    init_Kuendigung();
    //    break;
    default:
        qCritical() << "invalid template id?!";
        break;
    }
}

letterTemplate::letterTemplate( /*const qlonglong kreditor, */const letterTemplate::templId )
{   LOG_CALL;
    //tid=id;
    //if( not loadTemplate(id))
    //{
    //    init_defaults();
    //    saveTemplate();
    //}
}

void letterTemplate::setPlaceholder(QString var, QString val)
{   LOG_CALL;
    if( not placeholders.contains(var))
        qWarning() << "Unknown placeholder " << var << " set to value " << val;
    placeholders.insert (var, val);
}

//letterTemplate::templId letterTemplate::getIdFromName(QString n)
//{
//    LOG_CALL;
//    EnumArray<templId, int> ids;
//    for( auto i: ids)
//    {
//        if( getNameFromId(static_cast<letterTemplate::templId>(i)) == n)
//            return static_cast<letterTemplate::templId>(i);
//    }
//    return letterTemplate::templId::maxTemplateId;
//}
bool letterTemplate::saveDefaultTemplate() const
{
    return letterTemplate::saveTemplate(-1);
}
bool letterTemplate::saveTemplate(qlonglong /*kreditor*/) const
{   LOG_CALL;
    if( not tableExists("Briefvorlagen"))
        return false;
    return true;
}

bool letterTemplate::loadTemplate(letterTemplate::templId /*id*/, qlonglong /*kreditor*/)
{   LOG_CALL;
    //if( not tableExists("Briefvorlagen"))
    //{
    //    qDebug() << "Tabelle mit Briefvorlagen existiert nicht, Template Daten können nicht gespeichert werden";
    //    return false;
    //}
    //tid = id;
    //QString q = selectQueryFromFields( dkdbstructur["Briefvorlagen"].Fields(), QVector<dbForeignKey>(), "templateId ='" + templName(tid) + qsl("'"));
    //QSqlQuery query;
    //query.prepare(q);
    //if( not query.exec())
    //{
    //    qDebug() << "reading template from DB failed: " << query.lastError();
    //    return false;
    //}
    //query.last();
    //if( query.at() <= 0)
    //{
    //    qDebug() << "reading template from DB failed: " << "result size was " << query.size();
    //    return false;
    //}
    //query.first();
    //while(query.next())
    //{
    //    int prop = query.value(dkdbstructur["Briefvorlagen"].Fields()[1].name()).toInt();
    //    QString sValue = query.value(dkdbstructur["Briefvorlagen"].Fields()[2].name()).toString();
    //    if( prop >999)
    //        length[prop] = sValue.toInt();
    //    else
    //        html[prop] = sValue;
    //}
    return true;
}

bool letterTemplate::operator ==(const letterTemplate &) const
{   LOG_CALL;
    //if(tid not_eq b.tid) return false;
    //if( length.count() not_eq b.length.count()) return false;
    //if( html.count()   not_eq b.html.count()) return false;
    //for( int i = 0; i < maxDistance; i++)
    //{
    //    if( b.length[i] not_eq length[i])
    //        return false;
    //}
    //for( int i = 0; i < maxSection; i++)
    //{
    //    if( b.html[i] not_eq html[i])
    //        return false;
    //}
    return true;
}

bool letterTemplate::applyPlaceholders()
{   LOG_CALL;
    bool ret = true;
    for( int i=0; i < html.count(); i++)
    {
        QString text = html[i];
        while( text.contains("{{"))
        {
            int open = text.indexOf("{{");
            if( open == -1) break;
            int close = text.indexOf("}}", open);
            if( close == -1)
            {
                ret = false;
                qCritical() << "ERROR: {{ indicator, but no closing }}";
                text = text.left(open) + ".." + text.right(text.length()-open -2);
                break;
            }
            QString ph = text.mid(open+2, close -open -2);
            auto it = placeholders.find(ph);
            if( it == placeholders.end())
            {
                ret = false;
                qCritical() << "ERROR: unknown placeholder found in html: " << ph;
                text = text.left(open) + text.right(text.length()-close-2);
            }
            else
                text = text.left(open) + it.value() + text.right(text.length()-close -2);
        } // Eo While
        html[i] = text;
    }
    return ret;
}

bool letterTemplate::createDocument(QTextDocument& )
{   LOG_CALL;
//    QImage img1(":/res/weiss.png");
//    QImage img2(":/res/logo.png");
//    static QVariant vImg1(img1);
//    static QVariant vImg2(img2);
//    doc.addResource(QTextDocument::ImageResource,QUrl("weiss.png"),QVariant(vImg1));
//    doc.addResource(QTextDocument::ImageResource,QUrl("logo.png"),QVariant(vImg2));
//
////    doc.setPageSize(printer->paperSize(QPrinter::Point));
//    doc.setDocumentMargin(20 *mmInPt);
//
//    //some helper
//    QTextImageFormat spacer;
//    spacer.setName("weiss.png");
//    spacer.setHeight(5. *mmInPt); // set the hight of this part of the page
//    spacer.setWidth (.01 *mmInPt); // invidible
//
//    enum _side {left =0, right =1};
//
//    // general settinog
//    QTextOption to(Qt::AlignJustify); to.setWrapMode(QTextOption::WordWrap);
//    doc.setDefaultTextOption(to);
//    // Font family
//    QFont font(fontFamily, 11*fontOutputFactor);
//    font.setKerning(false);
//    doc.setDefaultFont(font);
//
//    // the documents content starts HERE
//    QTextCursor c = doc.rootFrame()->firstCursorPosition();
//    // page structure in table:
//    QTextTableFormat tableFormat;
//    tableFormat.setBorder( 0.);
//    tableFormat.setCellPadding(0.);
//    tableFormat.setMargin(0.);
//    // column width 2x 50%, logo on the left
//    QTextLength cw (QTextLength::PercentageLength, 50);
//    QVector<QTextLength> cwc{cw, cw};
//    tableFormat.setColumnWidthConstraints(cwc);
//    // build the rows
//    QTextTable* tt = c.insertTable(16, 2, tableFormat);
//    // start with 1 line of nothing
//    spacer.setHeight(length[topmost] *mmInPt);
//    c.insertImage(spacer, QTextFrameFormat::Position::FloatRight);
//    c.movePosition(c.NextRow);
//
//    // space + project address
//    spacer.setHeight(length[projectAddressHeight] *mmInPt);
//    c.insertImage(spacer, QTextFrameFormat::Position::FloatRight);
//    spacer.setHeight(length[overProjectAddress]* mmInPt);
//    c.insertImage(spacer, QTextFrameFormat::Position::InFlow);
//    c.insertBlock();
//    c.insertHtml(html[projectAddress]);
//
//    // top right: LOGO
//    c.movePosition(c.NextCell);
//    QTextBlockFormat bfUrl = c.blockFormat();
//    bfUrl.setAlignment(Qt::AlignRight);
//    c.setBlockFormat(bfUrl);
//    QTextImageFormat tfLogo;
//    tfLogo.setWidth(length[logoWidth] *mmInPt);
//    tfLogo.setName("logo.png");
//    c.insertImage(tfLogo, QTextFrameFormat::Position::InFlow);
//    c.insertBlock();
//    c.insertHtml(html[projectUrl]);
//
//    c.movePosition(c.NextRow);c.movePosition(c.NextCell); // Row index 2: date
//    QTextBlockFormat bfDate = c.blockFormat();
//    bfDate.setAlignment(Qt::AlignRight|Qt::AlignBottom);
//    c.setBlockFormat(bfDate);
//    c.insertText(html[dateFormat]);
//
//    c.movePosition(c.NextRow);
//    c.insertHtml(html[Address]);
//
//    c.movePosition(c.NextRow);
//    spacer.setHeight( length[overAbout] *mmInPt);
//    c.insertImage(spacer, QTextFrameFormat::FloatRight);
//
//    c.movePosition(c.NextRow);
//    tt->mergeCells(tt->cellAt(c).row(), tt->cellAt(c).column(), 1, 2);
//    c.insertHtml(html[about]);
//
//    c.movePosition(c.NextRow);
//    // Space to the salutation
//    spacer.setHeight(length[overSalutation] *mmInPt);
//    c.insertImage(spacer, QTextFrameFormat::FloatRight);
//
//    c.movePosition(c.NextRow);
//    tt->mergeCells(tt->cellAt(c).row(), tt->cellAt(c).column(), 1, 2);
//    // Salutiation
//    c.insertHtml(html[salutation]);
//
//    c.movePosition(c.NextRow);
//    // Space to main text
//    spacer.setHeight(length[overText] *mmInPt);
//    c.insertImage(spacer, QTextFrameFormat::FloatRight);
//
//    c.movePosition(c.NextRow);
//    // Main text (before table)
//    tt->mergeCells(tt->cellAt(c).row(), tt->cellAt(c).column(), 1, 2);
//    c.insertHtml(html[mainText1]);
//
//    c.movePosition(c.NextRow);
//    // Row index 8: table
//    tt->mergeCells(tt->cellAt(c).row(), tt->cellAt(c).column(), 1, 2);
//
//    QTextBlockFormat bfAlignCellLeft = c.blockFormat();
//    bfAlignCellLeft.setAlignment(Qt::AlignLeft);
//    QTextBlockFormat bfAlignCellCenter = c.blockFormat();
//    bfAlignCellCenter.setAlignment(Qt::AlignCenter);
//    c.setBlockFormat(bfAlignCellCenter);
//    if( html[tableHeaderKennung].isEmpty() || html[tableHeaderKennung].compare("NOT used", Qt::CaseSensitivity::CaseInsensitive))
//    {
//        //    QTextTableFormat tfDks;
//        //    tfDks.setBorder(0.);
//        //    tfDks.setCellPadding(1 *mmInPt);
//        //    tfDks.setMargin(tableMargin *mmInPt);
//        //    tfDks.setLeftMargin(20 *mmInPt);
//        //    c.insertTable(3, 4, tfDks);
//        //    QTextTable* ttDks  = c.currentTable();
//        //    c = ttDks->cellAt(0,0).firstCursorPosition(); c.setBlockFormat(bfAlignCellLeft);
//        //    c.insertHtml(html[tableHeaderKennung]);
//        //    c.movePosition(c.NextCell); c.setBlockFormat(bfAlignCellCenter);
//        //    c.insertHtml(html[tableHeaderOldValue]);
//        //    c.movePosition(c.NextCell); c.setBlockFormat(bfAlignCellCenter);
//        //    c.insertHtml(html[tableHeaderInterest]);
//        //    c.movePosition(c.NextCell); c.setBlockFormat(bfAlignCellCenter);
//        //    c.insertHtml(html[tableHeaderNewValue]);
//        //    c.movePosition(c.NextRow); c.setBlockFormat(bfAlignCellLeft);
//        //    c.insertHtml("DK-ESP-2018-024");
//        //    c.movePosition(c.NextCell); c.setBlockFormat(bfAlignCellCenter);
//        //    c.insertHtml("12.300 Euro");
//        //    c.movePosition(c.NextCell); c.setBlockFormat(bfAlignCellCenter);
//        //    c.insertHtml("123 Euro");
//        //    c.movePosition(c.NextCell); c.setBlockFormat(bfAlignCellCenter);
//        //    c.insertHtml("12.423 Euro");
//        //    c.movePosition(c.NextRow); c.setBlockFormat(bfAlignCellLeft);
//        //    c.insertHtml("DK-ESP-2019-054");
//        //    c.movePosition(c.NextCell); c.setBlockFormat(bfAlignCellCenter);
//        //    c.insertHtml("10.100 Euro");
//        //    c.movePosition(c.NextCell); c.setBlockFormat(bfAlignCellCenter);
//        //    c.insertHtml("101 Euro");
//        //    c.movePosition(c.NextCell); c.setBlockFormat(bfAlignCellCenter);
//        //    c.insertHtml("10.201 Euro");
//        //    // step out of inner table
//        //    c.movePosition(c.NextBlock);
//    }
//
//    c.movePosition(c.NextRow);
//    // Row index 9: Main text after table
//    tt->mergeCells(tt->cellAt(c).row(), tt->cellAt(c).column(), 1, 2);
//    c.insertHtml(html[mainText2] );
//
//    c.movePosition(c.NextRow);
//    // Row index 6: Space to greeting
//    spacer.setHeight(length[overGreeting] *mmInPt);
//    c.insertImage(spacer, QTextFrameFormat::FloatRight);
//
//    c.movePosition(c.NextRow);
//    // Row index 10: Grußformel
//    c.insertHtml(html[greeting]);
//
//    c.movePosition(c.NextRow);
//    // Row index 6: Space to signee
//    spacer.setHeight(length[overSignee] *mmInPt);
//    c.insertImage(spacer, QTextFrameFormat::FloatRight);
//
//    c.movePosition(c.NextRow);
//    c.insertHtml(html[signee]);
//    return true;
//}
//
//bool letterTemplate::createPdf(QString file, const QTextDocument& doc)
//{   LOG_CALL;
//    QFile::remove(file);
//    if( QFile::exists(file))
//    {
//        qDebug() << "pdf file creation failed";
//        return false;
//    }
//
//#ifdef QT_DEBUG
//    QString testhtml = doc.toHtml();
//    QFile htmlfile(file + ".html");
//    htmlfile.open(QIODevice::WriteOnly);
//#endif
//
////    printer->setOutputFileName(file);
//#ifdef QT_DEBUG
//    int written = htmlfile.write(testhtml.toUtf8());
//    if( written <1)
//        qDebug() << "html not written" << htmlfile.errorString();
//#endif
//    doc.print(printer);
    return true;
}

QString letterTemplate::fileNameFromId(const QString& contractId)
{   LOG_CALL;

    QString outputfile = appConfig::Outdir();
    outputfile += "/";
    outputfile += QDate::currentDate().toString("yyyy-MM-dd_") + all_templates[tid] + "_" +contractId.trimmed() +".pdf";
    qDebug() << "printing to " << outputfile;
    return outputfile;
}
