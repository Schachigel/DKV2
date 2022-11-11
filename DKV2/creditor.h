#ifndef KREDITOR_H
#define KREDITOR_H

#include "helper.h"
#include "tabledatainserter.h"

struct creditor
{
    static const QString fnId;
    static const QString tablename;
    static const QString fnVorname;
    static const QString fnNachname;
    static const QString fnStrasse;
    static const QString fnPlz;
    static const QString fnStadt;
    static const QString fnLand;
    static const QString fnTel;
    static const QString fnEmail;
    static const QString fnAnmerkung;
    static const QString fnKontakt;
    static const QString fnBuchungskonto;
    static const QString fnIBAN;
    static const QString fnBIC;
    static const QString fnZeitstepel;

    static const dbtable& getTableDef();
    // constructors
    creditor() : ti(getTableDef()) {}
    creditor (qlonglong i) : ti(getTableDef()) { fromDb(i);}
    // comparison
    bool operator==(const creditor& c) const;
    // setter
    QString firstname() const   { return getValue(fnVorname).toString();}
    void setFirstname(const QString& v){ ti.setValue(fnVorname,  v); }
    QString lastname() const    { return getValue(fnNachname).toString();}
    void setLastname(const QString& n) { ti.setValue(fnNachname, n); }
    QString street() const      { return getValue(fnStrasse).toString();}
    void setStreet(const QString& s)   { ti.setValue(fnStrasse,  s); }
    QString postalCode() const  { return getValue(fnPlz).toString();}
    void setPostalCode(const QString& p){ ti.setValue(fnPlz,     p); }
    QString city() const        { return getValue(fnStadt).toString();}
    void setCity(const QString& s)     { ti.setValue(fnStadt,  s); }
    QString country() const     { return getValue(fnLand).toString();}
    void setCountry(const QString& s)  { ti.setValue(fnLand, s); }
    QString email() const       { return getValue(fnEmail).toString();}
    void setEmail(const QString& e)    { ti.setValue(fnEmail, e); }
    QString tel() const       { return getValue(fnTel).toString();}
    void setTel(const QString& e)    { ti.setValue(fnTel, e); }
    QString comment() const     { return getValue(fnAnmerkung).toString();}
    void setComment(const QString& a)  { ti.setValue(fnAnmerkung, a);}
    QString contact() const     { return getValue(fnKontakt).toString();}
    void setContact(const QString& a)  { ti.setValue(fnKontakt, a);}
    QString account() const     { return getValue(fnBuchungskonto).toString();}
    void setAccount(const QString& a)  { ti.setValue(fnBuchungskonto, a);}
    QString iban() const        {return getValue(qsl("IBAN")).toString();}
    void setIban(const QString& i)     { ti.setValue(qsl("IBAN"), i);}
    QString bic() const        {return getValue(qsl("BIC")).toString();}
    void setBic(const QString& b)      { ti.setValue(qsl("BIC"), b);}
    qlonglong id() const        {return getValue(qsl("id")).toLongLong();}
    void setId(const qlonglong i)     { ti.setValue(qsl("id"), i);}
    // interface
    bool fromDb(const qlonglong id);
    QVariant getValue(const QString& f) const { return ti.getValue(f);}
    QVariant getVariant();
    bool isValid(QString &errortext) const;
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

void getAllCreditorInfoSorted(QList<QPair<qlonglong, QString> > &entries);
void creditorsWithAnnualSettlement(QList<qlonglong> &entries, int bookingYear =-1);


// for testing
creditor saveRandomCreditor();
void saveRandomCreditors( const int i);

#endif // KREDITOR_H
