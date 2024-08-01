#pragma once

#include "Actor.h"

class Command {
public:
	virtual ~Command() {}
	virtual void Execute(class Core::Actor* actor, float deltaTime) = 0;
};

class UpCommand : public Command {
public:
	virtual ~UpCommand() {}
	void Execute(class Core::Actor* actor, float deltaTime) override;
};

class DownCommand : public Command {
public:
	virtual ~DownCommand() {}
	void Execute(class Core::Actor* actor, float deltaTime) override;
};

class ForwardCommand : public Command {
public:
	virtual ~ForwardCommand() {}
	void Execute(class Core::Actor* actor, float deltaTime) override;
};

class BackwardCommand : public Command {
public:
	virtual ~BackwardCommand() {}
	void Execute(class Core::Actor* actor, float deltaTime) override;
};

class RightCommand : public Command {
public:
	virtual ~RightCommand() {}
	void Execute(class Core::Actor* actor, float deltaTime) override;
};

class LeftCommand : public Command {
public:
	virtual ~LeftCommand() {}
	void Execute(class Core::Actor* actor, float deltaTime) override;
};
