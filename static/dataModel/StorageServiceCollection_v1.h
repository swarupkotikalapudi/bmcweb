#ifndef STORAGESERVICECOLLECTION_V1
#define STORAGESERVICECOLLECTION_V1

#include "NavigationReference__.h"
#include "Resource_v1.h"

struct StorageServiceCollection_v1_StorageServiceCollection
{
    std::string description;
    std::string name;
    Resource_v1_Resource oem;
    NavigationReference__ members;
};
#endif
