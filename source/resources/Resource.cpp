#include "Resource.h"
#include "ResourceUser.h"
#include "../utils/Config.h"

Resource::Resource(UID uid, Resource::Type type) : uid(uid), type(type)
{

}

Resource::~Resource()
{
    NotifyUsers(0);
}

bool Resource::LoadToMemory()
{
    if (!IsLoadedToMemory())
    {
        if (!LoadToMemory_Internal())
        {
            return false;
        }
    }

    referenceCount++;
    return true;
}

bool Resource::UnloadFromMemory()
{
    if (IsLoadedToMemory())
    {
        referenceCount--;

        if (referenceCount == 0)
        {
            UnloadFromMemory_Internal();
        }
        return true;
    }

    return false;
}

void Resource::AddReference(ResourceUser* user)
{
    if (std::find(users.begin(), users.end(), user) == users.end())
    {
        users.push_back(user);
    }
}

void Resource::RemoveReference(ResourceUser* user)
{
    auto it = std::find(users.begin(), users.end(), user);
    if (it != users.end())
    {
        users.erase(it);
    }
}

void Resource::NotifyUsers(UID newUID)
{
    std::vector<ResourceUser*> usersToNotify = users;

    for (ResourceUser* user : usersToNotify)
    {
        if (newUID == 0)
            user->OnResourceLost(this->uid);
        else
            user->OnResourceChanged(this->uid, newUID);
    }
    users.clear();
}