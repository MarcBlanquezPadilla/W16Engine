#pragma once
#include "../Global.h"

class ResourceUser
{
public:
    virtual ~ResourceUser() {};

    virtual void OnResourceChanged(UID resourceUID, UID newUID)
    {
        
    }

    virtual void OnResourceLost(UID resourceUID) = 0;
};