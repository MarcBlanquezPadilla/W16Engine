#include "Engine.h"
#include "Log.h"
#include "Window.h"
#include "input.h"
#include "Render.h"


Engine& Engine::GetInstance() {
    static Engine instance;
    return instance;
}

Engine::Engine() {

    Timer timer = Timer();
    startTime = Timer();
    frameTime = PerfTimer();

    window = new Window(true);
    input = new Input(true);
    render = new Render(true);
    
    AddModule(window);
    AddModule(input);
    AddModule(render);
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
        ret = false;

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

    return ret;
}

bool Engine::CleanUp() {

    bool ret = true;

    for (Module* module : moduleList) {

        ret = module->CleanUp();
        if (!ret) {

            continue;
        }
    }

    return ret;
}

void Engine::AddModule(Module* module) {
    
    moduleList.push_back(module);
}