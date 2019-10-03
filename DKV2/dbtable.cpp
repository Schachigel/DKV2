#include "dbtable.h"
#include "dbfield.h"

dbfield dbtable::operator[](QString s)
{
    for (auto f : Fields)
    {
        if( f.name == s)
            return f;
    }
    return dbfield();
}
