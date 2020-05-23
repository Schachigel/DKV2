#include <QSqlRecord>
#include <QRegularExpression>
#include "finhelper.h"
#include "helper.h"
#include "sqlhelper.h"
#include "creditor.h"

QList<QString> Vornamen {"Holger", "Volker", "Peter", "Hans", "Susi", "Roland", "Claudia", "Emil", "Evelyn", "Ötzgür", "Thomas", "Elke", "Berta", "Malte", "Jori", "Paul", "Jonas", "Finn", "Leon", "Luca", "Emma", "Mia", "Lena", "Anna", "Anne", "Martha", "Ruth", "Rosemie", "Rosemarie", "Verena", "Ursula", "Erika", "Adrian", "Avan", "Anton", "Benno", "Karl", "Merlin", "Noah", "Oliver","Olaf", "Pepe", "Zeno", "Apollo", "Edward", "Ronaldo", "Siegbert", "Thomas", "Michael"};
QList<QString> Nachnamen {"Maier", "Müller", "Schmit", "Kramp", "Adams", "Häcker", "Maresch", "Beutl", "Chauchev", "Chen", "Kirk", "Ohura", "Gorbatschov", "Merkel", "Karrenbauer", "Tritin", "Schmidt", "Rao", "Lassen", "Hurgedü", "vom Dach", "Langstrumpf", "Lederstrumpf", "Potter", "Poppins", "Wisley", "Li", "Wang", "Ran", "vom Dach", "Eckrich", "Staar", "Funke", "Engelein", "Kruffel", "Calzone"};
QList<QString> Strassen {"Hauptstrasse", "Nebenstrasse", "Bahnhofstrasse", "Kirchstraße", "Dorfstrasse", "Süterlinweg", "Sorbenstrasse", "Kleines Gässchen", "Industriestrasse", "Sesamstrasse", "Lindenstrasse", "Theaterstrasse", "Museumsstrasse", "Opernplatz", "Schillerstrasse", "Lessingstrasse", "Rathausplatz", "Parkstrasse", "Turmstrasse", "Neuer Weg", "Neuer Anfang", "Main Street", "Center Court" };
QList<QString> emailprovider {"gmail.com", "googlemail.com", "mailbox.org", "t-online.de", "mail.de", "mail.com", "online.de", "yahoo.de", "yahoo.com", "telekom.de", "proivder.co.uk", "AOL.de", "outlook.com", "microsoft.com", "sap.com", "sap-ag.de", "abb.de", "skype.de", "provider.de"};
QList<QString> ibans {"BG80BNBG96611020345678", "DE38531742365852502530", "DE63364408232964251731", "DE38737364268384258531", "DE69037950954001627624", "DE63377045386819730665", "DE18851420444163951769", "DE77921850720298609321", "DE70402696485599313572", "DE70455395581860402838", "DE94045704387963352767", "DE30724236236062816411", "DE62772043290447861437", "DE33387723124963875990", "DE15867719874951165967", "DE96720348741083219766", "DE23152931057149592044", "DE13220161295670898833", "DE49737651031822324605", "DE38017168378078601588", "DE07717138875827514267"};
QList <QPair<QString, QString>> Cities {{"68305", "Mannheim"}, {"69123", "Heidelberg"}, {"69123", "Karlsruhe"}, {"90345", "Hamburg"}, {"90334", "Rottenburg"}, {"23345", "Reinfeld"}, {"83475", "Dresden"}, {"35725", "Weissnich"}, {"23245", "Drieben"}};

bool creditor::operator==(const creditor& c) const
{
    bool ret = false;
    do{
        if( id()        != c.id()) break;
        if( firstname() != c.firstname()) break;
        if( lastname()  != c.lastname()) break;
        if( street()    != c.street()) break;
        if( postalCode()!= c.postalCode()) break;
        if( city()      != c.city()) break;
        if( comment()   != c.comment()) break;
        if( email()     != c.email()) break;
        if( iban()      != c.iban()) break;
        if( bic()       != c.bic()) break;
        ret = true;
    } while(false);
    return ret;
}

bool creditor::fromDb( int i)
{   LOG_CALL;

    QSqlRecord rec = executeSingleRecordSql(dkdbstructur["Kreditoren"].Fields(), "id="+QString::number(i));
    if( rec.isEmpty()) return false;

    for(int i=0; i<rec.count(); i++)
    {
        qDebug() << "reading Kreditor from db; Field:" << rec.field(i).name() << "-value:" << rec.field(i).value() << "(" << rec.field(i).value().type() << ")";
        if( dkdbstructur["Kreditoren"][rec.field(i).name()].type() == QVariant::Type::String)
            ti.setValue(rec.field(i).name(), rec.field(i).value().toString());
        else if( dkdbstructur["Kreditoren"][rec.field(i).name()].type() == QVariant::Type::LongLong)
            ti.setValue(rec.field(i).name(), rec.field(i).value().toLongLong());
        else if( dkdbstructur["Kreditoren"][rec.field(i).name()].type() == QVariant::Type::Double)
            ti.setValue(rec.field(i).name(), rec.field(i).value().toDouble());
        else
            ti.setValue(rec.field(i).name(), rec.field(i).value());
    }
    return true;
}

bool creditor::isValid() const
{
    QString ignoredMsg;
    return isValid(ignoredMsg);
}

