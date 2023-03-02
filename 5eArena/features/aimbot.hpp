#pragma once
#include "../includes.hpp"

class AimBot : public Singleton <AimBot>
{
public:
	void Trigger();
	void Aim();
	void Init(DWORD Player);

	DWORD Target;

	int mSmoothAmt = 10;
	int trigger_delay = 100;
};
