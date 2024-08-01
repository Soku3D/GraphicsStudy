#include "Command.h"



void UpCommand::Execute(Core::Actor* actor, float deltaTime)
{
	actor->MoveUp(deltaTime);
}

void DownCommand::Execute(Core::Actor* actor, float deltaTime)
{
	actor->MoveDown(deltaTime);
}

void RightCommand::Execute(Core::Actor* actor, float deltaTime)
{
	actor->MoveRight(deltaTime);
}

void LeftCommand::Execute(Core::Actor* actor, float deltaTime)
{
	actor->MoveLeft(deltaTime);
}

void ForwardCommand::Execute(Core::Actor* actor, float deltaTime)
{
	actor->MoveForward(deltaTime);
}

void BackwardCommand::Execute(Core::Actor* actor, float deltaTime)
{
	actor->MoveBackward(deltaTime);
}
