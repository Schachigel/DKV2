
#include "dbfield.h"
#include "dbtable.h"

dbfieldinfo dbfield::getInfo()
{
    return dbfieldinfo{table->Name, name, vType };
}

