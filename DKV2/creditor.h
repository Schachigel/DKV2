#ifndef KREDITOR_H
#define KREDITOR_H

#include <QString>
#include <QPair>
#include <QList>

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
    QString firstname() const   { return getValue("Vorname").toString();}
    void setFirstname(QString v){ ti.setValue("Vorname",  v); }
    QString lastname() const    { return getValue("Nachname").toString();}
    void setLastname(QString n) { ti.setValue("Nachname", n); }
    QString street() const      { return getValue("Strasse").toString();}
    void setStreet(QString s)   { ti.setValue("Strasse",  s); }
    QString postalCode() const  { return getValue("Plz").toString();}
    void setPostalCode(QString p){ ti.setValue("Plz",      p); }
    QString city() const        { return getValue("Stadt").toString();}
    void setCity(QString s)     { ti.setValue("Stadt",  s); }
    QString email() const       { return getValue("Email").toString();}
    void setEmail(QString e)    { ti.setValue("Email", e); }
    QString comment() const     { return getValue("Vorname").toString();}
    void setComment(QString a)  { ti.setValue("Anmerkung", a);}
    QString iban() const        {return getValue("IBAN").toString();}
    void setIban(QString i)     { ti.setValue("IBAN", i);}
    QString bic() const        {return getValue("BIC").toString();}
    void setBic(QString b)      { ti.setValue("BIC", b);}
    qlonglong id() const        {return getValue("id").toLongLong();}
    void setId(qlonglong i)     { ti.setValue("id", i);}
    // interface
    bool fromDb(int id);
    QVariant getValue(const QString& f) const { return ti.getValue(f);}
    bool isValid( QString& errortext) const;
    bool isValid() const;
    int save();
    int update() const;
    void KreditorenListeMitId(QList<QPair<int,QString>> &entries) const;
    bool hasActiveContracts(){return hasActiveContracts(id());};
    static bool hasActiveContracts(qlonglong i);
    bool remove();
    static bool remove(int index);
private:
    // data
    TableDataInserter ti;
    // helper
};

// sample data for testing
extern QList<QString> Vornamen;// {"Holger", "Volker", "Peter", "Hans", "Susi", "Roland", "Claudia", "Emil", "Evelyn", "Ötzgür", "Thomas", "Elke", "Berta", "Malte", "Jori", "Paul", "Jonas", "Finn", "Leon", "Luca", "Emma", "Mia", "Lena", "Anna", "Anne", "Martha", "Ruth", "Rosemie", "Rosemarie", "Verena", "Ursula", "Erika", "Adrian", "Avan", "Anton", "Benno", "Karl", "Merlin", "Noah", "Oliver","Olaf", "Pepe", "Zeno", "Apollo", "Edward", "Ronaldo", "Siegbert", "Thomas", "Michael"};
extern QList<QString> Nachnamen;// {"Maier", "Müller", "Schmit", "Kramp", "Adams", "Häcker", "Maresch", "Beutl", "Chauchev", "Chen", "Kirk", "Ohura", "Gorbatschov", "Merkel", "Karrenbauer", "Tritin", "Schmidt", "Rao", "Lassen", "Hurgedü", "vom Dach", "Langstrumpf", "Lederstrumpf", "Potter", "Poppins", "Wisley", "Li", "Wang", "Ran", "vom Dach", "Eckrich", "Staar", "Funke", "Engelein", "Kruffel", "Calzone"};
extern QList<QString> Strassen;// {"Hauptstrasse", "Nebenstrasse", "Bahnhofstrasse", "Kirchstraße", "Dorfstrasse", "Süterlinweg", "Sorbenstrasse", "Kleines Gässchen", "Industriestrasse", "Sesamstrasse", "Lindenstrasse", "Theaterstrasse", "Museumsstrasse", "Opernplatz", "Schillerstrasse", "Lessingstrasse", "Rathausplatz", "Parkstrasse", "Turmstrasse", "Neuer Weg", "Neuer Anfang", "Main Street", "Center Court" };
extern QList<QString> emailprovider;// {"gmail.com", "googlemail.com", "mailbox.org", "t-online.de", "mail.de", "mail.com", "online.de", "yahoo.de", "yahoo.com", "telekom.de", "proivder.co.uk", "AOL.de", "outlook.com", "microsoft.com", "sap.com", "sap-ag.de", "abb.de", "skype.de", "provider.de", ""};
extern QList<QString> ibans;//  {"BG80BNBG96611020345678", "DE38531742365852502530", "DE63364408232964251731", "DE38737364268384258531", "DE69037950954001627624", "DE63377045386819730665", "DE18851420444163951769", "DE77921850720298609321", "DE70402696485599313572", "DE70455395581860402838", "DE94045704387963352767", "DE30724236236062816411", "DE62772043290447861437", "DE33387723124963875990", "DE15867719874951165967", "DE96720348741083219766", "DE23152931057149592044", "DE13220161295670898833", "DE49737651031822324605", "DE38017168378078601588", "DE07717138875827514267"};
extern QList <QPair<QString, QString>> Cities;// {{"68305", "Mannheim"}, {"69123", "Heidelberg"}, {"69123", "Karlsruhe"}, {"90345", "Hamburg"}, {"90334", "Rottenburg"}, {"23345", "Reinfeld"}, {"83475", "Dresden"}, {"35725", "Weissnich"}, {"23245", "Drieben"}};
// for testing
creditor saveRandomCreditor();
void saveRandomCreditors( int i);

#endif // KREDITOR_H
