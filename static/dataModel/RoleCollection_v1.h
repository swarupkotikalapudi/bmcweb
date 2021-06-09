#ifndef ROLECOLLECTION_V1
#define ROLECOLLECTION_V1

#include "NavigationReference_.h"
#include "Resource_v1.h"

struct RoleCollectionV1RoleCollection
{
    std::string description;
    std::string name;
    ResourceV1Resource oem;
    NavigationReference_ members;
};
#endif
