#ifndef KREDITOR_H
#define KREDITOR_H

#include <QString>
#include <QPair>
#include <QList>

#include "helper.h"
#include "tabledatainserter.h"
#include "dkdbhelper.h"


struct creditor
{
    static const dbtable& getTableDef();
    // constructors
    creditor() : ti(getTableDef()){}
    creditor (int i) : ti(getTableDef()){ fromDb(i);}
    // comparison
    bool operator==(const creditor& c) const;
    // setter
    QString firstname() const   { return getValue(qsl("Vorname")).toString();}
    void setFirstname(const QString& v){ ti.setValue(qsl("Vorname"),  v); }
    QString lastname() const    { return getValue(qsl("Nachname")).toString();}
    void setLastname(const QString& n) { ti.setValue(qsl("Nachname"), n); }
    QString street() const      { return getValue(qsl("Strasse")).toString();}
    void setStreet(const QString& s)   { ti.setValue(qsl("Strasse"),  s); }
    QString postalCode() const  { return getValue(qsl("Plz")).toString();}
    void setPostalCode(const QString& p){ ti.setValue(qsl("Plz"),     p); }
    QString city() const        { return getValue(qsl("Stadt")).toString();}
    void setCity(const QString& s)     { ti.setValue(qsl("Stadt"),  s); }
    QString country() const     { return getValue(qsl("Land")).toString();}
    void setCountry(const QString& s)  { ti.setValue(qsl("Land"), s); }
    QString email() const       { return getValue(qsl("Email")).toString();}
    void setEmail(const QString& e)    { ti.setValue(qsl("Email"), e); }
    QString comment() const     { return getValue(qsl("Anmerkung")).toString();}
    void setComment(const QString& a)  { ti.setValue(qsl("Anmerkung"), a);}
    QString iban() const        {return getValue(qsl("IBAN")).toString();}
    void setIban(const QString& i)     { ti.setValue(qsl("IBAN"), i);}
    QString bic() const        {return getValue(qsl("BIC")).toString();}
    void setBic(const QString& b)      { ti.setValue(qsl("BIC"), b);}
    qlonglong id() const        {return getValue(qsl("id")).toLongLong();}
    void setId(const qlonglong i)     { ti.setValue(qsl("id"), i);}
    // interface
    bool fromDb(const int id);
    QVariant getValue(const QString& f) const { return ti.getValue(f);}
    bool isValid( QString& errortext) const;
    bool isValid() const;
    int save();
    int update() const;
    bool hasActiveContracts(){return hasActiveContracts(id());};
    static bool hasActiveContracts(const qlonglong i);
    bool remove();
    static bool remove(const int index);
private:
    // data
    TableDataInserter ti;
    // helper
};

void KreditorenListeMitId(QList<QPair<int,QString>> &entries);


// sample data for testing
extern QList<QString> Vornamen;// {"Holger", "Volker", "Peter", "Hans", ...
extern QList<QString> Nachnamen;// {"Maier", "Müller", "Schmit", "Kramp", ...
extern QList<QString> Strassen;// {"Hauptstrasse", "Nebenstrasse", "Bahnhofstrasse"...
extern QList<QString> emailprovider;// {"gmail.com", "googlemail.com", "mailbox.org",...
extern QList<QString> ibans;//  {"BG80BNBG96611020345678", "DE38531742365852502530", ...
extern QList <QPair<QString, QString>> Cities;// {{"68305", "Mannheim"}, {"69123", ...
// for testing
creditor saveRandomCreditor();
void saveRandomCreditors( const int i);
void saveRandomCreditors_q( const int i);

#endif // KREDITOR_H
