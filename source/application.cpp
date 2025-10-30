#include "Engine.h"
#include <stdlib.h>
#include "Log.h"

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
				state = EngineState::START;
			else
			{
				LOG("ERROR: Awake failed");
				state = EngineState::FAIL;
			}

			break;

			
		case EngineState::START:
			
			if (Engine::GetInstance().Start() == true)
			{
				state = EngineState::LOOP;
				LOG("\n");
				LOG("------------------------------");
				LOG("--- STARTED WITHOUT ERRORS ---");
				LOG("------------------------------\n\n");
			}
			else
			{
				state = EngineState::FAIL;
				LOG("ERROR: Start failed");
			}
			break;

			
		case EngineState::LOOP:

			if (Engine::GetInstance().PreUpdate() == false)
			{
				state = EngineState::FAIL;
				LOG("ERROR: Preupdate failed");
			}
				

			if (Engine::GetInstance().Update() == false)
				state = EngineState::CLEAN;

			if (Engine::GetInstance().PostUpdate() == false)
			{
				state = EngineState::FAIL;
				LOG("ERROR: Postupdate failed");
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
			LOG("Exiting with errors");
			result = EXIT_FAILURE;
			state = EngineState::EXIT;
			break;
		}
    }

    return 0;
}