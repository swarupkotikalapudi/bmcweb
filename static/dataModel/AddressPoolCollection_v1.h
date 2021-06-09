#ifndef ADDRESSPOOLCOLLECTION_V1
#define ADDRESSPOOLCOLLECTION_V1

#include "NavigationReference__.h"
#include "Resource_v1.h"

struct AddressPoolCollection_v1_AddressPoolCollection
{
    std::string description;
    std::string name;
    Resource_v1_Resource oem;
    NavigationReference__ members;
};
#endif
