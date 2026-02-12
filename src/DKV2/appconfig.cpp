#include "appconfig.h"
#include "helper_core.h"
#include "helpersql.h"
#include "dbstructure.h"
#include "dkv2version.h"


/* static data */
#ifndef QT_DEBUG
QString appconfig::keyOutdir = qsl("outdir");
QString appconfig::keyLastDb = qsl("db/last");
#else
QString appconfig::keyOutdir = qsl("dbg-outdir");
QString appconfig::keyLastDb = qsl("dbg-db/last");
#endif
const QString appconfig::tnMeta { qsl("Meta")};
const QString appconfig::fnName { qsl("Name")};
const QString appconfig::fnWert { qsl("Wert")};

dbtable appconfig::getTableDef()
{
    static dbtable table(tnMeta);
    if( table.Fields().size() == 0) {
        table.append(dbfield(qsl("Name"), QMetaType::QString).setPrimaryKey());
        table.append(dbfield(qsl("Wert"), QMetaType::QString).setNotNull());
    }

    return table;
}

// db config info in 'meta' table
void initMetaInfo( const QString& name, const QString& initialValue, const QSqlDatabase& db)
{   LOG_CALL;
    QVariant value= executeSingleValueSql(dkdbstructur[qsl("Meta")][qsl("Wert")], qsl("Name='%1'").arg(name), db);
    if( value.metaType().id() == QMetaType::UnknownType)
        setMetaInfo(name, initialValue, db);
}
void initNumMetaInfo( const QString& name, const double newValue, const QSqlDatabase& db)
{   LOG_CALL;
    QVariant value= executeSingleValueSql(dkdbstructur[qsl("Meta")][qsl("Wert")], qsl("Name='%1'").arg(name), db);
    if( value.metaType().id() == QMetaType::UnknownType)
        setNumMetaInfo(name, newValue, db);
}
QString getMetaInfo(const QString& name, const QString& def, const QSqlDatabase& db)
{
    if( not db.isValid())
        RETURN_OK(def, qsl("getMetaInfo: No database ready (yet), defaulting"));

    QVariant value= executeSingleValueSql(dkdbstructur[qsl("Meta")][qsl("Wert")], qsl("Name='%1'").arg(name), db);
    if( not value.isValid())
        RETURN_OK( def, qsl("getMetaInfo: read uninitialized property "), qsl(" -> using default "), def);

    RETURN_OK( value.toString(), qsl("getMetaInfo: Property "), value.toString ());
}
double getNumMetaInfo(const QString& name, const double def, const QSqlDatabase& db)
{
    if( not db.isValid())
        RETURN_OK( def, qsl("getNumMetaInfo: No database ready (yet), defaulting"));

    QVariant value= executeSingleValueSql(dkdbstructur[qsl("Meta")][qsl("Wert")], qsl("Name='%1'").arg(name), db);
    if( not value.isValid())
        RETURN_OK( def, qsl("getNumMetaInfo: Read empty property "), name, qsl( "> using default"));
    RETURN_OK( value.toDouble(), qsl("getNumMetaInfo: "), name, value.toString());
}
void setMetaInfo(const QString& name, const QString& value, const QSqlDatabase& db, const QString& tblAlias /*=QString()*/)
{   LOG_CALL_W(name);
    QString tname =qsl("Meta");
    if( tblAlias.size()) tname =tblAlias +qsl(".") +tname;
    QString sql{qsl("INSERT OR REPLACE INTO %1 (Name, Wert) VALUES (:name, :value)")};
    if( not executeSql_wNoRecords(sql.arg(tname), {name, value}, db)) {
        qInfo() << QSqlDatabase::database().lastError();
        qCritical() << "writing meta info failed " << name << " : " << value;
    }

}
void setNumMetaInfo(const QString& name, const double value, const QSqlDatabase& db, const QString& tblAlias /*=QString()*/)
{   LOG_CALL_W(name);
    QString tablename =qsl("Meta");
    if( tblAlias.size()) tablename =tblAlias +qsl(".") +tablename;
    QString sql {qsl("INSERT OR REPLACE INTO %1 (Name, Wert) VALUES (:name, :value)")};
    executeSql_wNoRecords(sql.arg(tablename), {name, value}, db);
}

