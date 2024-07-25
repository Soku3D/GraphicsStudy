#pragma once

class Command {
public:
	virtual ~Command() {}
	virtual void Execute(class Camera* camera, float deltaTime) = 0;
};

class UpCommand : public Command {
public:
	virtual ~UpCommand() {}
	void Execute(class Camera* camera, float deltaTime) override;
};

class DownCommand : public Command {
public:
	virtual ~DownCommand() {}
	void Execute(class Camera* camera, float deltaTime) override;
};

class ForwardCommand : public Command {
public:
	virtual ~ForwardCommand() {}
	void Execute(class Camera* camera, float deltaTime) override;
};

class BackwardCommand : public Command {
public:
	virtual ~BackwardCommand() {}
	void Execute(class Camera* camera, float deltaTime) override;
};

class RightCommand : public Command {
public:
	virtual ~RightCommand() {}
	void Execute(class Camera* camera, float deltaTime) override;
};

class LeftCommand : public Command {
public:
	virtual ~LeftCommand() {}
	void Execute(class Camera* camera, float deltaTime) override;
};
