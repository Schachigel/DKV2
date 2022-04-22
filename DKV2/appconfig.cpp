#include <iso646.h>
#include <QSettings>
#include <QStandardPaths>
#include <QFileDialog>

#include "helper.h"
#include "helperfin.h"
#include "appconfig.h"
#include "dkdbhelper.h"


/* static data */
#ifndef QT_DEBUG
QString appConfig::keyOutdir = qsl("outdir");
QString appConfig::keyLastDb = qsl("db/last");
#else
QString appConfig::keyOutdir = qsl("dbg-outdir");
QString appConfig::keyLastDb = qsl("dbg-db/last");
#endif

// db config info in 'meta' table
void initMetaInfo( const QString& name, const QString& initialValue, const QSqlDatabase& db)
{   LOG_CALL;
    QVariant value= executeSingleValueSql(dkdbstructur[qsl("Meta")][qsl("Wert")], qsl("Name='%1'").arg(name), db);
    if( value.type() == QVariant::Type::Invalid)
        setMetaInfo(name, initialValue, db);
}
void initNumMetaInfo( const QString& name, const double newValue, const QSqlDatabase& db)
{   LOG_CALL;
    QVariant value= executeSingleValueSql(dkdbstructur[qsl("Meta")][qsl("Wert")], qsl("Name='%1'").arg(name), db);
    if( value.type() == QVariant::Type::Invalid)
        setNumMetaInfo(name, newValue, db);
}
QString getMetaInfo(const QString& name, const QString& def, const QSqlDatabase& db)
{   LOG_CALL_W(name);
    if( not db.isValid()){
        qInfo() << "getMetaInfo: No database ready (yet), defaulting";
        return def;
    }
    QVariant value= executeSingleValueSql(dkdbstructur[qsl("Meta")][qsl("Wert")], qsl("Name='%1'").arg(name), db);
    if( not value.isValid()) {
        qInfo() << "getMetaInfo: read uninitialized property " << name << " -> using default " << def;
        return def;
    }
    qInfo() << "Property " << name << " : " << value;
    return value.toString();
}
double getNumMetaInfo(const QString& name, const double def, const QSqlDatabase& db)
{   LOG_CALL_W(name);
    if( not db.isValid()){
        qInfo() << "no database ready (yet), defaulting";
        return def;
    }
    QVariant value= executeSingleValueSql(dkdbstructur[qsl("Meta")][qsl("Wert")], qsl("Name='%1'").arg(name), db);
    if( not value.isValid()) {
        qInfo() << "getNumProperty read empty property " << name << " -> using default";
        return def;
    }
    qInfo() << "Property " << name << " : " << value.toDouble();
    return value.toDouble();
}
void setMetaInfo(const QString& name, const QString& value, const QSqlDatabase& db, const QString& tblAlias /*=QString()*/)
{   LOG_CALL_W(name);
    QString tname =qsl("Meta");
    if( tblAlias.size()) tname =tblAlias +qsl(".") +tname;
    QString sql{qsl("INSERT OR REPLACE INTO %1 (Name, Wert) VALUES (:name, :value)")};
    executeSql_wNoRecords(sql.arg(tname), {name, value}, db);
}
void setNumMetaInfo(const QString& name, const double value, const QSqlDatabase& db, const QString& tblAlias /*=QString()*/)
{   LOG_CALL_W(name);
    QString tablename =qsl("Meta");
    if( tblAlias.size()) tablename =tblAlias +qsl(".") +tablename;
    QString sql {qsl("INSERT OR REPLACE INTO %1 (Name, Wert) VALUES (:name, :value)")};
    executeSql_wNoRecords(sql.arg(tablename), {name, value}, db);
}

/* statics */
bool appConfig::testmode = false;

