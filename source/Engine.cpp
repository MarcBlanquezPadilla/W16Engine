#include "Engine.h"
#include "Log.h"

Engine& Engine::GetInstance() {
    static Engine instance;
    return instance;
}

Engine::Engine() {

    Timer timer = Timer();
    startTime = Timer();
    frameTime = PerfTimer();
}

bool Engine::Awake() {

    startTime.Start();
    frameTime.Start();

    bool ret = true;
    return ret;
}

bool Engine::Start() {
    
    bool ret = true;
    return ret;
}

bool Engine::Update() {
    
    //CALCULATE FRAMETIME
    dt = frameTime.ReadMs();
    frameTime.Start();


    bool ret = true;
    return ret;
}

bool Engine::CleanUp() {

    bool ret = true;
    return ret;
}