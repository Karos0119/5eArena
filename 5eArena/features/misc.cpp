#include "misc.hpp"

void Misc::Bhop() 
{
    BYTE flag = Memory::Get().Read<BYTE>(Memory::Get().Local + hazedumper::netvars::m_fFlags);
    if (GetAsyncKeyState(VK_SPACE) && flag & (1 << 0))
    {
        Memory::Get().Write<DWORD>(Memory::Get().Client + hazedumper::signatures::dwForceJump, 5);
    }
    else if (GetAsyncKeyState(VK_SPACE) && !(flag & (1 << 0)))
    {
        Memory::Get().Write<DWORD>(Memory::Get().Client + hazedumper::signatures::dwForceJump, 4);
        Memory::Get().Write<DWORD>(Memory::Get().Client + hazedumper::signatures::dwForceJump, 5);
        Memory::Get().Write<DWORD>(Memory::Get().Client + hazedumper::signatures::dwForceJump, 4);
    }
    else
    {
        Memory::Get().Write<DWORD>(Memory::Get().Client + hazedumper::signatures::dwForceJump, 4);
    }
}

void Misc::Fakelag(int ticks)
{
	Memory::Get().Write(Memory::Get().Engine + hazedumper::signatures::dwbSendPackets, false);

	int curr_choke = 999;
	do
	{
		Memory::Get().ReadDst(Memory::Get().ClientState + hazedumper::signatures::clientstate_choked_commands, curr_choke);
	} while (curr_choke < ticks);

	Memory::Get().Write(Memory::Get().Engine + hazedumper::signatures::dwbSendPackets, true);
}