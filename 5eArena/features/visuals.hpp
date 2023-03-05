#pragma once
#include "../includes.hpp"

class Visuals : public Singleton <Visuals>
{
public:
	void Glow(DWORD Player, bool IsEnemy);
};

