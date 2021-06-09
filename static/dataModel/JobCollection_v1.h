#ifndef JOBCOLLECTION_V1
#define JOBCOLLECTION_V1

#include "NavigationReference_.h"
#include "Resource_v1.h"

struct JobCollectionV1JobCollection
{
    std::string description;
    std::string name;
    ResourceV1Resource oem;
    NavigationReference_ members;
};
#endif
