#include <QSettings>
#include <QTextDocument>
#include <QTextImageFormat>
#include <QTextFrame>
#include <QTextCursor>
#include <QTextTable>
#include <QImage>

#include "helper.h"
#include "letterTemplate.h"
#include "dkdbhelper.h"
#include "sqlhelper.h"

QPrinter* letterTemplate::printer =nullptr;

void letterTemplate::initPrinter()
{LOG_ENTRY_and_EXIT;
    if( printer) return;
    printer = new QPrinter(QPrinter::HighResolution);

    printer->setOutputFormat(QPrinter::PdfFormat);
    printer->setPaperSize(QPrinter::A4);
    printer->setPageMargins(0., 0., 0., 0., QPrinter::Millimeter);
    printer->setMargins({0.,0.,0.,0.});
    //QPagedPaintDevice::Margins m = printer.margins();
    printer->setFullPage(true);
}

void letterTemplate::init_JA_thesa()
{LOG_ENTRY_and_EXIT;
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

    html[about] = "Kontoauszug Direktkredit(e) <b>Jahresabschluß {{abrechnungsjahr}} </b>";
    html[mainText1] = "die Mitglieder des Wohnprojektes Esperanza wünschen ein schönes neues Jahr und bedanken sich herzlich für Deine Unterstützung.<p>"
                      "Dies ist der Kontoauszug Deiner Direktkredite für das Jahr 2019 bei Esperanza Franklin GmbH. "
                      "Vereinbarungsgemäß wurden die Zinsen Deinem Direktkredit Konto gut geschrieben. Auf Wunsch erstellen wir eine gesonderte Zinsbescheinigung für die Steuererklärung.";
    html[tableHeaderKennung] = "<b> {{tbh.kennung}} <b>";
    html[tableHeaderOldValue] = "<b> {{tbh.old}} </b>";
    html[tableHeaderInterest] = "<b> {{tbh.zins}} {{abrechnungsjahr}} </b>";
    html[tableHeaderNewValue] = "<b> {{tbh.new}} </b>";
    html[mainText2] = "Wenn Du Fragen zu dieser Abrechnung hast, zögere bitte nicht, Dich bei uns per Post oder E-Mail zu melden.<p>"
                      "Wir hoffen auch in diesem Jahr auf Deine Solidarität. Für weitere Umschuldungen benötigen wir weiterhin Direktkredite. "
                      "Empfehle uns Deinen Freund*innen und Verwandten.";
}

void letterTemplate::init_JA_auszahlend()
{LOG_ENTRY_and_EXIT;
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

    html[about] = "Kontoauszug Direktkredit(e) <b>Jahresabschluß {{abrechnungsjahr}} </b>";
    html[mainText1] = "die Mitglieder des Wohnprojektes Esperanza wünschen ein schönes neues Jahr und bedanken sich herzlich für Deine Unterstützung.<p>"
                      "Dies ist der Kontoauszug Deiner Direktkredite für das Jahr 2019 bei Esperanza Franklin GmbH. "
                      "Vereinbarungsgemäß werden die Zinsen Deines Direktkredits in den nächsten Tagen ausgezahlt. Auf Wunsch erstellen wir eine gesonderte Zinsbescheinigung für die Steuererklärung.";
    html[tableHeaderKennung] = "<b> {{tbh.kennung}} <b>";
    html[tableHeaderOldValue] = "<b> {{tbh.old}} </b>";
    html[tableHeaderInterest] = "<b> {{tbh.Zins}} {{abrechnungsjahr}} </b>";
    html[tableHeaderNewValue] = "NOT used";
    html[mainText2] = "Wenn Du Fragen zu dieser Abrechnung hast, zögere bitte nicht, Dich bei uns per Post oder E-Mail zu melden.<p>"
                      "Wir hoffen auch in diesem Jahr auf Deine Solidarität. Für weitere Umschuldungen benötigen wir weiterhin Direktkredite. "
                      "Empfehle uns Deinen Freund*innen und Verwandten.";
}

