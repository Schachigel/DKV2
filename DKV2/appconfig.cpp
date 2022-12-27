#include "helper.h"
#include "helpersql.h"
#include "dbstructure.h"
#include "dkv2version.h"
#include "appconfig.h"


/* static data */
#ifndef QT_DEBUG
QString appConfig::keyOutdir = qsl("outdir");
QString appConfig::keyLastDb = qsl("db/last");
#else
QString appConfig::keyOutdir = qsl("dbg-outdir");
QString appConfig::keyLastDb = qsl("dbg-db/last");
#endif
const QString appConfig::tnMeta { qsl("Meta")};
const QString appConfig::fnName { qsl("Name")};
const QString appConfig::fnWert { qsl("Wert")};

dbtable appConfig::getTableDef()
{
    static dbtable table(tnMeta);
    table.append(dbfield(qsl("Name"), QVariant::String).setPrimaryKey());
    table.append(dbfield(qsl("Wert"), QVariant::String).setNotNull());

    return table;
}

// db config info in 'meta' table
void initMetaInfo( const QString& name, const QString& initialValue, const QSqlDatabase& db)
{   LOG_CALL;
    QVariant value= executeSingleValueSql(dkdbstructur[qsl("Meta")][qsl("Wert")], qsl("Name='%1'").arg(name), db);
    if( value.type() == QVariant::Invalid)
        setMetaInfo(name, initialValue, db);
}
void initNumMetaInfo( const QString& name, const double newValue, const QSqlDatabase& db)
{   LOG_CALL;
    QVariant value= executeSingleValueSql(dkdbstructur[qsl("Meta")][qsl("Wert")], qsl("Name='%1'").arg(name), db);
    if( value.type() == QVariant::Invalid)
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
    QString defaultDir {QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +qsl("/DKV2")};
    do {
        od = getUserData(keyOutdir, defaultDir);
        if( od.isEmpty())
            setOutDirInteractive();
    } while (od.isEmpty());
    QDir().mkdir (od);
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
bool appConfig::hasLastDb()
{
    QSettings config;
    return config.contains(keyLastDb);
}
/* static */
QString appConfig::LastDb()
{
    static QString defaultDb = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +qsl("/Dkv2.dkdb");
    QString ldb = getUserData(keyLastDb, defaultDb);
    qInfo() << "lastDb read as " << ldb;
    return ldb;
}
/* static */ /* for testing puropose */
void appConfig::delLastDb()
{   LOG_CALL;
    deleteUserData(keyLastDb);
}


QVariantMap getMetaTableAsMap(const QSqlDatabase &db)
{
    QVariantMap vm;
    QVector<QSqlRecord> table;
    if( not executeSql(qsl("SELECT * FROM Meta"), table, db))
        RETURN_ERR(QVariantMap(), QString(__FUNCTION__), qsl("Failed to read meta table"));

    QString name, value;
    static QRegularExpression re("[/\\.]");
    for( const QSqlRecord& record: qAsConst(table))  {
        name  =record.value("name").toString().replace(re, "");
        value =record.value("Wert").toString();
        vm[name] =value;
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
    {DKV2_VERSION,   {qsl("dkv2.exe.Version"),     QVariant(qsl(CURRENT_DKV2_VERSION))}},
    {GMBH_PROJECT,   {qsl("gmbh.projekt"),         QVariant(qsl("Esperanza"))}},
    {GMBH_ADDRESS1,  {qsl("gmbh.address1"),        QVariant(qsl("Esperanza Franklin GmbH"))}},
    {GMBH_ADDRESS2,  {qsl("gmbh.address2"),        QVariant(emptyStringV)}},
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