/* statics */
bool appconfig::testmode = false;

void appconfig::setTestMode(bool on)
{
    testmode = on;
}

bool appconfig::isTestMode()
{
    return testmode;
}

QString appconfig::testBaseDir()
{
    const QString cwd = QDir::currentPath();
    return QDir::cleanPath(QDir(cwd).filePath(qsl("../data")));
}

QString appconfig::testOutDir()
{
    return QDir::cleanPath(QDir(testBaseDir()).filePath(qsl("output")));
}

/* static methods */
/* static */
void appconfig::setOutDir(const QString& od)
{   LOG_CALL_W(od);
    setUserData(keyOutdir, od);
}
/* static */
QString appconfig::Outdir()
{
    QString od;
    QString defaultDir;
    if (testmode)
        defaultDir = testOutDir();
    else
        defaultDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +qsl("/DKV2");
    return getUserData(keyOutdir, defaultDir);
}
/* static */ /* for testing puropose */
void appconfig::delOutDir()
{
    qInfo() << "deleting outdir";
    deleteUserData(keyOutdir);
}

/* static */
void appconfig::setLastDb(const QString& filename)
{   LOG_CALL_W(filename);
    setUserData(keyLastDb, filename);
}
/* static */
bool appconfig::hasLastDb()
{
    QSettings config;
    return config.contains(makeUserKey(keyLastDb));
}
/* static */
QString appconfig::LastDb()
{
    static QString defaultDb = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +qsl("/Dkv2.dkdb");
    QString ldb = getUserData(keyLastDb, defaultDb);
    qInfo() << "lastDb read as " << ldb;
    return ldb;
}
/* static */ /* for testing puropose */
void appconfig::delLastDb()
{   LOG_CALL;
    deleteUserData(keyLastDb);
}

QString appconfig::keynameZoom {qsl("Zoom")};
double appconfig::Zoom()
{
    QString sZoom =getUserData(keynameZoom, qsl("1.2"));
    double dZoom =sZoom.toDouble();
    if( dZoom >= 0.5 and dZoom < 2)
        return dZoom;
    else {
        setZoom( 1.1);
        return 1.1;
    }
}
void appconfig::setZoom(double d)
{
    if( d >= 0.5 and d < 2)
        setUserData(keynameZoom, QString::number(d));
}

namespace {
static QMap<QString, QVariant> runtimeData; // clazy:exclude=non-pod-global-static
}
void appconfig::setRuntimeData( const QString& name, const QVariant& value)
{
    runtimeData[name] =value;
}
QVariant appconfig::getRuntimeData( const QString& name)
{
    return runtimeData.value(name);
}


QVariantMap getMetaTableAsMap(const QSqlDatabase &db)
{
    QVariantMap vm;
    QVector<QSqlRecord> table;
    if( not executeSql(qsl("SELECT * FROM Meta"), table, db))
        RETURN_ERR(QVariantMap(), QString(__FUNCTION__), qsl("Failed to read meta table"));

    QString name, value;
    static QRegularExpression re(qsl("[/\\.]"));
    for( const QSqlRecord& record: std::as_const(table))  {
        name  =record.value("name").toString().replace(re, "");
        value =record.value("Wert").toString();
        vm[name] =value;
    }
    return vm;
}