void letterTemplate::init_Kontoabschluss()
{LOG_ENTRY_and_EXIT;
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

    html[about] = "Beendigung Deines Direktkredites <b> {{vertraege.kennung}} </b>";
    html[mainText1] = "wunschgemäß beenden wir Deinen Direktkredit zum {{vertraege.buchungsdatum}}.<p>"
                      "Der Kredit und die angelaufenen Zinsen werden in den kommenden Tagen auf das von Dir angegebene Konto mit der IBAN {{kerditoren.iban}} ausbezahlt. "
                      "Auf Wunsch erstellen wir eine gesonderte Zinsbescheinigung für die Steuererklärung.";
    html[tableHeaderKennung]  = "<b> {{tbh.kennung}} </b>";
    html[tableHeaderOldValue] = "<b> {{tbh.old}} </b>";
    html[tableHeaderInterest] = "<b> {{thb.zins}} </b>";
    html[tableHeaderNewValue] = "<b> {{thb.new}} </b>";
    html[mainText2] = "Wenn Du Fragen zu dieser Abrechnung hast, zögere bitte nicht, Dich bei uns per Post oder E-Mail zu melden.<p>"
                      "Wir hoffen auch weiterhin auf Deine Solidarität und dass wir Dich bald wieder zu unseren Unterstüzern zählen können. "
                      "Denn für weitere Umschuldungen benötigen wir weiterhin Direktkredite.  Empfehle uns auch Deinen Freund*innen und Verwandten.";
}

void letterTemplate::init_Geldeingang()
{LOG_ENTRY_and_EXIT;
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

    html[about] = "Bestätigung Deines Direktkredites <b> {{vertraege.kennung}} </b>";
    html[mainText1] = "Hiermit bestätigen wir den Geldeingang von {{vertraege.betrag}} zum {{vertraege.buchungsdatum}}. Dein Vertrag wird unter der Kennung {{vertraege.kennung}} geführt.<p>"
                      "Herzlichen Dank für Deine Unterstützung. Wir werden Dich ab jetzt jährlich über die Zinsentwicklung Deines Kredits informieren.";
    html[tableHeaderKennung] = "NOT used";
    html[tableHeaderOldValue] = "NOT used";
    html[tableHeaderInterest] = "NOT used";
    html[tableHeaderNewValue] = "NOT used";
    html[mainText2] = "Wenn Du Fragen zu Deinem Kredit hast, zögere bitte nicht, Dich bei uns per Post oder E-Mail zu melden.<p>"
                      "Wir hoffen auch weiterhin auf Deine Solidarität. Denn für weitere Umschuldungen benötigen wir auch weiterhin Direktkredite. "
                      "Empfehle uns auch Deinen Freund*innen und Verwandten.";
}

void letterTemplate::init_Kuendigung()
{LOG_ENTRY_and_EXIT;
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

    html[about] = "Bestätigung der Kündigung Deines Direktkredites <b> {{vertraege.kennung}} </b>";
    html[mainText1] = "Hiermit bestätigen wir den Eingang der Kündigung des Vertrags {{vertraege.betrag}} zum {{kuendigungsdatum}}.<p>"
                      "Das Vertragsende ergibt sich aus der Kündigungsfrist von {{vertraege.kfrist}} Monaten zum {{vertraege.laufzeitende}}";
    html[tableHeaderKennung] = "NOT used";
    html[tableHeaderOldValue] = "NOT used";
    html[tableHeaderInterest] = "NOT used";
    html[tableHeaderNewValue] = "NOT used";
    html[mainText2] = "Wenn Du Fragen zu Deinem Kredit hast, zögere bitte nicht, Dich bei uns per Post oder E-Mail zu melden.<p>"
                      "Wir hoffen auch weiterhin auf Deine Solidarität. Denn für weitere Umschuldungen benötigen wir auch weiterhin Direktkredite. "
                      "Empfehle uns auch Deinen Freund*innen und Verwandten.";
}

void letterTemplate::init_defaults()
{LOG_ENTRY_and_EXIT;
    html[sections::dateFormat] = "Mannheim, den {{datum}}";
    html[projectAddress] = "{{gmbh.address1}}<p>{{gmbh.address2}}<br>{{gmbh.strasse}}<br><b>{{gmbh.plz}}</b> {{gmbh.stadt}}<br><small>{{gmbh.email}}</small>";
    html[projectUrl] = "<small>{{gmbh.url}}</small>";
    html[Address] ="{{kreditoren.vorname}} {{kreditoren.nachname}} <p> {{kreditoren.strasse}} <br><b> {{kreditoren.plz}} </b> {{kreditoren.stadt}} <br><small> {{kreditoren.email}} </small>";
    html[salutation] = "Liebe(r) {{kreditoren.vorname}}, ";
    html[greeting] = "Mit freundlichen Grüßen";
    html[signee] =  "{{gmbh.dkKontakt}}<br><small>Direktkreditverwaltung</small>";

    switch(tid)
    {
    case Geldeingang:
        init_Geldeingang();
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
    case Kuendigung:
        init_Kuendigung();
        break;
    default:
        qCritical() << "invalid template id?!";
        break;
    }
}

