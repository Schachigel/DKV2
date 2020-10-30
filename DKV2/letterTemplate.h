#ifndef LETTER_H
#define LETTER_H

#include <QMap>
#include <QDate>
#include <QTextDocument>
#include <QPrinter>

#include "dbtable.h"
#include "helper.h"
#include "helpersql.h"
#include "appconfig.h"

template <class IndexType, class ValueType>
class EnumArray {
public:
    ValueType& operator[](IndexType i) {
        return array_[static_cast<int>(i)];
    }

    const ValueType& operator[](IndexType i) const {
        return array_[static_cast<int>(i)];
    }

    int size() const { return size_; }
    auto begin() { return &array_[0]; }
    auto end() { return &array_[static_cast<int>(IndexType::maxTemplateId) -1]; }

private:
    ValueType array_[static_cast<int>(IndexType::maxTemplateId)];
    int size_ = static_cast<int>(IndexType::maxTemplateId);
};

const double mmInPt {2.83465};

class letterTemplate
{
public: // types and data
    enum class sections{ Adresse, Logo, Datum, Betreff, Anrede, Text1, Abrechnungstabelle, Text2, Gruss, Fuss, maxSection};
    static QMap<sections, QString> all_sections;
    int operator() (sections s) { Q_ASSERT(static_cast<int>(s) > 0; Q_ASSERT(static_cast<int>(s) < static_cast<int>(maxSection)); return static_cast<int>(s); }

    enum class templId{ Vertragsabschluss =1, Geldeingang, JA_thesa, JA_auszahlend, Kontoabschluss, Kuendigung, maxTemplateId };
    static QVector<templId> all_templIds;
    int operator() (templId id) { Q_ASSERT(static_cast<int>(id) > 0); Q_ASSERT(static_cast<int>(id) < static_cast<int>(templId::maxTemplateId)); return static_cast<int>(id); }
    QString templName(templId id);

    QMap<QString, QString> placeholders
    {   {qsl("datum"),QString()}, {qsl("abrechnungsjahr"), qsl("2020")}, {qsl("kuendigungsdatum"), QString()},
        {GMBH_ADDRESS1,qsl("Esperanza Franklin GmbH")}, {GMBH_ADDRESS2,QString()}, {GMBH_STREET,qsl("Turley-Platz 9")}, {GMBH_PLZ, qsl("68167")}, {GMBH_CITY, qsl("Mannheim")}, {GMBH_EMAIL, qsl("info@esperanza-mannheim.de")}, {GMBH_URL, qsl("www.esperanza-mannheim.de")},{qsl("gmbh.dkKontakt"), qsl("Jutta Sichau")},
        {qsl("kreditoren.vorname"), QString()}, {qsl("kreditoren.nachname"), QString()}, {qsl("kreditoren.strasse"), QString()}, {qsl("kreditoren.plz"), QString()}, {qsl("kreditoren.stadt"), QString()}, {qsl("kreditoren.email"), QString()}, {qsl("kreditoren.iban"), QString()},
        {qsl("vertraege.kennung"), QString()}, {qsl("vertraege.betrag"), QString()}, {qsl("vertraege.buchungsdatum"), QString()}, {qsl("vertraege.kfrist"), QString()}, {qsl("vertraege.laufzeitende"), QString()},
        {qsl("tbh.kennung"), qsl("Vertragskennung")}, {qsl("tbh.old"), qsl("Vorjahreswert")}, {qsl("tbh.zins"), qsl("Zinssatz")}, {qsl("tbh.new"), qsl("Neuer Wert")}
    };

public: // interface
    static dbtable getTabelDef_sections();
    static dbtable letterTemplate::getTableDef();
    static bool insert_sections(QSqlDatabase db);
    static bool insert_letters(QSqlDatabase db);
    //explicit letterTemplate(){ initPrinter();};
    letterTemplate(templId type);
    bool saveDefaultTemplate() const;
    bool saveTemplate(qlonglong kreditor) const;
    bool loadDefaultTemplate(templId id);
    bool loadTemplate(templId id, qlonglong kreditor);

    void init_Geldeingang();
    void init_JA_thesa();
    void init_JA_auszahlend();
    void init_Kontoabschluss();
    void init_Kuendigung();

    void setPlaceholder(QString var, QString val);
    bool applyPlaceholders();
    bool createDocument(QTextDocument& doc);
    QString fileNameFromId(const QString& contractId);

//    static templId getIdFromName(QString n);
    bool operator ==(const letterTemplate &b) const;
    void init_defaults();
    QMap<int, QString>& Html(){ return html;}

private:
    templId tid;
    QMap<int, QString> html;
    QMap<int, double>  length;
    QString fontFamily{"Verdana"};
    double fontOutputFactor{0.65};
};

#endif // LETTER_H
