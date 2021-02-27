
#include "helper.h"
#include "investment.h"

investment::investment()
{

}

/*static*/ const dbtable& investment::getTableDef()
{
    static dbtable investmentTable(qsl("Geldanlagen"));
    if( 0 == investmentTable.Fields().size()){
        investmentTable.append(dbfield(qsl("ZSatz"), QVariant::Int).setNotNull());
        investmentTable.append(dbfield(qsl("Anfang"), QVariant::Date).setNotNull());
        investmentTable.append(dbfield(qsl("Ende"), QVariant::Date).setNotNull());
        investmentTable.append(dbfield(qsl("Typ"), QVariant::String).setNotNull());
        investmentTable.append(dbfield(qsl("Abgeschlossen"), QVariant::Bool).setNotNull());
    }
    return investmentTable;
}
