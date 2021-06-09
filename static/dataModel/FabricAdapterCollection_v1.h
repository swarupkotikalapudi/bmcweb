#ifndef FABRICADAPTERCOLLECTION_V1
#define FABRICADAPTERCOLLECTION_V1

#include "NavigationReference_.h"
#include "Resource_v1.h"

struct FabricAdapterCollectionV1FabricAdapterCollection
{
    std::string description;
    std::string name;
    ResourceV1Resource oem;
    NavigationReference_ members;
};
#endif
