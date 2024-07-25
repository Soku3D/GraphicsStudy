#include "Command.h"
#include "Camera.h"

void UpCommand::Execute(Camera* camera, float deltaTime)
{
	camera->MoveUp(deltaTime);
}

void DownCommand::Execute(Camera* camera, float deltaTime)
{
	camera->MoveDown(deltaTime);
}

void RightCommand::Execute(Camera* camera, float deltaTime)
{
	camera->MoveRight(deltaTime);
}

void LeftCommand::Execute(Camera* camera, float deltaTime)
{
	camera->MoveLeft(deltaTime);
}

void ForwardCommand::Execute(Camera* camera, float deltaTime)
{
	camera->MoveForward(deltaTime);
}

void BackwardCommand::Execute(Camera* camera, float deltaTime)
{
	camera->MoveBackward(deltaTime);
}
