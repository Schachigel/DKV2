#include <QStringBuilder>
#include <QSqlRecord>
#include <QRegularExpression>
#include <QRandomGenerator>

#include "helper.h"
#include "helperfin.h"
#include "helpersql.h"
#include "creditor.h"
#define qsl(x) QStringLiteral(x)

QList<QString> Vornamen {qsl("Holger"), qsl("Volker"), qsl("Peter"), qsl("Hans"), qsl("Susi"), qsl("Roland"),
            qsl("Claudia"), qsl("Emil"), qsl("Evelyn"), qsl("Ötzgür"), qsl("Thomas"), qsl("Elke"), qsl("Berta"),
            qsl("Malte"), qsl("Jori"), qsl("Paul"), qsl("Jonas"), qsl("Finn"), qsl("Leon"), qsl("Luca"),
            qsl("Emma"), qsl("Mia"), qsl("Lena"), qsl("Anna"), qsl("Anne"), qsl("Martha"), qsl("Ruth"),
            qsl("Rosemie"), qsl("Rosemarie"), qsl("Verena"), qsl("Ursula"), qsl("Erika"), qsl("Adrian"),
            qsl("Avan"), qsl("Anton"), qsl("Benno"), qsl("Karl"), qsl("Merlin"), qsl("Noah"), qsl("Oliver"),
            qsl("Olaf"), qsl("Pepe"), qsl("Zeno"), qsl("Apollo"), qsl("Edward"), qsl("Ronaldo"), qsl("Siegbert"),
            qsl("Thomas"), qsl("Michael")};
QList<QString> Nachnamen {qsl("Maier"), qsl("Müller"), qsl("Schmit"), qsl("Kramp"), qsl("Adams"), qsl("Häcker"),
            qsl("Maresch"), qsl("Beutl"), qsl("Chauchev"), qsl("Chen"), qsl("Kirk"), qsl("Ohura"),
            qsl("Gorbatschov"), qsl("Merkel"), qsl("Karrenbauer"), qsl("Tritin"), qsl("Schmidt"), qsl("Rao"),
            qsl("Lassen"), qsl("Hurgedü"), qsl("vom Dach"), qsl("Langstrumpf"), qsl("Lederstrumpf"),
            qsl("Potter"), qsl("Poppins"), qsl("Wisley"), qsl("Li"), qsl("Wang"), qsl("Ran"), qsl("vom Docht"),
            qsl("Eckrich"), qsl("Staar"), qsl("Funke"), qsl("Engelein"), qsl("Kruffel"), qsl("Calzone")};
QList<QString> Strassen {qsl("Hauptstrasse"), qsl("Nebenstrasse"), qsl("Bahnhofstrasse"), qsl("Kirchstraße"),
            qsl("Dorfstrasse"), qsl("Süterlinweg"), qsl("Sorbenstrasse"), qsl("Kleines Gässchen"),
            qsl("Industriestrasse"), qsl("Sesamstrasse"), qsl("Lindenstrasse"), qsl("Theaterstrasse"),
            qsl("Museumsstrasse"), qsl("Opernplatz"), qsl("Schillerstrasse"), qsl("Lessingstrasse"),
            qsl("Rathausplatz"), qsl("Parkstrasse"), qsl("Turmstrasse"), qsl("Neuer Weg"), qsl("Neuer Anfang"),
            qsl("Main Street"), qsl("Center Court")};
QList<QString> emailprovider {qsl("gmail.com"), qsl("googlemail.com"), qsl("mailbox.org"), qsl("t-online.de"),
            qsl("mail.de"), qsl("mail.com"), qsl("online.de"), qsl("yahoo.de"), qsl("yahoo.com"), qsl("telekom.de"),
            qsl("proivder.co.uk"), qsl("AOL.de"), qsl("outlook.com"), qsl("microsoft.com"), qsl("sap.com"),
            qsl("sap-ag.de"), qsl("abb.de"), qsl("skype.de"), qsl("provider.de")};
QList<QString> ibans {qsl("BG80BNBG96611020345678"), qsl("DE38531742365852502530"), qsl("DE63364408232964251731"),
            qsl("DE38737364268384258531"), qsl("DE69037950954001627624"), qsl("DE63377045386819730665"),
            qsl("DE18851420444163951769"), qsl("DE77921850720298609321"), qsl("DE70402696485599313572"),
            qsl("DE70455395581860402838"), qsl("DE94045704387963352767"), qsl("DE30724236236062816411"),
            qsl("DE62772043290447861437"), qsl("DE33387723124963875990"), qsl("DE15867719874951165967"),
            qsl("DE96720348741083219766"), qsl("DE23152931057149592044"), qsl("DE13220161295670898833"),
            qsl("DE49737651031822324605"), qsl("DE38017168378078601588"), qsl("DE07717138875827514267")};
