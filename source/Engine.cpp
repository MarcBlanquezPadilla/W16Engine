#include "Engine.h"
#include "utils/Log.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleRender.h"
#include "ModuleScene.h"
#include "ModuleEditor.h"
#include "ModulePhysics.h"
#include "ModuleLoader.h"
#include "ModuleEvents.h"
#include "ModuleResources.h"
#include "ModuleTime.h"


Engine& Engine::GetInstance() {
    static Engine instance;
    return instance;
}

Engine::Engine() {

    moduleTime = new ModuleTime(true);
    moduleEvents = new ModuleEvents(true);
    moduleWindow = new ModuleWindow(true);
    moduleInput = new ModuleInput(true);
    modulePhysics = new ModulePhysics(true);
    moduleRender = new ModuleRender(true);
    moduleResources = new ModuleResources(true);
    moduleScene = new ModuleScene(true);
    moduleLoader = new ModuleLoader(true);
    moduleEditor = new ModuleEditor(true);
    
    //CORE MODULES
    AddModule(moduleTime);
    AddModule(moduleEvents);
    AddModule(moduleWindow);
    AddModule(moduleInput);
    AddModule(modulePhysics);
    AddModule(moduleRender);

    //DATA MODULES
    AddModule(moduleResources);
    AddModule(moduleLoader);

    //LOGIC MODULES
    AddModule(moduleScene);

    //TOOLS MODULES
    AddModule(moduleEditor);
}

bool Engine::Awake() {
    
    bool ret = true;

    for (Module* module : moduleList) {

        ret = module->Awake();
        if (!ret) {

            LOG(LogType::LOG_ERROR, "%s failed in Awake!", module->name.c_str());
            return false;
        }
    }

    quit = false;
    
    return ret;
}

bool Engine::Start() {
    
    bool ret = true;

    for (Module* module : moduleList) {

        ret = module->Start();
        if (!ret) {

            LOG(LogType::LOG_ERROR, "%s failed in Start!", module->name.c_str());
            return false;
        }
    }

    return ret;
}

bool Engine::PreUpdate() {

    bool ret = true;

    for (Module* module : moduleList) {

        ret = module->PreUpdate();
        if (!ret) {

            LOG(LogType::LOG_ERROR, "%s failed in Pre Update!", module->name.c_str());
            return false;
        }
    }

    return ret;
}

bool Engine::Update() {
    
    bool ret = true;

    for (Module* module : moduleList) {

        ret = module->Update();
        if (!ret) {

            LOG(LogType::LOG_ERROR, "%s failed in Update!", module->name.c_str());
            return false;
        }
    }

    //QUIT CONDITION
    if (moduleInput->GetWindowEvent(WE_QUIT) == true)
        quit = true;

    if (quit) ret = false;

    return ret;
}

bool Engine::FixedUpdate() {

    bool ret = true;

    for (Module* module : moduleList) {

        ret = module->FixedUpdate();
        if (!ret) {

            LOG(LogType::LOG_ERROR, "%s failed in Fixed Update!", module->name.c_str());
            return false;
        }
    }

    return ret;
}

bool Engine::PostUpdate() {

    bool ret = true;

    for (Module* module : moduleList) {

        ret = module->PostUpdate();
        if (!ret) {

            LOG(LogType::LOG_ERROR, "%s failed in Post Update!", module->name.c_str());
            return false;
        }
    }

    moduleWindow->Swap();

    return ret;
}

bool Engine::CleanUp() {

    bool ret = true;

    for (auto it = moduleList.rbegin(); it != moduleList.rend(); ++it)
    {
        Module* module = *it;

        if (module->CleanUp() == false)
        {
            LOG(LogType::LOG_ERROR, "%s failed Cleaning Up!", module->name.c_str());
            ret = false;
        }

        delete module;
    }

    moduleList.clear();

    return ret;
}

void Engine::AddModule(Module* module) {
    
    moduleList.push_back(module);
}

void Engine::QuitApplication()
{
    quit = true;
}