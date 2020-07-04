
#include <QString>
#include <QSettings>
#include <QStandardPaths>
#include <QFileDialog>

#include "helper.h"
#include "appconfig.h"
#include "dkdbhelper.h"

/* static data */
#ifndef QT_DEBUG
QString appConfig::keyOutdir = "outdir";
QString appConfig::keyLastDb = "db/last";
QString appConfig::keyCurrentDb = "db/current";
#else
QString appConfig::keyOutdir = "dbg-outdir";
QString appConfig::keyLastDb = "dbg-db/last";
QString appConfig::keyCurrentDb = "dbg-db/current";
#endif

// db config info in 'meta' table
void initMetaInfo( const QString& name, const QString& initialValue, QSqlDatabase db)
{   LOG_CALL;
    QVariant value= executeSingleValueSql(dkdbstructur["Meta"]["Wert"], "Name='" + name +"'", db);
    if( value.type() == QVariant::Type::Invalid)
        setMetaInfo(name, initialValue, db);
}
void initNumMetaInfo( const QString& name, const double& newValue, QSqlDatabase db)
{   LOG_CALL;
    QVariant value= executeSingleValueSql(dkdbstructur["Meta"]["Wert"], "Name='" + name +"'", db);
    if( value.type() == QVariant::Type::Invalid)
        setNumMetaInfo(name, newValue, db);
}
QString getMetaInfo(const QString& name, const QString& def, QSqlDatabase db)
{   LOG_CALL_W(name);
    if( !db.isValid()){
        qInfo() << "no database ready (yet), defaulting";
        return def;
    }
    QVariant value= executeSingleValueSql(dkdbstructur["Meta"]["Wert"], "Name='" + name +"'");
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

    QVariant value= executeSingleValueSql(dkdbstructur["Meta"]["Wert"], "Name='" + name +"'", db);
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
    QString sql="INSERT OR REPLACE INTO Meta (Name, Wert) VALUES ('%1', '%2')";
    sql = sql.arg(name).arg(Wert);
    if( !q.exec(sql))
        qCritical() << "Failed to insert Meta information " << q.lastError() << endl << q.lastQuery();
}
void setNumMetaInfo(const QString& name, const double Wert, QSqlDatabase db)
{   LOG_CALL_W(name);
    QString sql= "INSERT OR REPLACE INTO Meta (Name, Wert) VALUES ('%1', '%2')";
    sql = sql.arg(name).arg(QString::number(Wert));
    QSqlQuery q(db);
    if( !q.exec(sql))
        qCritical() << "Failed to insert Meta information " << q.lastError() << endl << q.lastQuery();
}

/* statics */
QMap<QString, QString> appConfig::runtimedata;
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
    dir = QFileDialog::getExistingDirectory(parent, "Ausgabeverzeichnis", dir,
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
    QString ldb = getUserData(keyLastDb);
    qDebug() << "lastDb read as " << ldb;
    return ldb;
}
/* static */ /* for testing puropose */
void appConfig::delLastDb()
{
    deleteUserData(keyLastDb);
}

/* static */
void appConfig::setCurrentDb(const QString& path)
{   LOG_CALL_W(path);
    setRuntimeData( keyCurrentDb, path);
}
/* static */
QString appConfig::CurrentDb()
{
    QString cdb = getRuntimeData(keyCurrentDb);
    return cdb;
}
/* static */ /* for testing puropose */
void appConfig::delCurrentDb()
{
    deleteRuntimeData(keyCurrentDb);
}

/* private */
/* static */
void appConfig::setUserData(const QString& name, const QString& value)
{
    QSettings config;
    config.setValue(name, value);
}
/* static */
QString appConfig::getUserData( const QString& name, const QString& defaultvalue)
{
    QSettings config;
    return config.value(name, defaultvalue).toString();
}
/* static */
void appConfig::deleteUserData(const QString& name)
{
    QSettings config;
    config.remove(name);
}
// QString getNumUserData(QString name);

/* static */
void appConfig::setRuntimeData( const QString& name, const QString& value)
{
    runtimedata.insert(name,value);
}
/* static */
QString appConfig::getRuntimeData( const QString& name, const QString& defaultvalue)
{
    return runtimedata.value(name, defaultvalue);
}
/* static */
void appConfig::deleteRuntimeData(const QString& name)
{
    runtimedata.remove(name);
}

