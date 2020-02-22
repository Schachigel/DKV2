#ifndef LETTER_H
#define LETTER_H

#include <QMap>
#include <QDate>
#include <QSqlDatabase>
#include <QTextDocument>
#include <QtPrintSupport/QtPrintSupport>

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
    enum templateId{ geldeingang, JA_thesa, JA_auszahlend, Kontoabschluss, maxTemplateId
    };
    QMap<QString, QString> placeholders
    {   {"datum",""}, {"abrechnungsjahr", "2020"},
        {"gmbh.address1","Esperanza Franklin GmbH"}, {"gmbh.address2",""}, {"gmbh.strasse","Turley-Platz 9"}, {"gmbh.plz", "68167"}, {"gmbh.stadt", "Mannheim"}, {"gmbh.email", "info@esperanza-mannheim.de"}, {"gmbh.url", "www.esperanza-mannheim.de"},{"gmbh.dkKontakt", "Jutta Sichau"},
        {"kreditoren.vorname", ""}, {"kreditoren.nachname", ""}, {"kreditoren.strasse", ""}, {"kreditoren.plz", ""}, {"kreditoren.stadt", ""}, {"kreditoren.email", ""}, {"kreditoren.iban", ""},
        {"vertraege.kennung", ""}, {"vertraege.betrag", ""}, {"vertraege.buchungsdatum", ""},
        {"tbh.kennung", "Vertragskennung"}, {"tbh.old", "Vorjahreswert"}, {"tbh.zins", "Zinssatz"}, {"tbh.new", "Neuer Wert"}
    };

public:
    explicit letterTemplate(){ initPrinter();};
    letterTemplate(templateId type);
    ~letterTemplate(){ if( printer) delete printer; printer=nullptr;}
    bool saveTemplate(const QString& con =QSqlDatabase::defaultConnection) const;
    bool loadTemplate(letterTemplate::templateId id, const QString& con =QSqlDatabase::defaultConnection);

    void init_geldeingang();
    void init_JA_thesa();
    void init_JA_auszahlend();
    void init_Kontoabschluss();

    void setPlaceholder(QString var, QString val);
    bool applyPlaceholders();
    bool createDocument(QTextDocument& doc);
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
