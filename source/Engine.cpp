#include "Engine.h"
#include "utils/Log.h"
#include "Window.h"
#include "input.h"
#include "Render.h"
#include "Camera.h"
#include "Scene.h"
#include "Editor.h"
#include "Loader.h"


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
    camera = new Camera(true);
    scene = new Scene(true);
    editor = new Editor(true);
    loader = new Loader(true);
    
    AddModule(window);
    AddModule(input);
    AddModule(render);
    AddModule(camera);
    AddModule(scene);
    AddModule(editor);
    AddModule(loader);
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

    return ret;
}

bool Engine::CleanUp() {

    bool ret = true;

    for (int i = 0; i < moduleList.size(); i++) {

        ret = moduleList[i]->CleanUp();
        if (!ret) {

            continue;
        }
        delete moduleList[i];
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