/* static */ dbConfig dbConfig::fromRuntimeData()
{
    dbConfig c;
    c.address1 =appConfig::getRuntimeData(GMBH_ADDRESS1);
    c.address2 =appConfig::getRuntimeData(GMBH_ADDRESS2);
    c.street   =appConfig::getRuntimeData(GMBH_STREET);
    c.plz      =appConfig::getRuntimeData(GMBH_PLZ);
    c.city     =appConfig::getRuntimeData(GMBH_CITY);
    c.email    =appConfig::getRuntimeData(GMBH_EMAIL);
    c.url      =appConfig::getRuntimeData(GMBH_URL);
    c.pi       =appConfig::getRuntimeData(GMBH_PI);
    c.startindex=appConfig::getRuntimeData(STARTINDEX).toInt();
    c.dbId     =appConfig::getRuntimeData(DBID);
    c.dbVersion=appConfig::getRuntimeData(DB_VERSION).toDouble();
    c.hre      =appConfig::getRuntimeData(GMBH_HRE);
    c.gefue1   =appConfig::getRuntimeData(GMBH_GEFUE1);
    c.gefue2   =appConfig::getRuntimeData(GMBH_GEFUE2);
    c.gefue3   =appConfig::getRuntimeData(GMBH_GEFUE3);
    c.dkv      =appConfig::getRuntimeData(GMBH_DKV);
    return c;
}

void dbConfig::toRuntimeData()
{
    appConfig::setRuntimeData(GMBH_ADDRESS1, address1);
    appConfig::setRuntimeData(GMBH_ADDRESS2, address2);
    appConfig::setRuntimeData(GMBH_STREET,   street);
    appConfig::setRuntimeData(GMBH_PLZ,      plz);
    appConfig::setRuntimeData(GMBH_CITY,     city);
    appConfig::setRuntimeData(GMBH_EMAIL,    email);
    appConfig::setRuntimeData(GMBH_URL,      url);
    appConfig::setRuntimeData(GMBH_PI,       pi);
    appConfig::setRuntimeData(STARTINDEX,    QString::number(startindex));
    appConfig::setRuntimeData(DBID,          dbId);
    appConfig::setRuntimeData(DB_VERSION,    QString::number(dbVersion));
    appConfig::setRuntimeData(GMBH_HRE,      hre);
    appConfig::setRuntimeData(GMBH_GEFUE1,   gefue1);
    appConfig::setRuntimeData(GMBH_GEFUE2,   gefue2);
    appConfig::setRuntimeData(GMBH_GEFUE3,   gefue3);
    appConfig::setRuntimeData(GMBH_DKV,      dkv);
}

void dbConfig::toDb(QSqlDatabase db)
{
    setMetaInfo(GMBH_ADDRESS1, address1, db);
    setMetaInfo(GMBH_ADDRESS2, address2, db);
    setMetaInfo(GMBH_STREET,   street,   db);
    setMetaInfo(GMBH_PLZ,      plz,      db);
    setMetaInfo(GMBH_CITY,     city,     db);
    setMetaInfo(GMBH_EMAIL,    email,    db);
    setMetaInfo(GMBH_URL,      url,      db);
    setMetaInfo(GMBH_PI,       pi,       db);
    setNumMetaInfo(STARTINDEX,    startindex, db);
    setMetaInfo(DBID,          dbId,     db);
    setNumMetaInfo(DB_VERSION, dbVersion,db);
    setMetaInfo(GMBH_HRE,      hre,      db);
    setMetaInfo(GMBH_GEFUE1,   gefue1,   db);
    setMetaInfo(GMBH_GEFUE2,   gefue2,   db);
    setMetaInfo(GMBH_GEFUE3,   gefue3,   db);
    setMetaInfo(GMBH_DKV,      dkv,      db);
}

/* static */ dbConfig dbConfig::fromDb(QSqlDatabase db)
{
    dbConfig c;
    c.address1 =getMetaInfo(GMBH_ADDRESS1, c.address1,  db);
    c.address2 =getMetaInfo(GMBH_ADDRESS2, c.address2,  db);
    c.street   =getMetaInfo(GMBH_STREET,   c.street,    db);
    c.plz      =getMetaInfo(GMBH_PLZ,      c.plz,       db);
    c.city     =getMetaInfo(GMBH_CITY,     c.city,      db);
    c.email    =getMetaInfo(GMBH_EMAIL,    c.email,     db);
    c.url      =getMetaInfo(GMBH_URL,      c.url,       db);
    c.pi       =getMetaInfo(GMBH_PI,       c.pi,        db);
    c.startindex=getNumMetaInfo(STARTINDEX, c.startindex, db);
    c.dbId     =getMetaInfo(DBID,          c.dbId,      db);
    c.dbVersion=getNumMetaInfo(DB_VERSION, c.dbVersion, db);
    c.hre      =getMetaInfo(GMBH_HRE,      c.hre,       db);
    c.gefue1   =getMetaInfo(GMBH_GEFUE1,   c.gefue1,    db);
    c.gefue2   =getMetaInfo(GMBH_GEFUE2,   c.gefue2,    db);
    c.gefue3   =getMetaInfo(GMBH_GEFUE3,   c.gefue3,    db);
    c.dkv      =getMetaInfo(GMBH_DKV,      c.dkv,       db);
    return c;
}
