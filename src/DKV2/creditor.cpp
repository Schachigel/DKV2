
#include "creditor.h"
#include "helpersql.h"
#include "helperfin.h"
#include "dbstructure.h"
//#include "ibanvalidator.h"
#include "booking.h"

const QString creditor::fnId           {qsl("id")};
const QString creditor::tablename      {qsl("Kreditoren")};
const QString creditor::fnVorname      {qsl("Vorname")};
const QString creditor::fnNachname     {qsl("Nachname")};
const QString creditor::fnStrasse      {qsl("Strasse")};
const QString creditor::fnPlz          {qsl("Plz")};
const QString creditor::fnStadt        {qsl("Stadt")};
const QString creditor::fnLand         {qsl("Land")};
const QString creditor::fnTel          {qsl("Telefon")};
const QString creditor::fnEmail        {qsl("Email")};
const QString creditor::fnAnmerkung    {qsl("Anmerkung")};
const QString creditor::fnKontakt      {qsl("Kontakt")};
const QString creditor::fnBuchungskonto{qsl("Buchungskonto")};
const QString creditor::fnIBAN         {qsl("IBAN")};
const QString creditor::fnBIC          {qsl("BIC")};
const QString creditor::fnZeitstepel   {qsl("Zeitstempel")};


bool creditor::operator==(const creditor& c) const
{
    bool ret = false;
    do{
        if( id()        not_eq c.id()) break;
        if( firstname() not_eq c.firstname()) break;
        if( lastname()  not_eq c.lastname()) break;
        if( street()    not_eq c.street()) break;
        if( postalCode() not_eq c.postalCode()) break;
        if( city()      not_eq c.city()) break;
        if( comment()   not_eq c.comment()) break;
        if (email() not_eq c.email()) break;
        if( tel()       not_eq c.tel()) break;
        if (contact() not_eq c.contact()) break;
        if (account() not_eq c.account()) break;
        if( iban()      not_eq c.iban()) break;
        if( bic()       not_eq c.bic()) break;
        ret = true;
    } while(false);
    if( not ret) {
        qInfo() << id() << " vs. " << c.id();
        qInfo() << firstname() << " vs. " << c.firstname();
        qInfo() << lastname() << " vs. " << c.lastname();
        qInfo() << street() << " vs. " << c.street();
        qInfo() << postalCode() << " vs. " << c.postalCode();
        qInfo() << city() << " vs. " << c.city();
        qInfo() << comment() << " vs. " << c.comment();
        qInfo() << email() << " vs. " << c.email();
        qInfo() << tel() << " vs. " << c.tel();
        qInfo() << contact() << " vs. " << c.contact();
        qInfo() << account() << " vs. " << c.account();
        qInfo() << iban() << " vs. " << c.iban();
        qInfo() << bic() << " vs. " << c.bic();
    }

    return ret;
}

