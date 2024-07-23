#pragma once
#include "Command.h"
#include <memory>

class InputHandler {

	std::shared_ptr<Command> upCommand;

	void ExicuteCommand(class Actor* actor);
};