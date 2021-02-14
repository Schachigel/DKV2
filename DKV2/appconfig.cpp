#include <QSettings>
#include <QStandardPaths>
#include <QFileDialog>

#include "helper.h"
#include "helperfin.h"
#include "appconfig.h"
#include "dkdbhelper.h"


const double CURRENT_DB_VERSION {2.5};
/* static data */
#ifndef QT_DEBUG
QString appConfig::keyOutdir = qsl("outdir");
QString appConfig::keyLastDb = qsl("db/last");
#else
QString appConfig::keyOutdir = qsl("dbg-outdir");
QString appConfig::keyLastDb = qsl("dbg-db/last");
#endif

// db config info in 'meta' table
void initMetaInfo( const QString& name, const QString& initialValue, QSqlDatabase db)
{   LOG_CALL;
    QVariant value= executeSingleValueSql(dkdbstructur[qsl("Meta")][qsl("Wert")], qsl("Name='") + name +qsl("'"), db);
    if( value.type() == QVariant::Type::Invalid)
        setMetaInfo(name, initialValue, db);
}
void initNumMetaInfo( const QString& name, const double& newValue, QSqlDatabase db)
{   LOG_CALL;
    QVariant value= executeSingleValueSql(dkdbstructur[qsl("Meta")][qsl("Wert")], qsl("Name='") + name +qsl("'"), db);
    if( value.type() == QVariant::Type::Invalid)
        setNumMetaInfo(name, newValue, db);
}
QString getMetaInfo(const QString& name, const QString& def, QSqlDatabase db)
{   LOG_CALL_W(name);
    if( !db.isValid()){
        qInfo() << "no database ready (yet), defaulting";
        return def;
    }
    QVariant value= executeSingleValueSql(dkdbstructur[qsl("Meta")][qsl("Wert")], qsl("Name='") + name +qsl("'"), db);
    if( ! value.isValid()) {
        qInfo() << "read uninitialized property " << name << " -> using default " << def;
        return def;
    }
    qInfo() << "Property " << name << " : " << value;
    return value.toString();
}
double getNumMetaInfo(const QString& name, const double def, QSqlDatabase db)
{   LOG_CALL_W(name);
    if( !db.isValid()){
        qInfo() << "no database ready (yet), defaulting";
        return def;
    }
    QVariant value= executeSingleValueSql(dkdbstructur[qsl("Meta")][qsl("Wert")], qsl("Name='") +name +qsl("'"), db);
    if( ! value.isValid()) {
        qInfo() << "getNumProperty read empty property " << name << " -> using default";
        return def;
    }
    qInfo() << "Property " << name << " : " << value.toDouble();
    return value.toDouble();
}
void setMetaInfo(const QString& name, const QString& Wert, QSqlDatabase db)
{   LOG_CALL_W(name);
    QSqlQuery q(db);
    QString sql{qsl("INSERT OR REPLACE INTO Meta (Name, Wert) VALUES ('%1', '%2')")};
    sql = sql.arg(name, Wert);
    if( !q.exec(sql))
        qCritical() << "Failed to insert Meta information " << q.lastError() << Qt::endl << q.lastQuery();
}
void setNumMetaInfo(const QString& name, const double Wert, QSqlDatabase db)
{   LOG_CALL_W(name);
    QString sql {qsl("INSERT OR REPLACE INTO Meta (Name, Wert) VALUES ('%1', '%2')")};
    sql = sql.arg(name, QString::number(Wert));
    QSqlQuery q(db);
    if( !q.exec(sql))
        qCritical() << "Failed to insert Meta information " << q.lastError() << Qt::endl << q.lastQuery();
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

/* private */
/* static */
void appConfig::setUserData(const QString& name, const QString& value)
{
    QSettings config;
    qInfo() << "setUserDatea " << name << " : " << value;
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
    {DKV2_VERSION,   {qsl("dkv2.exe.Version"),     QVariant(0)}},
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
    {MIN_AMOUNT,     {qsl("minVertragswert"),  QVariant(599)}},
    {MAX_INTEREST,   {qsl("maxZins"),          QVariant(200)}},
    {DBID,           {qsl("dbId"),             QVariant("ESP1234")}},
    {GMBH_HRE,       {qsl("gmbh.Handelsregister"), QVariant("Amtsgericht Mannheim HRB HRB 7.....3")}},
    {GMBH_GEFUE1,    {qsl("gmbh.gefue1"),      QVariant("Gefü 1")}},
    {GMBH_GEFUE2,    {qsl("gmbh.gefue2"),      QVariant("Gefü 2")}},
    {GMBH_GEFUE3,    {qsl("gmbh.gefue3"),      QVariant("Gefü 3")}},
    {GMBH_DKV,       {qsl("gmbh.dkv"),         QVariant("DK Verwalter")}},
    {MAX_PC_INDEX,   {qsl("n/a"),              QVariant()}}
};

/*static*/ QVariant dbConfig::readVersion(QSqlDatabase db)
{
    QString invalid(qsl("invalid"));
    QString ret =getMetaInfo(defaultParams.value(DB_VERSION).first, invalid, db);
    if( ret == invalid)
        return QVariant();
    else
        return ret;
}

/*static*/ void dbConfig::writeDefaults(QSqlDatabase db /*=QSqlDatabase::database()*/)
{
    for( int i =0; i< MAX_PC_INDEX; i++) {
        projectConfiguration pc {(projectConfiguration)i};
        writeValue(pc, defaultParams.value(pc).second, db);
    }
}


/*static*/ QVariant dbConfig::readValue(projectConfiguration pc, QSqlDatabase db)
{   LOG_CALL;
    if( isValidIndex(pc))
        return getMetaInfo(defaultParams.value(pc).first, defaultParams.value(pc).second.toString(), db);
    else {
        qCritical() << "invalid parameter requested";
        return QVariant();
    }
}

/*static*/ void dbConfig::writeValue(projectConfiguration pc, QVariant value, QSqlDatabase db)
{   LOG_CALL;
    if( ! isValidIndex(pc)) {
        qCritical() << "invalid paramter to be set";
        return;
    }
    switch (value.type()) {
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
    case QVariant::Double:
        setNumMetaInfo(defaultParams.value(pc).first, value.toDouble(), db);
        break;
    default:
        setMetaInfo(defaultParams.value(pc).first, value.toString(), db);
    }
}