letterTemplate::letterTemplate(letterTemplate::templateId id)
{LOG_ENTRY_and_EXIT;
    tid=id;
    initPrinter();
    if( !loadTemplate(id))
    {
        init_defaults();
        saveTemplate();
    }
}

void letterTemplate::setPlaceholder(QString var, QString val)
{LOG_ENTRY_and_EXIT;
    if( !placeholders.contains(var))
        qWarning() << "Unknown placeholder " << var << " set to value " << val;
    placeholders.insert (var, val);
}

QString letterTemplate::getNameFromId(templateId id)
{LOG_ENTRY_and_EXIT;
    switch(id)
    {
    case Geldeingang:
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
    case Kuendigung:
        return "Kuendigung";
        break;
    default:
        Q_ASSERT("false lettertemplate id");
        return QString();
        break;
    }
}

letterTemplate::templateId letterTemplate::getIdFromName(QString n)
{LOG_ENTRY_and_EXIT;
    for( int i = 0; i < letterTemplate::templateId::maxTemplateId; i++)
    {
        letterTemplate::templateId id=(letterTemplate::templateId)i;
        if( getNameFromId(id) == n)
            return id;
    }
    return letterTemplate::templateId::maxTemplateId;
}

bool letterTemplate::saveTemplate(const QString& con) const
{LOG_ENTRY_and_EXIT;
    QSqlDatabase db = QSqlDatabase::database(con);
    if( !ensureTable(dkdbAddtionalTables["Briefvorlagen"], db))
        return false;

    for( int i = 0; i < letterTemplate::sections::maxSection; i++)
    {
        TableDataInserter tdi(  dkdbAddtionalTables["Briefvorlagen"]);
        tdi.setValue(dkdbAddtionalTables["Briefvorlagen"].Fields()[0].name(), QVariant(tid) );
        tdi.setValue(dkdbAddtionalTables["Briefvorlagen"].Fields()[1].name(), QVariant(i));
        tdi.setValue(dkdbAddtionalTables["Briefvorlagen"].Fields()[2].name(), QVariant(html[i]));
        if( -1 == tdi.InsertOrReplaceData(db))
        {
            qDebug() << "failed to write template data: " << i << ": " << html[i];
            return false;
        }
    }
    for( int i = topmost; i < letterTemplate::distances::maxDistance; i++)
    {
        TableDataInserter tdi(  dkdbAddtionalTables["Briefvorlagen"]);
        tdi.setValue( dkdbAddtionalTables["Briefvorlagen"].Fields()[0].name(), QVariant(tid) );
        tdi.setValue( dkdbAddtionalTables["Briefvorlagen"].Fields()[1].name(), QVariant(i));
        tdi.setValue( dkdbAddtionalTables["Briefvorlagen"].Fields()[2].name(), QVariant(QString::number(length[i])));
        if( -1 == tdi.InsertOrReplaceData(db))
        {
            qDebug() << "failed to write template data: " << i << ": " << length[i];
            return false;
        }
    }
    return true;
}
bool letterTemplate::loadTemplate(letterTemplate::templateId id, const QString& con)
{LOG_ENTRY_and_EXIT;
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
        int prop = query.value(dkdbAddtionalTables["Briefvorlagen"].Fields()[1].name()).toInt();
        QString sValue = query.value(dkdbAddtionalTables["Briefvorlagen"].Fields()[2].name()).toString();
        if( prop >999)
            length[prop] = sValue.toInt();
        else
            html[prop] = sValue;
    }
    return true;
}