QList <QPair<QString, QString>> Cities {{qsl("68305"), qsl("Mannheim")}, {qsl("69123"), qsl("Heidelberg")},
                                        {qsl("69123"), qsl("Karlsruhe")}, {qsl("90345"), qsl("Hamburg")},
                                        {qsl("90334"), qsl("Rottenburg")}, {qsl("23345"), qsl("Reinfeld")},
                                        {qsl("83475"), qsl("Dresden")}, {qsl("35725"), qsl("Weissnich")},
                                        {qsl("23245"), qsl("Drieben")}};

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

    QSqlRecord rec = executeSingleRecordSql(dkdbstructur[qsl("Kreditoren")].Fields(), qsl("id=")+QString::number(i));
    if( rec.isEmpty()) return false;

    for(int i=0; i<rec.count(); i++)
    {
        qDebug() << "reading Kreditor from db; Field:" << rec.field(i).name() << "-value:" << rec.field(i).value() << "(" << rec.field(i).value().type() << ")";
        if( dkdbstructur["Kreditoren"][rec.field(i).name()].type() == QVariant::Type::String)
            ti.setValue(rec.field(i).name(), rec.field(i).value().toString());
        else if( dkdbstructur[qsl("Kreditoren")][rec.field(i).name()].type() == QVariant::Type::LongLong)
            ti.setValue(rec.field(i).name(), rec.field(i).value().toLongLong());
        else if( dkdbstructur[qsl("Kreditoren")][rec.field(i).name()].type() == QVariant::Type::Double)
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
    if( (ti.getValue(qsl("Vorname")).toString().isEmpty() && ti.getValue(qsl("Vorname")).toString().isEmpty())
         ||
        ti.getValue(qsl("Strasse")).toString().isEmpty()
         ||
        ti.getValue(qsl("Plz")).toString().isEmpty()
         ||
        ti.getValue(qsl("Stadt")).toString().isEmpty())
        errortext = qsl("Die Adressdaten sind unvollständig");

    QString email = ti.getValue(qsl("Email")).toString();
    if( !email.isEmpty() || email == qsl("NULL_STRING"))
    {
        QRegularExpression rx("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b",
                              QRegularExpression::CaseInsensitiveOption);
        if( !rx.match(email).hasMatch())
            errortext = "Das Format der e-mail Adresse ist ungültig: " + email;
    }

    IbanValidator iv; int pos = 0;
    QString iban = ti.getValue(qsl("IBAN")).toString();
    if( !iban.isEmpty())
        if( iv.validate(iban, pos) != IbanValidator::State::Acceptable)
            errortext = qsl("Das Format der IBAN ist nicht korrekt: ") + iban;

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

bool creditor::remove()
{
    bool ret = creditor::remove(id());
    if( ret) setId( -1);
    return ret;
}

/* static */ bool creditor::remove(int index)
{   LOG_CALL;
    // referential integrity will delete [inactive] contracts (this is w/o bookings)
    // [ creditor <-> contract ] On Delete Cascade
    // deletion with active contracts will fail due to ref. integrity contracts <> bookings
    // [ contract <-> booking ] On Delete Restrict
    QSqlQuery deleteQ;
    if( deleteQ.exec(qsl("DELETE FROM Kreditoren WHERE Id=") +QString::number(index)))
        return true;

    if( "19" == deleteQ.lastError().nativeErrorCode())
        qDebug() << qsl("Delete Kreditor failed due to refer. integrity rules") << Qt::endl << deleteQ.lastQuery();
    else
        qCritical() << qsl("Delete Kreditor failed ")<< deleteQ.lastError() << Qt::endl << deleteQ.lastQuery();
    return false;
}

/* static */ bool creditor::hasActiveContracts(qlonglong i)
{
    // SELECT sum(Buchungen.Betrag) FROM Buchungen
    // WHERE Buchungen.VertragsId IN (SELECT Vertraege.id FROM Vertraege WHERE Vertraege.KreditorId = 14)
    QString where = qsl("Buchungen.VertragsId IN (SELECT Vertraege.id FROM Vertraege WHERE Vertraege.KreditorId = %1)");
    where = where.arg(i);
    QVariant a = executeSingleValueSql(qsl("SUM(Buchungen.Betrag)"), qsl("Buchungen"), where);
    if( a.toDouble() > 0)
        return true;
    return false;
}

/* static */ const dbtable& creditor::getTableDef()
{
    static dbtable creditortable(qsl("Kreditoren"));
    if( 0 == creditortable.Fields().size())
    {
        creditortable.append(dbfield("id",       QVariant::LongLong).setPrimaryKey().setAutoInc());
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
    if( !query.exec(qsl("SELECT id,  Nachname || ', ' || Vorname || ' '||  Plz || '-' || Stadt || ' ' || Strasse FROM Kreditoren ORDER BY Nachname ASC, Vorname ASC")))
    {
        qCritical() << "Error reading DKGeber while creating a contract: " << query.lastError().text();
        return;
    }

    while(query.next())
    {
        QString Entry = query.value(1).toString();
        QList<QPair<int,QString>> entry {{query.value(qsl("id")).toInt(), Entry}};
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
    QString email = c.firstname() % qsl(".") % c.lastname() % qsl("@") + emailprovider[rand->bounded(emailprovider.count())];
    email = email.toLower();
    email = email.replace(qsl("ü"), qsl("ue"));
    email = email.replace(qsl("ä"), qsl("ae"));
    email = email.replace(qsl("ö"), qsl("oe"));
    email = email.replace(qsl("ß"), qsl("ss"));

    c.setEmail(email);
    c.setIban(ibans[rand->bounded(ibans.count())]);
    c.setBic(qsl("bic...         ."));
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
