#ifndef POWERDOMAINCOLLECTION_V1
#define POWERDOMAINCOLLECTION_V1

#include "NavigationReference_.h"
#include "Resource_v1.h"

struct PowerDomainCollectionV1PowerDomainCollection
{
    std::string description;
    std::string name;
    ResourceV1Resource oem;
    NavigationReference_ members;
};
#endif
