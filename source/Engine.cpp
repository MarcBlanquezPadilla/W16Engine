#include "Engine.h"
#include "utils/Log.h"
#include "Window.h"
#include "input.h"
#include "Render.h"
#include "Scene.h"
#include "Editor.h"
#include "Loader.h"
#include "EventSystem.h"


Engine& Engine::GetInstance() {
    static Engine instance;
    return instance;
}

Engine::Engine() {

    Timer timer = Timer();
    startTime = Timer();
    frameTime = PerfTimer();

    events = new EventSystem(true);
    window = new Window(true);
    input = new Input(true);
    render = new Render(true);
    scene = new Scene(true);
    loader = new Loader(true);
    editor = new Editor(true);
    
    AddModule(events);
    AddModule(window);
    AddModule(input);
    AddModule(render);
    AddModule(scene);
    AddModule(loader);
    AddModule(editor);
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
    if (input->GetWindowEvent(WE_QUIT) == true)
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

    window->Swap();

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