/* static methods */
/* static */
void appConfig::setOutDir(const QString& od)
{   LOG_CALL_W(od);
    setUserData(keyOutdir, od);
}
/* static */
void appConfig::setOutDirInteractive(QWidget* parent)
{   LOG_CALL;
    QString dir(getUserData(keyOutdir, QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)));
    dir = QFileDialog::getExistingDirectory(parent, qsl("Ausgabeverzeichnis"), dir,
               QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    setOutDir(dir);
}
/* static */
QString appConfig::Outdir()
{
    QString od;
    do {
        od = getUserData(keyOutdir, QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
        if( od.isEmpty())
            setOutDirInteractive();
    } while (od.isEmpty());
    return od;
}
/* static */ /* for testing puropose */
void appConfig::delOutDir()
{
    qInfo() << "deleting outdir";
    QSettings set;
    set.remove(keyOutdir);
}

/* static */
void appConfig::setLastDb(const QString& filename)
{   LOG_CALL_W(filename);
    setUserData(keyLastDb, filename);
}
/* static */
QString appConfig::LastDb()
{
    static QString defaultDb = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +qsl("/Dkv2.dkdb");
    QString ldb = getUserData(keyLastDb, defaultDb);
    qDebug() << "lastDb read as " << ldb;
    return ldb;
}
/* static */ /* for testing puropose */
void appConfig::delLastDb()
{   LOG_CALL;
    deleteUserData(keyLastDb);
}


QVariantMap getMetaTableAsMap(const QSqlDatabase &db)
{
    LOG_CALL;
    QVariantMap vm;
    QSqlQuery q(db); // default database connection -> active database
    q.setForwardOnly(true);
    if (not q.exec(qsl("SELECT * FROM Meta")))
    {
        qInfo() << "no data returned from Meta table";
        return vm;
    }

    QString name;
    QString value;
    QRegularExpression re("[/\\.]");
    while (q.next())
    {
        QSqlRecord rec = q.record();
        name = rec.value("name").toString().replace(re,"");
        value = rec.value("Wert").toString();
        vm[name] = value;
    }
    return vm;
}

/* private */
/* static */
void appConfig::setUserData(const QString& name, const QString& value)
{
    QSettings config;
    qInfo() << "setUserData " << name << " : " << value;
    config.setValue(name, value);
}
/* static */
QString appConfig::getUserData( const QString& name, const QString& defaultvalue)
{
    QSettings config;
    qInfo() << "getUserData " << name << "; def.Value: " << defaultvalue;
    return config.value(name, defaultvalue).toString();
}
/* static */
void appConfig::deleteUserData(const QString& name)
{   LOG_CALL_W(name);
    QSettings config;
    config.remove(name);
}
// QString getNumUserData(QString name);

QMap<projectConfiguration, QPair<QString, QVariant>> dbConfig::defaultParams ={
    {DB_VERSION,     {qsl("Version"),              QVariant(CURRENT_DB_VERSION)}},
    {DKV2_VERSION,   {qsl("dkv2.exe.Version"),     QVariant(CURRENT_DKV2_VERSION)}},
    {GMBH_PROJECT,   {qsl("gmbh.projekt"),         QVariant("Esperanza")}},
    {GMBH_ADDRESS1,  {qsl("gmbh.address1"),        QVariant("Esperanza Franklin GmbH")}},
    {GMBH_ADDRESS2,  {qsl("gmbh.address2"),        QVariant("")}},
    {GMBH_STREET,    {qsl("gmbh.strasse"),     QVariant("Turley-Platz 9")}},
    {GMBH_PLZ,       {qsl("gmbh.plz"),         QVariant("68167")}},
    {GMBH_CITY,      {qsl("gmbh.stadt"),       QVariant("Mannheim")}},
    {GMBH_EMAIL,     {qsl("gmbh.email"),       QVariant("info@esperanza-mannheim.de")}},
    {GMBH_URL,       {qsl("gmbh.url"),         QVariant("www.esperanza-mannheim.de")}},
    {GMBH_INITIALS,  {qsl("gmbh.Projektinitialen"), QVariant("ESP")}},
    {STARTINDEX,     {qsl("startindex"),       QVariant(111)}},
    {MIN_PAYOUT,     {qsl("minAuszahlung"),    QVariant(100)}},
    {MIN_AMOUNT,     {qsl("minVertragswert"),  QVariant(500)}},
    {MAX_INTEREST,   {qsl("maxZins"),          QVariant(200)}},
    {DBID,           {qsl("dbId"),             QVariant("ESP1234")}},
    {GMBH_HRE,       {qsl("gmbh.Handelsregister"), QVariant("Amtsgericht Mannheim HRB HRB 7.....3")}},
    {GMBH_GEFUE1,    {qsl("gmbh.gefue1"),      QVariant("Gefü 1")}},
    {GMBH_GEFUE2,    {qsl("gmbh.gefue2"),      QVariant("Gefü 2")}},
    {GMBH_GEFUE3,    {qsl("gmbh.gefue3"),      QVariant("Gefü 3")}},
    {GMBH_DKV,       {qsl("gmbh.dkv"),         QVariant("DK Verwalter")}},
    {MAX_INVESTMENT_NBR, {qsl("maxInvestNbr"), QVariant(20)}},
    {MAX_INVESTMENT_SUM, {qsl("maxInvestSum"), QVariant(100000.)}},
    {ZINSUSANCE,     {qsl("Zinsusance"),       QVariant("30/360")}},
    {MAX_PC_INDEX,   {qsl("n/a"),              QVariant()}},
};

/*static*/ QVariant dbConfig::read_DBVersion(const QSqlDatabase& db)
{
    QString invalid(qsl("invalid"));
    QString ret =getMetaInfo(defaultParams.value(DB_VERSION).first, invalid, db);
    if( ret == invalid)
        return QVariant();
    else
        return ret;
}

/*static*/ QString dbConfig::read_DKV2_Version(const QSqlDatabase& db)
{
    QString invalid(qsl("invalid"));
    QString ret =getMetaInfo(defaultParams.value(DKV2_VERSION).first, invalid, db);
    if( ret == invalid)
        return QString();
    else
        return ret;
}

/*static*/ void dbConfig::write_DBVersion(const QSqlDatabase& db, const QString& tblAlias)
{
    writeValue(DB_VERSION, defaultParams.value(DB_VERSION).second, db, tblAlias);
}

/*static*/ void dbConfig::writeDefaults(const QSqlDatabase &db /*=QSqlDatabase::database()*/)
{
    for( int i =0; i< MAX_PC_INDEX; i++) {
        projectConfiguration pc {(projectConfiguration)i};
        writeValue(pc, defaultParams.value(pc).second, db);
    }
}


/*static*/ QVariant dbConfig::readValue(const projectConfiguration pc, const QSqlDatabase& db)
{   LOG_CALL;
    if( isValidIndex(pc))
        return getMetaInfo(defaultParams.value(pc).first, defaultParams.value(pc).second.toString(), db);
    else {
        qCritical() << "invalid parameter requested";
        return QVariant();
    }
}

/*static*/ QString dbConfig::readString(const projectConfiguration pc, const QSqlDatabase &db)
{
    return readValue(pc, db).toString();
}

/*static*/ void dbConfig::writeValue(projectConfiguration pc, const QVariant& value, const QSqlDatabase& db, const QString& tblAlias /*=QString()*/)
{   LOG_CALL;
    if( not isValidIndex(pc)) {
        qCritical() << "invalid paramter to be set";
        return;
    }
    switch (value.type()) {
    case QVariant::Double:
        setNumMetaInfo(defaultParams.value(pc).first, value.toDouble(), db, tblAlias);
        break;
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::String:
    default:
        setMetaInfo(defaultParams.value(pc).first, value.toString(), db, tblAlias);
    }
}



