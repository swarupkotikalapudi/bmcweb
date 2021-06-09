#ifndef METRICDEFINITIONCOLLECTION_V1
#define METRICDEFINITIONCOLLECTION_V1

#include "NavigationReference_.h"
#include "Resource_v1.h"

struct MetricDefinitionCollectionV1MetricDefinitionCollection
{
    std::string description;
    std::string name;
    ResourceV1Resource oem;
    NavigationReference_ members;
};
#endif
