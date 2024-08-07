#include "InputHandler.h"
#include <iostream>

using namespace Core;

InputHandler::InputHandler()
{
	upCommand = std::make_unique<UpCommand>();
	downCommand = std::make_unique<DownCommand>();
	rightCommand = std::make_unique<RightCommand>();
	leftCommand = std::make_unique<LeftCommand>();
	forwardCommand = std::make_unique<ForwardCommand>();
	backwardCommand = std::make_unique<BackwardCommand>();
}

void InputHandler::ExicuteCommand(Core::Actor* actor, float deltaTime, bool bIsFPSMode)
{
	if (bIsFPSMode) {
		actor->RotateDirection();

		if (m_currKeyStates[upKey]) {
			upCommand->Execute(actor, deltaTime);
		}
		if (m_currKeyStates[downKey]) {
			downCommand->Execute(actor, deltaTime);
		}
		if (m_currKeyStates[forwardKey]) {
			forwardCommand->Execute(actor, deltaTime);
		}
		if (m_currKeyStates[backwardKey]) {
			backwardCommand->Execute(actor, deltaTime);
		}
		if (m_currKeyStates[rightKey]) {
			rightCommand->Execute(actor, deltaTime);
		}
		if (m_currKeyStates[leftKey]) {
			leftCommand->Execute(actor, deltaTime);
		}
	}
}

void InputHandler::UpdateKeyDown(const int& wParam)
{
	m_currKeyStates[wParam] = true;
}

void InputHandler::UpdateKeyUp(const int& wParam)
{
	m_currKeyStates[wParam] = true;
}

