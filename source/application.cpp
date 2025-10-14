#include "Engine.h"
#include <stdlib.h>
#include "Log.h"

int main()
{
	LOG("Engine starting ...");

    EngineState state = CREATE;
	int result = EXIT_FAILURE;

    while (state != EXIT)
    {
		switch (state)
		{
			
		case EngineState::CREATE:
			LOG("CREATION PHASE ===============================");
			state = EngineState::AWAKE;

			break;

			
		case EngineState::AWAKE:
			LOG("AWAKE PHASE ===============================");
			if (Engine::GetInstance().Awake() == true)
				state = EngineState::START;
			else
			{
				LOG("ERROR: Awake failed");
				state = EngineState::FAIL;
			}

			break;

			
		case EngineState::START:
			LOG("START PHASE ===============================");
			if (Engine::GetInstance().Start() == true)
			{
				state = EngineState::LOOP;
				LOG("UPDATE PHASE ===============================");
			}
			else
			{
				state = EngineState::FAIL;
				LOG("ERROR: Start failed");
			}
			break;

			
		case EngineState::LOOP:
			if (Engine::GetInstance().Update() == false)
				state = EngineState::CLEAN;
			break;

			
		case EngineState::CLEAN:
			LOG("CLEANUP PHASE ===============================");
			if (Engine::GetInstance().CleanUp() == true)
			{
				result = EXIT_SUCCESS;
				state = EngineState::EXIT;
			}
			else
				state = EngineState::FAIL;

			break;

			
		case EngineState::FAIL:
			LOG("Exiting with errors");
			result = EXIT_FAILURE;
			state = EngineState::EXIT;
			break;
		}
    }

    return 0;
}