bool creditor::isValid( QString& errortext) const
{//   LOG_CALL;
    errortext.clear();
    if( (ti.getValue("Vorname").toString().isEmpty() && ti.getValue("Vorname").toString().isEmpty())
         ||
        ti.getValue("Strasse").toString().isEmpty()
         ||
        ti.getValue("Plz").toString().isEmpty()
         ||
        ti.getValue("Stadt").toString().isEmpty())
        errortext = "Die Adressdaten sind unvollständig";

    QString email = ti.getValue("Email").toString();
    if( !email.isEmpty() || email == "NULL_STRING")
    {
        QRegularExpression rx("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b",
                              QRegularExpression::CaseInsensitiveOption);
        if( !rx.match(email).hasMatch())
            errortext = "Das Format der e-mail Adresse ist ungültig: " + email;
    }

    IbanValidator iv; int pos = 0;
    QString iban = ti.getValue("IBAN").toString();
    if( !iban.isEmpty())
        if( iv.validate(iban, pos) != IbanValidator::State::Acceptable)
            errortext = "Das Format der IBAN ist nicht korrekt: " + iban;

    if( errortext.isEmpty())
        return true;
    qInfo() << "creditor::isValid found error: " << errortext;
    return false;
}

int creditor::save()
{   LOG_CALL;
    if( ti.getRecord().isEmpty() )
        return -1;
    setId(ti.InsertData());
    return id();
}

int creditor::update() const
{   LOG_CALL;
    if( ti.getRecord().isEmpty())
        return -1;
    return ti.UpdateData();
}

/* static */ bool creditor::Loeschen(int index)
{   LOG_CALL;
    // referential integrity will delete the contracts
    QSqlQuery deleteQ;
    if( !deleteQ.exec("DELETE FROM Kreditoren WHERE Id=" +QString::number(index))) {
        qCritical() << "Delete Kreditor failed "<< deleteQ.lastError() << endl << deleteQ.lastQuery();
        return false;
    }
    else
        return true;
}

/* static */ const dbtable& creditor::getTableDef()
{
    static dbtable creditortable("Kreditoren");
    if( 0 == creditortable.Fields().size())
    {
        creditortable.append(dbfield("id",       QVariant::LongLong, "PRIMARY KEY").setAutoInc());
        creditortable.append(dbfield("Vorname",  QVariant::String).setNotNull());
        creditortable.append(dbfield("Nachname", QVariant::String).setNotNull());
        creditortable.append(dbfield("Strasse",  QVariant::String).setNotNull());
        creditortable.append(dbfield("Plz",      QVariant::String).setNotNull());
        creditortable.append(dbfield("Stadt",    QVariant::String).setNotNull());
        creditortable.append(dbfield("Email",    QVariant::String).setNotNull());
        creditortable.append(dbfield("Anmerkung", QVariant::String).setNotNull());
        creditortable.append(dbfield("IBAN",     QVariant::String).setNotNull());
        creditortable.append(dbfield("BIC",      QVariant::String).setNotNull());
        QVector<dbfield> unique;
        unique.append(creditortable["Vorname"]);
        unique.append(creditortable["Nachname"]);
        unique.append(creditortable["Strasse"]);
        unique.append(creditortable["Stadt"]);
        creditortable.setUnique(unique);
    }
    return creditortable;
}

void creditor::KreditorenListeMitId(QList<QPair<int,QString>> &entries) const
{   LOG_CALL;
    QSqlQuery query;
    query.setForwardOnly(true);
    query.prepare("SELECT id, Vorname, Nachname, Plz, Stadt, Strasse FROM Kreditoren ORDER BY Nachname ASC, Vorname ASC");
    if( !query.exec())
    {
        qCritical() << "Error reading DKGeber while creating a contract: " << query.lastError().text();
        return;
    }

    while(query.next())
    {
        QString Entry = query.value("Nachname").toString() + QString(", ") + query.value("Vorname").toString();
        Entry += QString(", ") + query.value("Plz").toString() + "-" +query.value("Stadt").toString();
        Entry += QString(", ") + query.value("Strasse").toString();
        QList<QPair<int,QString>> entry {{query.value("id").toInt(), Entry}};
        entries.append(entry);
    }
}

creditor saveRandomCreditor()
{
    static QRandomGenerator* rand { QRandomGenerator::system()};
    creditor c;
    c.setFirstname(Vornamen [rand->bounded(Vornamen.count ())]);
    c.setLastname(Nachnamen [rand->bounded(Nachnamen.count ())]);
    c.setStreet(Strassen[rand->bounded(Strassen.count())]);
    int indexCity = rand->bounded(Cities.count());
    c.setPostalCode(Cities[indexCity].first);
    c.setCity(Cities[indexCity].second);
    QString email = c.firstname() + "." + c.lastname() + "@" + emailprovider[rand->bounded(emailprovider.count())];
    email = email.toLower();
    email = email.replace("ü", "ue");
    email = email.replace("ä", "ae");
    email = email.replace("ö", "oe");
    email = email.replace("ß", "ss");

    c.setEmail(email);
    c.setIban(ibans[rand->bounded(ibans.count())]);
    c.setBic("bic...         .");
    Q_ASSERT(c.isValid());
    c.save();
    return c;
}

void saveRandomCreditors(int count)
{
    Q_ASSERT(count>0);
    for( int i = 0; i< count; i++)
        saveRandomCreditor();
}
