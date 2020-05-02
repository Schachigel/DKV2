#ifndef LETTER_H
#define LETTER_H

#include <QMap>
#include <QDate>
#include <QSqlDatabase>
#include <QTextDocument>
#include <QPrinter>

#include "sqlhelper.h"

const double mmInPt {2.83465};

class letterTemplate
{
public:
    enum sections{ dateFormat, projectAddress, projectUrl, Address, about, salutation,
                   mainText1, tableHeaderKennung, tableHeaderOldValue, tableHeaderInterest,
                   tableHeaderNewValue, mainText2, greeting, signee, maxSection
    };
    enum distances{ topmost= maxSection+1000, overProjectAddress, projectAddressHeight, logoWidth, overAbout,
                    overSalutation, overText, tableMargin, overGreeting, overSignee, maxDistance
    };
    enum templateId{ Geldeingang, JA_thesa, JA_auszahlend, Kontoabschluss, Kuendigung, maxTemplateId
    };
    QMap<QString, QString> placeholders
    {   {"datum",""}, {"abrechnungsjahr", "2020"}, {"kuendigungsdatum", ""},
        {"gmbh.address1","Esperanza Franklin GmbH"}, {"gmbh.address2",""}, {"gmbh.strasse","Turley-Platz 9"}, {"gmbh.plz", "68167"}, {"gmbh.stadt", "Mannheim"}, {"gmbh.email", "info@esperanza-mannheim.de"}, {"gmbh.url", "www.esperanza-mannheim.de"},{"gmbh.dkKontakt", "Jutta Sichau"},
        {"kreditoren.vorname", ""}, {"kreditoren.nachname", ""}, {"kreditoren.strasse", ""}, {"kreditoren.plz", ""}, {"kreditoren.stadt", ""}, {"kreditoren.email", ""}, {"kreditoren.iban", ""},
        {"vertraege.kennung", ""}, {"vertraege.betrag", ""}, {"vertraege.buchungsdatum", ""}, {"vertraege.kfrist", ""}, {"vertraege.laufzeitende", ""},
        {"tbh.kennung", "Vertragskennung"}, {"tbh.old", "Vorjahreswert"}, {"tbh.zins", "Zinssatz"}, {"tbh.new", "Neuer Wert"}
    };

public:
    //explicit letterTemplate(){ initPrinter();};
    letterTemplate(templateId type, QSqlDatabase db =defaultDb());
    ~letterTemplate(){ }
    bool saveTemplate(QSqlDatabase db = defaultDb()) const;
    bool loadTemplate(letterTemplate::templateId id, QSqlDatabase db= defaultDb());

    void init_Geldeingang();
    void init_JA_thesa();
    void init_JA_auszahlend();
    void init_Kontoabschluss();
    void init_Kuendigung();

    void setPlaceholder(QString var, QString val);
    bool applyPlaceholders();
    bool createDocument(QTextDocument& doc);
    QString fileNameFromId(const QString& contractId);
    bool print( const QString& name);

    static QString getNameFromId(templateId id) ;
    static templateId getIdFromName(QString n);
    bool operator ==(const letterTemplate &b) const;
    void init_defaults();
    QMap<int, QString>& Html(){ return html;};

private:
    static QPrinter* printer;
    void initPrinter();
    bool createPdf(QString file, const QTextDocument& doc);
    templateId tid;
    QMap<int, QString> html;
    QMap<int, double>  length;
    QString fontFamily{"Verdana"};
    double fontOutputFactor{0.65};

};

#endif // LETTER_H