bool creditor::fromDb( const qlonglong id)
{   LOG_CALL;

    QSqlRecord rec = executeSingleRecordSql(dkdbstructur[qsl("Kreditoren")].Fields(), qsl("id=")+i2s(id));
    if( rec.isEmpty()) return false;

    for(int i=0; i<rec.count(); i++)
    {
        qInfo() << "reading Kreditor from db; Field:" << rec.field(i).name() << "-value:" << rec.field(i).value() << "(" << rec.field(i).value().metaType().name() << ")";
        if( dkdbstructur[tablename][rec.field(i).name()].metaType().id() == QMetaType::QString)
            ti.setValue(rec.field(i).name(), rec.field(i).value().toString());
        else if( dkdbstructur[tablename][rec.field(i).name()].value().metaType().id() == QMetaType::LongLong)
            ti.setValue(rec.field(i).name(), rec.field(i).value().toLongLong());
        else if( dkdbstructur[tablename][rec.field(i).name()].value().metaType().id() == QMetaType::Double)
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
    if( (ti.getValue(fnVorname).toString().isEmpty() and ti.getValue(qsl("Nachname")).toString().isEmpty())
         ||
        ti.getValue(fnStrasse).toString().isEmpty()
         ||
        ti.getValue(fnPlz).toString().isEmpty()
         ||
        ti.getValue(fnStadt).toString().isEmpty())
        errortext = qsl("Die Adressdaten sind unvollständig");
    QString email = ti.getValue(fnEmail).toString();

    static const QRegularExpression rx(qsl("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b"),
                                       QRegularExpression::CaseInsensitiveOption);

    if( email.size() or email == qsl("NULL_STRING")){
        if( not rx.match(email).hasMatch())
            errortext = "Das Format der e-mail Adresse ist ungültig: " + email;
    }
    QString iban = ti.getValue(qsl("IBAN")).toString();
    if( iban.size()) {
        if( not isValidIban(iban))
            errortext = qsl("Das Format der IBAN ist nicht korrekt: ") + iban;
    }
    if( errortext.isEmpty())
        return true;
    qInfo() << "creditor::isValid found error: " << errortext;
    return false;
}

QVariantMap creditor::getVariantMap()
{
    QVariantMap v;
    v["id"] = ti.getValue(fnId);
    v["strId"] = v["id"].toString();
    v["Vorname"] = ti.getValue(fnVorname).toString();
    v["Nachname"] = ti.getValue(fnNachname).toString();
    v["Strasse"] = ti.getValue(fnStrasse).toString();
    v["Plz"] = ti.getValue(fnPlz).toString();
    v["Stadt"] = ti.getValue(fnStadt).toString();
    v["Land"] = ti.getValue(fnLand).toString();
    v["Email"] = ti.getValue(fnEmail).toString();
    v["Telefon"] = ti.getValue(fnTel).toString();
    v["Kontakt"] = ti.getValue(fnKontakt).toString();
    v["Buchungskonto"] = ti.getValue(fnBuchungskonto).toString();
    v["IBAN"] = ti.getValue(fnIBAN).toString();
    v["BIC"] = ti.getValue(fnBIC).toString();
    v["Anmerkung"] = getValue(fnAnmerkung).toString();
    return v;
}

tableindex_t creditor::save()
{   LOG_CALL;
    if( ti.getRecord().isEmpty() )
        return SQLITE_invalidRowId;
    setId(ti.InsertRecord());
    return id();
}

tableindex_t creditor::update() const
{   LOG_CALL;
    if( ti.getRecord().isEmpty())
        return SQLITE_invalidRowId;
    return ti.UpdateRecord();
}

bool creditor::remove()
{
    bool ret = creditor::remove(id());
    if( ret) setId( SQLITE_invalidRowId);
    return ret;
}

/* static */ bool creditor::remove(const tableindex_t index)
{   LOG_CALL;
    // referential integrity will delete [inactive] contracts (this is w/o bookings)
    // [ creditor <-> contract ] On Delete Cascade
    // deletion with active contracts will fail due to ref. integrity contracts <> bookings
    // [ contract <-> booking ] On Delete Restrict
    if( executeSql_wNoRecords(qsl("DELETE FROM Kreditoren WHERE Id=") +i2s(index)))
        return true;
    else
        return false;
}

/* static */ bool creditor::hasActiveContracts(const qlonglong i)
{
    // SELECT sum(Buchungen.Betrag) FROM Buchungen
    // WHERE Buchungen.VertragsId IN (SELECT Vertraege.id FROM Vertraege WHERE Vertraege.KreditorId = 14)
    QString where = qsl("Buchungen.VertragsId IN (SELECT Vertraege.id FROM Vertraege WHERE Vertraege.KreditorId = %1)");
    where = where.arg(i);
    QVariant a = executeSingleValueSql(qsl("SUM(%1.%2)").arg(tn_Buchungen, fn_bBetrag), tn_Buchungen, where);
    if( a.toDouble() > 0)
        return true;
    return false;
}

/* static */ const dbtable& creditor::getTableDef()
{
    static dbtable creditortable(tablename);
    if( 0 == creditortable.Fields().size())
    {
        creditortable.append(dbfield(fnId,       QMetaType::LongLong).setAutoInc());
        creditortable.append(dbfield(fnVorname,  QMetaType::QString).setDefault(emptyStringV));
        creditortable.append(dbfield(fnNachname, QMetaType::QString).setDefault(emptyStringV));
        creditortable.append(dbfield(fnStrasse,  QMetaType::QString).setDefault(emptyStringV));
        creditortable.append(dbfield(fnPlz,      QMetaType::QString).setDefault(emptyStringV));
        creditortable.append(dbfield(fnStadt,    QMetaType::QString).setDefault(emptyStringV));
        creditortable.append(dbfield(fnLand,     QMetaType::QString).setDefault(emptyStringV));
        creditortable.append(dbfield(fnTel,      QMetaType::QString).setDefault(emptyStringV));
        creditortable.append(dbfield(fnEmail,    QMetaType::QString).setDefault(emptyStringV));
        creditortable.append(dbfield(fnAnmerkung,QMetaType::QString).setDefault(emptyStringV));
        creditortable.append(dbfield(fnKontakt,  QMetaType::QString).setDefault(emptyStringV));
        creditortable.append(dbfield(fnBuchungskonto,QMetaType::QString).setDefault(emptyStringV));
        creditortable.append(dbfield(fnIBAN,     QMetaType::QString).setDefault(emptyStringV));
        creditortable.append(dbfield(fnBIC,      QMetaType::QString).setDefault(emptyStringV));
        creditortable.append(dbfield(fnZeitstepel, QMetaType::QDateTime).setDefaultNow());
        QVector<dbfield> unique;
        unique.append(creditortable[fnVorname]);
        unique.append(creditortable[fnNachname]);
        unique.append(creditortable[fnStrasse]);
        unique.append(creditortable[fnStadt]);
        creditortable.setUnique(unique);
    }
    return creditortable;
}

void getAllCreditorInfoSorted(QList<QPair<qlonglong,QString>>& entries)
{   LOG_CALL;
    QString sql;
        sql =qsl(R"str(
SELECT id
  , Nachname || ', ' || Vorname || ' '||  Plz || '-' || Stadt || ' ' || Strasse
FROM Kreditoren
ORDER BY Nachname ASC, Vorname ASC
)str");
    QVector<QSqlRecord> result;
    if( not executeSql(sql, result)) {
        qCritical() << "Error reading DKGeber while creating a contract";
        return;
    }
    for( const auto& record: std::as_const( result))
        entries.append({record.value(0).toLongLong (), record.value(1).toString ()});
}

void creditorsWithAnnualSettlement(QList<tableindex_t> &entries, const int bookingYear)
{   LOG_CALL;
    if( bookingYear < 1950) {
        qCritical() << "invalid booking year reading DKGeber";
        return;
    }

    QString sql{qsl(R"str(
SELECT DISTINCT id FROM
    (
    SELECT id, Nachname, Vorname
    FROM Kreditoren
    WHERE id IN (
        SELECT DISTINCT Kreditoren.Id AS kid
        FROM Buchungen
        INNER JOIN Vertraege ON Buchungen.VertragsId = Vertraege.id
        INNER JOIN Kreditoren ON Vertraege.KreditorId = Kreditoren.id
        WHERE (Buchungen.BuchungsArt = %1 OR Buchungen.BuchungsArt = %2) AND SUBSTR(Buchungen.Datum, 1, 4) = '%3'
        UNION
        SELECT DISTINCT Kreditoren.Id AS kid
        FROM exBuchungen
        INNER JOIN exVertraege ON exBuchungen.VertragsId = exVertraege.id
        INNER JOIN Kreditoren ON exVertraege.KreditorId = Kreditoren.id
        WHERE (exBuchungen.BuchungsArt = %1 OR exBuchungen.BuchungsArt = %2) AND SUBSTR(exBuchungen.Datum, 1, 4) = '%3'
        )
        ORDER BY Nachname ASC, Vorname ASC
    )
)str")
                    .arg(bookingTypeToNbrString(bookingType::annualInterestDeposit),
                         bookingTypeToNbrString(bookingType::reInvestInterest),
                         i2s(bookingYear))};

    QVector<QSqlRecord> result;
    if( not executeSql(sql, result)) {
        qCritical() << "error getting creditors";
        return;
    }
    for(const auto& record : std::as_const(result))
        entries.append (record.value(0).toLongLong ());
}

namespace {
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
            qsl("Lassen"), qsl("Hurgedue"), qsl("vom Dach"), qsl("Langstrumpf"), qsl("Lederstrumpf"),
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
}

creditor saveRandomCreditor()
{
    static QRandomGenerator* rand { QRandomGenerator::system()};
    static int count =0;
    creditor c;
    c.setFirstname(Vornamen [rand->bounded(Vornamen.count ())]);
    c.setLastname(Nachnamen [rand->bounded(Nachnamen.count ())]);
    c.setStreet(Strassen[rand->bounded(Strassen.count())] +i2s(count++));
    int indexCity = rand->bounded((int)Cities.count());
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
    c.setComment("");
    c.setTel("");
    c.setContact("");
    c.setAccount("");
    Q_ASSERT(c.isValid());
    c.save();
    return c;
}

void saveRandomCreditors(const int count)
{
    Q_ASSERT(count>0);
    for( int i = 0; i< count; i++)
        saveRandomCreditor();
}
