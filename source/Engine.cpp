#include "Engine.h"
#include "utils/Log.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleRender.h"
#include "ModuleScene.h"
#include "ModuleEditor.h"
#include "ModuleLoader.h"
#include "ModuleEvents.h"


Engine& Engine::GetInstance() {
    static Engine instance;
    return instance;
}

Engine::Engine() {

    Timer timer = Timer();
    startTime = Timer();
    frameTime = PerfTimer();

    moduleEvents = new ModuleEvents(true);
    moduleWindow = new ModuleWindow(true);
    moduleInput = new ModuleInput(true);
    moduleRender = new ModuleRender(true);
    moduleScene = new ModuleScene(true);
    moduleLoader = new ModuleLoader(true);
    moduleEditor = new ModuleEditor(true);
    
    AddModule(moduleEvents);
    AddModule(moduleWindow);
    AddModule(moduleInput);
    AddModule(moduleRender);
    AddModule(moduleScene);
    AddModule(moduleLoader);
    AddModule(moduleEditor);
}

bool Engine::Awake() {
    
    bool ret = true;

    for (Module* module : moduleList) {

        ret = module->Awake();
        if (!ret) {

            continue;
        }
    }

    startTime.Start();
    frameTime.Start();
    quit = false;
    
    return ret;
}

bool Engine::Start() {
    
    bool ret = true;

    for (Module* module : moduleList) {

        ret = module->Start();
        if (!ret) {

            continue;
        }
    }

    return ret;
}

bool Engine::PreUpdate() {

    bool ret = true;

    for (Module* module : moduleList) {

        ret = module->PreUpdate();
        if (!ret) {

            continue;
        }
    }

    return ret;
}

bool Engine::Update() {
    
    bool ret = true;

    for (Module* module : moduleList) {

        ret = module->Update(dt);
        if (!ret) {

            continue;
        }
    }

    //QUIT CONDITION
    if (moduleInput->GetWindowEvent(WE_QUIT) == true)
        quit = true;

    if (quit) ret = false;

    //UPDATE FRAMERATE
    dt = frameTime.ReadMs();
    frameTime.Start();

    return ret;
}

bool Engine::PostUpdate() {

    bool ret = true;

    for (Module* module : moduleList) {

        ret = module->PostUpdate();
        if (!ret) {

            continue;
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
            LOG("Error cleaning up module %s", module->name.c_str());
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

float Engine::GetDtMs()
{
    return dt;
}

float Engine::GetDtS()
{
    return dt/1000;
}


void Engine::QuitApplication()
{
    quit = true;
}