#include "dkdbindices.h"

#include "helper_core.h"
#include "helpersql.h"

namespace {

bool createDbIndex( const QString& iName, const QString& iFields, const QSqlDatabase& db)
{
    // sample arguments: qsl("Buchungen_vId"), qsl("'Buchungen'   ( 'VertragsId')")}
    if( not executeSql_wNoRecords (qsl("DROP INDEX IF EXISTS '%1'").arg(iName), db))
        RETURN_ERR(false, qsl("createDbIndex: failed to delete index"));
    if( not executeSql_wNoRecords (qsl("CREATE INDEX '%1' ON %2").arg(iName, iFields), db))
        RETURN_ERR(false, qsl("createDbIndex: failed to create index"), iName, iFields);
    RETURN_OK( true, qsl("successfully created index"), iName);
}
} // eo namespace

const QMap<QString, QString> indices ={
    { qsl("Buchungen_vId"),        qsl("'Buchungen'   ( 'VertragsId')"          )},
    { qsl("Buchungen_vid-bdatum"), qsl("'Buchungen'   ( 'VertragsId', 'Datum' DESC)" )},
    { qsl("Buchungen_vid-bdatum"), qsl("'Buchungen'   ( 'VertragsId', 'Datum' DESC, 'id' DESC)" )},
    { qsl("Buchungen_BArt"),       qsl("'Buchungen'   ( 'BuchungsArt')"         )},
    { qsl("Vertraege_aId"),        qsl("'Vertraege'   ( 'AnlagenId')"           )},
    { qsl("Vertraege_Datum"),      qsl("'Vertraege'   ( 'Vertragsdatum')"       )},
    { qsl("Geldanlagen_Ende"),     qsl("'Geldanlagen' ( 'Ende')"                )}
};

bool createDkDbIndices(const QSqlDatabase& db)
{
    for (auto [iName , index] : indices.asKeyValueRange()) {
        if( not createDbIndex(iName, index, db))
            RETURN_ERR(false, qsl("createDkDbIndices failed"));
    }
    RETURN_OK(true, qsl("createDkDbIndices: indices were created successfully"));
}