bool letterTemplate::operator ==(const letterTemplate &b) const
{LOG_ENTRY_and_EXIT;
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

bool letterTemplate::applyPlaceholders()
{LOG_ENTRY_and_EXIT;
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

bool letterTemplate::createDocument(QTextDocument& doc)
{LOG_ENTRY_and_EXIT;
    QImage img1(":/res/weiss.png");
    QImage img2(":/res/logo.png");
    static QVariant vImg1(img1);
    static QVariant vImg2(img2);
    doc.addResource(QTextDocument::ImageResource,QUrl("weiss.png"),QVariant(vImg1));
    doc.addResource(QTextDocument::ImageResource,QUrl("logo.png"),QVariant(vImg2));

    doc.setPageSize(printer->paperSize(QPrinter::Point));
    doc.setDocumentMargin(20 *mmInPt);

    //some helper
    QTextImageFormat spacer;
    spacer.setName("weiss.png");
    spacer.setHeight(5. *mmInPt); // set the hight of this part of the page
    spacer.setWidth (.01 *mmInPt); // invidible

    enum _side {left =0, right =1};

    // general settinog
    QTextOption to(Qt::AlignJustify); to.setWrapMode(QTextOption::WordWrap);
    doc.setDefaultTextOption(to);
    // Font family
    QFont font(fontFamily, 11*fontOutputFactor);
    font.setKerning(false);
    doc.setDefaultFont(font);

    // the documents content starts HERE
    QTextCursor c = doc.rootFrame()->firstCursorPosition();
    // page structure in table:
    QTextTableFormat tableFormat;
    tableFormat.setBorder( 0.);
    tableFormat.setCellPadding(0.);
    tableFormat.setMargin(0.);
    // column width 2x 50%, logo on the left
    QTextLength cw (QTextLength::PercentageLength, 50);
    QVector<QTextLength> cwc{cw, cw};
    tableFormat.setColumnWidthConstraints(cwc);
    // build the rows
    QTextTable* tt = c.insertTable(16, 2, tableFormat);
    // start with 1 line of nothing
    spacer.setHeight(length[topmost] *mmInPt);
    c.insertImage(spacer, QTextFrameFormat::Position::FloatRight);
    c.movePosition(c.NextRow);

    // space + project address
    spacer.setHeight(length[projectAddressHeight] *mmInPt);
    c.insertImage(spacer, QTextFrameFormat::Position::FloatRight);
    spacer.setHeight(length[overProjectAddress]* mmInPt);
    c.insertImage(spacer, QTextFrameFormat::Position::InFlow);
    c.insertBlock();
    c.insertHtml(html[projectAddress]);

    // top right: LOGO
    c.movePosition(c.NextCell);
    QTextBlockFormat bfUrl = c.blockFormat();
    bfUrl.setAlignment(Qt::AlignRight);
    c.setBlockFormat(bfUrl);
    QTextImageFormat tfLogo;
    tfLogo.setWidth(length[logoWidth] *mmInPt);
    tfLogo.setName("logo.png");
    c.insertImage(tfLogo, QTextFrameFormat::Position::InFlow);
    c.insertBlock();
    c.insertHtml(html[projectUrl]);

    c.movePosition(c.NextRow);c.movePosition(c.NextCell); // Row index 2: date
    QTextBlockFormat bfDate = c.blockFormat();
    bfDate.setAlignment(Qt::AlignRight|Qt::AlignBottom);
    c.setBlockFormat(bfDate);
    c.insertText(html[dateFormat]);

    c.movePosition(c.NextRow);
    c.insertHtml(html[Address]);

    c.movePosition(c.NextRow);
    spacer.setHeight( length[overAbout] *mmInPt);
    c.insertImage(spacer, QTextFrameFormat::FloatRight);

    c.movePosition(c.NextRow);
    tt->mergeCells(tt->cellAt(c).row(), tt->cellAt(c).column(), 1, 2);
    c.insertHtml(html[about]);

    c.movePosition(c.NextRow);
    // Space to the salutation
    spacer.setHeight(length[overSalutation] *mmInPt);
    c.insertImage(spacer, QTextFrameFormat::FloatRight);

    c.movePosition(c.NextRow);
    tt->mergeCells(tt->cellAt(c).row(), tt->cellAt(c).column(), 1, 2);
    // Salutiation
    c.insertHtml(html[salutation]);

    c.movePosition(c.NextRow);
    // Space to main text
    spacer.setHeight(length[overText] *mmInPt);
    c.insertImage(spacer, QTextFrameFormat::FloatRight);

    c.movePosition(c.NextRow);
    // Main text (before table)
    tt->mergeCells(tt->cellAt(c).row(), tt->cellAt(c).column(), 1, 2);
    c.insertHtml(html[mainText1]);

    c.movePosition(c.NextRow);
    // Row index 8: table
    tt->mergeCells(tt->cellAt(c).row(), tt->cellAt(c).column(), 1, 2);

    QTextBlockFormat bfAlignCellLeft = c.blockFormat();
    bfAlignCellLeft.setAlignment(Qt::AlignLeft);
    QTextBlockFormat bfAlignCellCenter = c.blockFormat();
    bfAlignCellCenter.setAlignment(Qt::AlignCenter);
    c.setBlockFormat(bfAlignCellCenter);

//    QTextTableFormat tfDks;
//    tfDks.setBorder(0.);
//    tfDks.setCellPadding(1 *mmInPt);
//    tfDks.setMargin(tableMargin *mmInPt);
//    tfDks.setLeftMargin(20 *mmInPt);
//    c.insertTable(3, 4, tfDks);
//    QTextTable* ttDks  = c.currentTable();
//    c = ttDks->cellAt(0,0).firstCursorPosition(); c.setBlockFormat(bfAlignCellLeft);
//    c.insertHtml(html[tableHeaderKennung]);
//    c.movePosition(c.NextCell); c.setBlockFormat(bfAlignCellCenter);
//    c.insertHtml(html[tableHeaderOldValue]);
//    c.movePosition(c.NextCell); c.setBlockFormat(bfAlignCellCenter);
//    c.insertHtml(html[tableHeaderInterest]);
//    c.movePosition(c.NextCell); c.setBlockFormat(bfAlignCellCenter);
//    c.insertHtml(html[tableHeaderNewValue]);
//    c.movePosition(c.NextRow); c.setBlockFormat(bfAlignCellLeft);
//    c.insertHtml("DK-ESP-2018-024");
//    c.movePosition(c.NextCell); c.setBlockFormat(bfAlignCellCenter);
//    c.insertHtml("12.300 Euro");
//    c.movePosition(c.NextCell); c.setBlockFormat(bfAlignCellCenter);
//    c.insertHtml("123 Euro");
//    c.movePosition(c.NextCell); c.setBlockFormat(bfAlignCellCenter);
//    c.insertHtml("12.423 Euro");
//    c.movePosition(c.NextRow); c.setBlockFormat(bfAlignCellLeft);
//    c.insertHtml("DK-ESP-2019-054");
//    c.movePosition(c.NextCell); c.setBlockFormat(bfAlignCellCenter);
//    c.insertHtml("10.100 Euro");
//    c.movePosition(c.NextCell); c.setBlockFormat(bfAlignCellCenter);
//    c.insertHtml("101 Euro");
//    c.movePosition(c.NextCell); c.setBlockFormat(bfAlignCellCenter);
//    c.insertHtml("10.201 Euro");
//    // step out of inner table
//    c.movePosition(c.NextBlock);

    c.movePosition(c.NextRow);
    // Row index 9: Main text after table
    tt->mergeCells(tt->cellAt(c).row(), tt->cellAt(c).column(), 1, 2);
    c.insertHtml(html[mainText2] );

    c.movePosition(c.NextRow);
    // Row index 6: Space to greeting
    spacer.setHeight(length[overGreeting] *mmInPt);
    c.insertImage(spacer, QTextFrameFormat::FloatRight);

    c.movePosition(c.NextRow);
    // Row index 10: Grußformel
    c.insertHtml(html[greeting]);

    c.movePosition(c.NextRow);
    // Row index 6: Space to signee
    spacer.setHeight(length[overSignee] *mmInPt);
    c.insertImage(spacer, QTextFrameFormat::FloatRight);

    c.movePosition(c.NextRow);
    c.insertHtml(html[signee]);
    return true;
}

bool letterTemplate::createPdf(QString file, const QTextDocument& doc)
{LOG_ENTRY_and_EXIT;
    QFile::remove(file);
    if( QFile::exists(file))
    {
        qDebug() << "pdf file creation failed";
        return false;
    }

    printer->setOutputFileName(file);
    doc.print(printer);
    return true;
}

bool letterTemplate::print(const QString& fileId)
{LOG_ENTRY_and_EXIT;
    QSettings config;
    QString outputfile = config.value("outdir").toString();
    outputfile +=+"/";
    outputfile += QDate::currentDate().toString("yyyy-MM-dd_") + getNameFromId(tid) + "_" +fileId.trimmed() +".pdf";
    qDebug() << "printing to " << outputfile;
    applyPlaceholders();
    QTextDocument doc;
    createDocument(doc);
    createPdf(outputfile, doc);
    return true;
}
