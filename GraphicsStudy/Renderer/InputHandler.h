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

	std::unordered_map<int, bool> m_prevKeyStates;
	std::unordered_map<int, bool> m_currKeyStates;

public:
	// 매 프레임당 호출되어 키 입력을 처리한다
	void ExicuteCommand(Core::Actor* actor, float deltaTime, bool bIsFPSMode);
	void UpdateKeyDown(const int& wParam);
	void UpdateKeyUp(const int& wParam);

private:
	unsigned int upKey = unsigned int('Q');
	unsigned int downKey = unsigned int('E');
	unsigned int rightKey = unsigned int('D');
	unsigned int leftKey = unsigned int('A');
	unsigned int forwardKey = unsigned int('W');
	unsigned int backwardKey = unsigned int('S');



};