#ifndef LETTER_H
#define LETTER_H

#include <QMap>
#include <QDate>
#include <QTextDocument>
#include <QPrinter>

#include "helper.h"
#include "helpersql.h"
#include "appconfig.h"

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
    {   {qsl("datum"),QString()}, {qsl("abrechnungsjahr"), qsl("2020")}, {qsl("kuendigungsdatum"), QString()},
        {GMBH_ADDRESS1,qsl("Esperanza Franklin GmbH")}, {GMBH_ADDRESS2,QString()}, {GMBH_STREET,qsl("Turley-Platz 9")}, {GMBH_PLZ, qsl("68167")}, {GMBH_CITY, qsl("Mannheim")}, {GMBH_EMAIL, qsl("info@esperanza-mannheim.de")}, {GMBH_URL, qsl("www.esperanza-mannheim.de")},{qsl("gmbh.dkKontakt"), qsl("Jutta Sichau")},
        {qsl("kreditoren.vorname"), QString()}, {qsl("kreditoren.nachname"), QString()}, {qsl("kreditoren.strasse"), QString()}, {qsl("kreditoren.plz"), QString()}, {qsl("kreditoren.stadt"), QString()}, {qsl("kreditoren.email"), QString()}, {qsl("kreditoren.iban"), QString()},
        {qsl("vertraege.kennung"), QString()}, {qsl("vertraege.betrag"), QString()}, {qsl("vertraege.buchungsdatum"), QString()}, {qsl("vertraege.kfrist"), QString()}, {qsl("vertraege.laufzeitende"), QString()},
        {qsl("tbh.kennung"), qsl("Vertragskennung")}, {qsl("tbh.old"), qsl("Vorjahreswert")}, {qsl("tbh.zins"), qsl("Zinssatz")}, {qsl("tbh.new"), qsl("Neuer Wert")}
    };

public:
    //explicit letterTemplate(){ initPrinter();};
    letterTemplate(templateId type);
    ~letterTemplate(){ }
    bool saveTemplate() const;
    bool loadTemplate(letterTemplate::templateId id);

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