/* private */
/* static */
void appconfig::setUserData(const QString& name, const QString& value)
{
    QSettings config;
    const QString key = makeUserKey(name);
    qInfo() << "setUserData " << key << " : " << value;
    config.setValue(key, value);
    config.sync();
}
/* static */
QString appconfig::getUserData( const QString& name, const QString& defaultvalue)
{
    QSettings config;
    const QString key = makeUserKey(name);
    qInfo() << "getUserData " << key << "; def.Value: " << defaultvalue;
    return config.value(key, defaultvalue).toString();
}
/* static */
void appconfig::deleteUserData(const QString& name)
{   LOG_CALL_W(name);
    QSettings config;
    config.remove(makeUserKey(name));
}

QString appconfig::makeUserKey(const QString& name)
{
    if (testmode)
        return qsl("test/") + name;
    return name;
}
// QString getNumUserData(QString name);

QMap<projectConfiguration, QPair<QString, QVariant>> dbConfig::defaultParams ={
    {DB_VERSION,     {qsl("Version"),              QVariant(CURRENT_DB_VERSION)}},
    {DKV2_VERSION,   {qsl("dkv2.exe.Version"),     QVariant(qsl(CURRENT_DKV2_VERSION))}},
    {GMBH_PROJECT,   {qsl("gmbh.projekt"),         QVariant(qsl("Esperanza"))}},
    {GMBH_ADDRESS1,  {qsl("gmbh.address1"),        QVariant(qsl("address 1"))}},
    {GMBH_ADDRESS2,  {qsl("gmbh.address2"),        QVariant(QString(""))}},
    {GMBH_STREET,    {qsl("gmbh.strasse"),     QVariant(qsl("Turley-Platz 8-9"))}},
    {GMBH_PLZ,       {qsl("gmbh.plz"),         QVariant(qsl("68167"))}},
    {GMBH_CITY,      {qsl("gmbh.stadt"),       QVariant(qsl("Mannheim"))}},
    {GMBH_EMAIL,     {qsl("gmbh.email"),       QVariant(qsl("info@esperanza-mannheim.de"))}},
    {GMBH_URL,       {qsl("gmbh.url"),         QVariant(qsl("www.esperanza-mannheim.de"))}},
    {GMBH_INITIALS,  {qsl("gmbh.Projektinitialen"), QVariant(qsl("ESP"))}},
    {STARTINDEX,     {qsl("startindex"),       QVariant(111)}},
    {MIN_PAYOUT,     {qsl("minAuszahlung"),    QVariant(100)}},
    {MIN_AMOUNT,     {qsl("minVertragswert"),  QVariant(500)}},
    {MAX_INTEREST,   {qsl("maxZins"),          QVariant(200)}},
    {DBID,           {qsl("dbId"),             QVariant(qsl("ESP1234"))}},
    {GMBH_HRE,       {qsl("gmbh.Handelsregister"), QVariant(qsl("Amtsgericht Mannheim HRB HRB 7.....3"))}},
    {GMBH_GEFUE1,    {qsl("gmbh.gefue1"),      QVariant(qsl("Gefü 1"))}},
    {GMBH_GEFUE2,    {qsl("gmbh.gefue2"),      QVariant(qsl("Gefü 2"))}},
    {GMBH_GEFUE3,    {qsl("gmbh.gefue3"),      QVariant(qsl("Gefü 3"))}},
    {GMBH_DKV,       {qsl("gmbh.dkv"),         QVariant(qsl("DK Verwalter"))}},
    {MAX_INVESTMENT_NBR, {qsl("maxInvestNbr"), QVariant(20)}},
    {MAX_INVESTMENT_SUM, {qsl("maxInvestSum"), QVariant(100000.)}},
    {ZINSUSANCE,     {qsl("Zinsusance"),       QVariant(qsl("30/360"))}},
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
        projectConfiguration pc {static_cast<projectConfiguration>(i)};
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
    switch (value.metaType().id()) {
    case QMetaType::Double:
        setNumMetaInfo(defaultParams.value(pc).first, value.toDouble(), db, tblAlias);
        break;
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
    case QMetaType::QString:
    default:
        setMetaInfo(defaultParams.value(pc).first, value.toString(), db, tblAlias);
    }
}
