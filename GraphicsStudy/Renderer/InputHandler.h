#pragma once

#include "Command.h"
#include <memory>
#include <unordered_map>

class InputHandler {
public:
	InputHandler();
	virtual ~InputHandler() {};

	//virtual bool Initialize();
	std::unique_ptr<UpCommand> upCommand;
	std::unique_ptr<DownCommand> downCommand;
	std::unique_ptr<RightCommand> rightCommand;
	std::unique_ptr<LeftCommand> leftCommand;
	std::unique_ptr<ForwardCommand> forwardCommand;
	std::unique_ptr<BackwardCommand> backwardCommand;

	std::unordered_map<int, bool> m_keyStates;
	
public:
	void ExicuteCommand(class Camera* camera, float deltaTime);

private:
	unsigned int upKey = unsigned int('Q');
	unsigned int downKey = unsigned int('E');
	unsigned int rightKey = unsigned int('D');
	unsigned int leftKey = unsigned int('A');
	unsigned int forwardKey = unsigned int('W');
	unsigned int backwardKey = unsigned int('S');
};