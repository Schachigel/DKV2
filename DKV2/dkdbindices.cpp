#include "helper.h"
#include "helpersql.h"

const QMap<QString, QString> indices ={
    { qsl("Buchungen_vId"),        qsl("'Buchungen'   ( 'VertragsId')"          )},
    { qsl("Buchungen_vid-bdatum"), qsl("'Buchungen'   ( 'VertragsId', 'Datum')" )},
    { qsl("Buchungen_BArt"),       qsl("'Buchungen'   ( 'BuchungsArt')"         )},
    { qsl("Vertraege_aId"),        qsl("'Vertraege'   ( 'AnlagenId')"           )},
    { qsl("Vertraege_Datum"),      qsl("'Vertraege'   ( 'Vertragsdatum')"       )},
    { qsl("Geldanlagen_Ende"),     qsl("'Geldanlagen' ( 'Ende')"                )}
};

bool createDkDbIndices(const QSqlDatabase& db)
{
    foreach(QString iName ,indices.keys()) {
        if( not createDbIndex(iName, indices[iName], db))
            RETURN_ERR(false, qsl("createDkDbIndices failed"));
    }
    RETURN_OK(true, qsl("createDkDbIndices: indices were created successfully"));
}

