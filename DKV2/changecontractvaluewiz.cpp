
#include "changecontractvaluewiz.h"

bool changeContractValueWiz(qlonglong /*contractId*/)
{
    wizChangeContractWiz w;

    w.exec();
    return true;
}
