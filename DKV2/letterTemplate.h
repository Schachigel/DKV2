#ifndef LETTER_H
#define LETTER_H

#include <QMap>
#include <QDate>
#include <QTextDocument>

#include "dbtable.h"
#include "helper.h"
#include "helpersql.h"
#include "appconfig.h"

template <class IndexType, class ValueType>
// template class to make enum class types "loopable"
// use if enum counts from 0, w/o gaps.
// define maxId as last+1 enum value
class EnumArray {
public:
    // array access
    ValueType& operator[](IndexType i) { return array_[static_cast<int>(i)]; }
    const ValueType& operator[](IndexType i) const { return array_[static_cast<int>(i)]; }
    // iteration (range based for loops)
    int size() const { return size_; }
    auto begin() { return &array_[0]; }
    auto end() { return &array_[static_cast<int>(IndexType::maxId) -1]; }

private:
    ValueType array_[static_cast<int>(IndexType::maxId)];
    int size_ = static_cast<int>(IndexType::maxId);
};

const double mmInPt {2.83465};

class letterTemplate
{
public: // types and data
    enum class elementType{ Adresse =0, Logo, Datum, Betreff, Anrede, Text1, Abrechnungstabelle, Text2, Gruss, Fuss, maxId};
    static QMap<elementType, QString> all_elementTypes;
    int operator() (elementType s) {
        Q_ASSERT(static_cast<int>(s) >= 0);
        Q_ASSERT(static_cast<int>(s) < static_cast<int>(elementType::maxId));
        return static_cast<int>(s);
    }

    enum class templId{ generic =0, contractConclusion, activation, annualInterestReinvest,
        annualInterestPayout, deposit, payout, termination, accountClosing, maxId };
    static QMap<templId, QString> all_templates;
    int operator() (templId id) {
        Q_ASSERT(static_cast<int>(id) >= 0);
        Q_ASSERT(static_cast<int>(id) < static_cast<int>(templId::maxId));
        return static_cast<int>(id); }

    QMap<QString, QString> placeholders
    {   {qsl("datum"),QString()}, {qsl("abrechnungsjahr"), qsl("2020")}, {qsl("kuendigungsdatum"), QString()},
        {GMBH_ADDRESS1,qsl("Esperanza Franklin GmbH")}, {GMBH_ADDRESS2,QString()}, {GMBH_STREET,qsl("Turley-Platz 9")}, {GMBH_PLZ, qsl("68167")}, {GMBH_CITY, qsl("Mannheim")}, {GMBH_EMAIL, qsl("info@esperanza-mannheim.de")}, {GMBH_URL, qsl("www.esperanza-mannheim.de")},{qsl("gmbh.dkKontakt"), qsl("Jutta Sichau")},
        {qsl("kreditoren.vorname"), QString()}, {qsl("kreditoren.nachname"), QString()}, {qsl("kreditoren.strasse"), QString()}, {qsl("kreditoren.plz"), QString()}, {qsl("kreditoren.stadt"), QString()}, {qsl("kreditoren.email"), QString()}, {qsl("kreditoren.iban"), QString()},
        {qsl("vertraege.kennung"), QString()}, {qsl("vertraege.betrag"), QString()}, {qsl("vertraege.buchungsdatum"), QString()}, {qsl("vertraege.kfrist"), QString()}, {qsl("vertraege.laufzeitende"), QString()},
        {qsl("tbh.kennung"), qsl("Vertragskennung")}, {qsl("tbh.old"), qsl("Vorjahreswert")}, {qsl("tbh.zins"), qsl("Zinssatz")}, {qsl("tbh.new"), qsl("Neuer Wert")}
    };
public: // interface
    static dbtable getTableDef_letterTypes();
    static dbtable getTabelDef_elementTypes();
    static dbtable getTableDef_letterElements();
    static bool insert_letterTypes(QSqlDatabase db);
    static bool insert_elementTypes(QSqlDatabase db);
    static bool insert_letterElements(QSqlDatabase db);

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
