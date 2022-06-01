#pragma once

#include "Core/System.hpp"


struct Event;


struct CameraControlSystem : public System
{
public:
	void Init();

	void Update(float dt);

private:
	std::bitset<8> mButtons;

	void InputListener(Event& event);
};
