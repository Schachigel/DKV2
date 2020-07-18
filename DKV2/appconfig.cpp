
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
        qCritical() << "Failed to insert Meta information " << q.lastError() << Qt::endl << q.lastQuery();
}
void setNumMetaInfo(const QString& name, const double Wert, QSqlDatabase db)
{   LOG_CALL_W(name);
    QString sql= "INSERT OR REPLACE INTO Meta (Name, Wert) VALUES ('%1', '%2')";
    sql = sql.arg(name).arg(QString::number(Wert));
    QSqlQuery q(db);
    if( !q.exec(sql))
        qCritical() << "Failed to insert Meta information " << q.lastError() << Qt::endl << q.lastQuery();
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
    QFileInfo fi(path);
    setRuntimeData( keyCurrentDb, fi.absoluteFilePath());
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

void dbConfig::loadRuntimeData()
{
    if( appConfig::getRuntimeData(DBID) == "") {
        // runtime data is uninitialized
        return;
    }
    address1   =appConfig::getRuntimeData(GMBH_ADDRESS1);
    address2   =appConfig::getRuntimeData(GMBH_ADDRESS2);
    street     =appConfig::getRuntimeData(GMBH_STREET);
    plz        =appConfig::getRuntimeData(GMBH_PLZ);
    city       =appConfig::getRuntimeData(GMBH_CITY);
    email      =appConfig::getRuntimeData(GMBH_EMAIL);
    url        =appConfig::getRuntimeData(GMBH_URL);
    pi         =appConfig::getRuntimeData(GMBH_PI);
    hre        =appConfig::getRuntimeData(GMBH_HRE);
    gefue1     =appConfig::getRuntimeData(GMBH_GEFUE1);
    gefue2     =appConfig::getRuntimeData(GMBH_GEFUE2);
    gefue3     =appConfig::getRuntimeData(GMBH_GEFUE3);
    dkv        =appConfig::getRuntimeData(GMBH_DKV);
    startindex =appConfig::getRuntimeData(STARTINDEX).toInt();
    minPayout  =appConfig::getRuntimeData(MIN_PAYOUT).toInt();
    minContract=appConfig::getRuntimeData(MIN_AMOUNT).toInt();
    dbId       =appConfig::getRuntimeData(DBID);
    dbVersion  =appConfig::getRuntimeData(DB_VERSION).toDouble();
}

void dbConfig::storeRuntimeData()
{
    appConfig::setRuntimeData(GMBH_ADDRESS1, address1);
    appConfig::setRuntimeData(GMBH_ADDRESS2, address2);
    appConfig::setRuntimeData(GMBH_STREET,   street);
    appConfig::setRuntimeData(GMBH_PLZ,      plz);
    appConfig::setRuntimeData(GMBH_CITY,     city);
    appConfig::setRuntimeData(GMBH_EMAIL,    email);
    appConfig::setRuntimeData(GMBH_URL,      url);
    appConfig::setRuntimeData(GMBH_PI,       pi);
    appConfig::setRuntimeData(GMBH_HRE,      hre);
    appConfig::setRuntimeData(GMBH_GEFUE1,   gefue1);
    appConfig::setRuntimeData(GMBH_GEFUE2,   gefue2);
    appConfig::setRuntimeData(GMBH_GEFUE3,   gefue3);
    appConfig::setRuntimeData(GMBH_DKV,      dkv);
    appConfig::setRuntimeData(STARTINDEX,    QString::number(startindex));
    appConfig::setRuntimeData(MIN_PAYOUT,    QString::number(minPayout));
    appConfig::setRuntimeData(MIN_AMOUNT,    QString::number(minContract));
    appConfig::setRuntimeData(DBID,          dbId);
    appConfig::setRuntimeData(DB_VERSION,    QString::number(dbVersion));
}

void dbConfig::writeDb(QSqlDatabase db)
{
    setMetaInfo(GMBH_ADDRESS1, address1,    db);
    setMetaInfo(GMBH_ADDRESS2, address2,    db);
    setMetaInfo(GMBH_STREET,   street,      db);
    setMetaInfo(GMBH_PLZ,      plz,         db);
    setMetaInfo(GMBH_CITY,     city,        db);
    setMetaInfo(GMBH_EMAIL,    email,       db);
    setMetaInfo(GMBH_URL,      url,         db);
    setMetaInfo(GMBH_PI,       pi,          db);
    setMetaInfo(GMBH_HRE,      hre,         db);
    setMetaInfo(GMBH_GEFUE1,   gefue1,      db);
    setMetaInfo(GMBH_GEFUE2,   gefue2,      db);
    setMetaInfo(GMBH_GEFUE3,   gefue3,      db);
    setMetaInfo(GMBH_DKV,      dkv,         db);
    setNumMetaInfo(STARTINDEX, startindex,  db);
    setNumMetaInfo(MIN_PAYOUT, double(minPayout),   db);
    setNumMetaInfo(MIN_AMOUNT, double(minContract), db);
    setMetaInfo(DBID,          dbId,        db);
    setNumMetaInfo(DB_VERSION, dbVersion,   db);
}

void dbConfig::readDb(QSqlDatabase db)
{
    address1   =getMetaInfo(GMBH_ADDRESS1, address1,   db);
    address2   =getMetaInfo(GMBH_ADDRESS2, address2,   db);
    street     =getMetaInfo(GMBH_STREET,   street,     db);
    plz        =getMetaInfo(GMBH_PLZ,      plz,        db);
    city       =getMetaInfo(GMBH_CITY,     city,       db);
    email      =getMetaInfo(GMBH_EMAIL,    email,      db);
    url        =getMetaInfo(GMBH_URL,      url,        db);
    pi         =getMetaInfo(GMBH_PI,       pi,         db);
    hre        =getMetaInfo(GMBH_HRE,      hre,        db);
    gefue1     =getMetaInfo(GMBH_GEFUE1,   gefue1,     db);
    gefue2     =getMetaInfo(GMBH_GEFUE2,   gefue2,     db);
    gefue3     =getMetaInfo(GMBH_GEFUE3,   gefue3,     db);
    dkv        =getMetaInfo(GMBH_DKV,      dkv,        db);
    startindex =getNumMetaInfo(STARTINDEX, startindex, db);
    minPayout  =getNumMetaInfo(MIN_PAYOUT, minPayout,  db);
    minContract=getNumMetaInfo(MIN_AMOUNT, minContract,db);
    dbId       =getMetaInfo(DBID,          dbId,       db);
    dbVersion  =getNumMetaInfo(DB_VERSION, dbVersion,  db);
}
