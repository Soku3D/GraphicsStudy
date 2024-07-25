#include "InputHandler.h"
#include <iostream>

InputHandler::InputHandler()
{
	upCommand = std::make_unique<UpCommand>();
	downCommand = std::make_unique<DownCommand>();
	rightCommand = std::make_unique<RightCommand>();
	leftCommand = std::make_unique<LeftCommand>();
	forwardCommand = std::make_unique<ForwardCommand>();
	backwardCommand = std::make_unique<BackwardCommand>();
}

void InputHandler::ExicuteCommand(Camera* camera, float deltaTime)
{
	if (m_keyStates[upKey]) {
		upCommand->Execute(camera, deltaTime);
	}
	if (m_keyStates[downKey]) {
		downCommand->Execute(camera, deltaTime);
	}
	if (m_keyStates[forwardKey]) {
		forwardCommand->Execute(camera, deltaTime);
	}
	if (m_keyStates[backwardKey]) {
		backwardCommand->Execute(camera, deltaTime);
	}
	if (m_keyStates[rightKey]) {
		rightCommand->Execute(camera, deltaTime);
	}
	if (m_keyStates[leftKey]) {
		leftCommand->Execute(camera, deltaTime);
	}
}


