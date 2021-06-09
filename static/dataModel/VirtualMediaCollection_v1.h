#ifndef VIRTUALMEDIACOLLECTION_V1
#define VIRTUALMEDIACOLLECTION_V1

#include "NavigationReference__.h"
#include "Resource_v1.h"

struct VirtualMediaCollection_v1_VirtualMediaCollection
{
    std::string description;
    std::string name;
    Resource_v1_Resource oem;
    NavigationReference__ members;
};
#endif
