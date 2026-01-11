
#include "Engine.h"
#include <stdlib.h>
#include "utils/Log.h"

//#ifdef _DEBUG // Solo para modo Debug
//
//#pragma comment(lib, "C:/Programming Tools/Visual Leak Detector/lib/Win64/vld.lib")
//#include "C:/Programming Tools/Visual Leak Detector/include/vld.h"
//
//#endif

int main()
{
    EngineState state = CREATE;
	int result = EXIT_FAILURE;

    while (state != EXIT)
    {
		switch (state)
		{
			
		case EngineState::CREATE:
			
			state = EngineState::AWAKE;

			break;

			
		case EngineState::AWAKE:
			
			if (Engine::GetInstance().Awake() == true)
			{
				LOG(LogType::LOG_INFO, "Awaked without errors.");
				state = EngineState::START;
			}
			else
			{
				LOG(LogType::LOG_ERROR, "Awake failed.");
				state = EngineState::FAIL;
			}

			break;

			
		case EngineState::START:
			
			if (Engine::GetInstance().Start() == true)
			{
				LOG(LogType::LOG_INFO, "Started without errors.");
				state = EngineState::LOOP;
			}
			else
			{
				LOG(LogType::LOG_ERROR, "Start failed.");
				state = EngineState::FAIL;
			}
			break;

			
		case EngineState::LOOP:

			if (Engine::GetInstance().PreUpdate() == false)
			{
				state = EngineState::FAIL;
				LOG(LogType::LOG_ERROR, "Preupdate failed.");
			}
				

			if (Engine::GetInstance().Update() == false)
			{
				state = EngineState::CLEAN;
			}
				

			if (Engine::GetInstance().PostUpdate() == false)
			{
				state = EngineState::FAIL;
				LOG(LogType::LOG_ERROR, "ERROR: Postupdate failed.");
			}

			break;

			
		case EngineState::CLEAN:
			
			if (Engine::GetInstance().CleanUp() == true)
			{
				result = EXIT_SUCCESS;
				state = EngineState::EXIT;
			}
			else
				state = EngineState::FAIL;

			break;

			
		case EngineState::FAIL:
			LOG(LogType::LOG_ERROR, "Exiting with errors.");
			result = EXIT_FAILURE;
			state = EngineState::EXIT;
			break;
		}
    }

    return 0;
}