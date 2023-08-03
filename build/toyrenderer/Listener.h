#pragma once
class Listener
{
public:
	Listener();
	~Listener();
	void virtual update() = 0;
};