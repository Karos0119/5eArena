#include "includes.hpp"

Vector GetBonePos(DWORD Player, int Bone)
{
    Vector EnemyBones;
    DWORD Bonebase = Memory::Get().Read<DWORD>(Player + hazedumper::netvars::m_dwBoneMatrix);
    EnemyBones.x = Memory::Get().Read<float>(Bonebase + 0x30 * Bone + 0x0C);
    EnemyBones.y = Memory::Get().Read<float>(Bonebase + 0x30 * Bone + 0x1C);
    EnemyBones.z = Memory::Get().Read<float>(Bonebase + 0x30 * Bone + 0x2C);
    return EnemyBones;
}

int WorldToScreenDist(DWORD Player, RECT game_window, int radius)
{
    int x = (int)GetBonePos(Player, 8).x - (game_window.right - game_window.left) / 2;
    int y = (int)GetBonePos(Player, 8).y - (game_window.bottom - game_window.top) / 2;
    return radius * radius - (x * x + y * y);
}

bool WorldToScreen(Vector pos, Vector& screen, view q, int windowWidth, int windowHeight) // converts 3D coords to 2D coords
{
    //Matrix-vector Product, multiplying world(eye) coordinates by projection matrix = clipCoords

    VectorAligned clipCoords;
    clipCoords.x = pos.x * q.v[0] + pos.y * q.v[1] + pos.z * q.v[2] + q.v[3];
    clipCoords.y = pos.x * q.v[4] + pos.y * q.v[5] + pos.z * q.v[6] + q.v[7];
    clipCoords.z = pos.x * q.v[8] + pos.y * q.v[9] + pos.z * q.v[10] + q.v[11];
    clipCoords.w = pos.x * q.v[12] + pos.y * q.v[13] + pos.z * q.v[14] + q.v[15];

    if (clipCoords.w < 0.1f) // if the camera is behind our player don't draw, i think?
        return false;


    Vector NDC;
    // Normalize our coords
    NDC.x = clipCoords.x / clipCoords.w;
    NDC.y = clipCoords.y / clipCoords.w;
    NDC.z = clipCoords.z / clipCoords.w;

    // Convert to window coords (x,y)
    screen.x = (windowWidth / 2 * NDC.x) + (NDC.x + windowWidth / 2);
    screen.y = -(windowHeight / 2 * NDC.y) + (NDC.y + windowHeight / 2);
    return true;
}

DWORD WINAPI OnDllAttach(LPVOID base)
{
    Utils::OpenConsole();

    Utils::ResizeConsole(350, 300);
    while (!(GetAsyncKeyState(VK_F10) & 1))
    {
        system("CLS");
        if (!Utils::GetPid("csgo.exe"))
            Utils::Print("Waiting for CS:GO");

        while (!Utils::GetPid("csgo.exe")) 
            Sleep(100);

        Memory::Get().Init();

        Utils::Print("Waiting for serverbrowser.dll");
        while (!Memory::Get().Browser)
        {
           Sleep(1000);
           Memory::Get().Init();
        }
        shouldupdate = !shouldupdate;
        while (Utils::GetPid("csgo.exe"))
        {
            if (GetAsyncKeyState(VK_F1) & 1)
            {
                Sleep(100);
                AimBot::Get().mSmoothAmt--;
                shouldupdate = !shouldupdate;
            }
            else if (GetAsyncKeyState(VK_F2) & 1)
            {
                Sleep(100);
                AimBot::Get().mSmoothAmt++;
                shouldupdate = !shouldupdate;
            }
            else if (GetAsyncKeyState(VK_F3) & 1)
            {
                Sleep(100);
                AimBot::Get().trigger_delay -= 5;
                shouldupdate = !shouldupdate;
            }
            else if (GetAsyncKeyState(VK_F4) & 1)
            {
                Sleep(100);
                AimBot::Get().trigger_delay += 5;
                shouldupdate = !shouldupdate;
            }
            else if (GetAsyncKeyState(VK_F5) & 1)
            {
                Sleep(100);
                aimtoggle = !aimtoggle;
                shouldupdate = !shouldupdate;
            }
            else if (GetAsyncKeyState(VK_F6) & 1)
            {
                Sleep(100);
                glowtoggle = !glowtoggle;
                shouldupdate = !shouldupdate;
            }
            else if (GetAsyncKeyState(VK_F7) & 1)
            {
                Sleep(100);
                bhoptoggle = !bhoptoggle;
                shouldupdate = !shouldupdate;
            }
            else if (GetAsyncKeyState(VK_F8) & 1)
            {
                Sleep(100);
                radartoggle = !radartoggle;
                shouldupdate = !shouldupdate;
            }
            else if (GetAsyncKeyState(VK_F9) & 1)
            {
                Sleep(100);
                flashtoggle = !flashtoggle;
                shouldupdate = !shouldupdate;
            }

            if (shouldupdate)
            {
                Sleep(100);
                system("CLS");
                printf("M5 trigger\n");
                printf("F1 decrease aim smooth\n");
                printf("smooth: %i\n", AimBot::Get().mSmoothAmt);
                printf("F2 increase aim smooth\n");
                printf("\n");
                printf("F3 decrease trigger delay\n");
                printf("delay: %i\n", AimBot::Get().trigger_delay);
                printf("F4 increase trigger delay\n");
                printf("F5 aim        [%s]\n", aimtoggle ? "\x1B[32mtrue\033[0m" : "\x1B[31mfalse\033[0m");
                printf("F6 glow       [%s]\n", glowtoggle ? "\x1B[32mtrue\033[0m" : "\x1B[31mfalse\033[0m");
                printf("F7 bhop       [%s]\n", bhoptoggle ? "\x1B[32mtrue\033[0m" : "\x1B[31mfalse\033[0m");
                printf("F8 radar      [%s]\n", radartoggle ? "\x1B[32mtrue\033[0m" : "\x1B[31mfalse\033[0m");
                printf("F9 low flash  [%s]\n", flashtoggle ? "\x1B[32mtrue\033[0m" : "\x1B[31mfalse\033[0m");
                printf("F10 exit\n");
                shouldupdate = !shouldupdate;
            }


            Memory::Get().Local = Memory::Get().Read<DWORD>(Memory::Get().Client + hazedumper::signatures::dwLocalPlayer);

            if (Entity::Get().IsAlive(Memory::Get().Local))
            {
                // trigger
                if (GetAsyncKeyState(VK_XBUTTON2))
                    AimBot::Get().Trigger();

                // bhop
                if (bhoptoggle)
                    Misc::Get().Bhop();

                // aimbot
                if (GetAsyncKeyState(VK_LBUTTON) && aimtoggle)
                    AimBot::Get().Aim();

                // fakelag
                //Misc::Get().FakelagChoke(4);
            }

            // antiflash
            if (flashtoggle)
                Memory::Get().Write<float>(Memory::Get().Local + hazedumper::netvars::m_flFlashMaxAlpha, 100.f);
            else
                Memory::Get().Write<float>(Memory::Get().Local + hazedumper::netvars::m_flFlashMaxAlpha, 255.f);

            // loop players
            for (int i = 1; i < 32; i++)
            {
                DWORD Player = Memory::Get().Read<DWORD>(Memory::Get().Client + hazedumper::signatures::dwEntityList + (i * 0x10));
                view Matrix = Memory::Get().Read<view>(Memory::Get().Client + hazedumper::signatures::dwViewMatrix);

                Vector Origin, HeadPos;
                Vector PlayerOrigin = Memory::Get().Read<Vector>(Player + hazedumper::netvars::m_vecOrigin);

                AimBot::Get().Init(Player);

                // force radar
                if (Entity::Get().IsEnemy(Player, Memory::Get().Local) && Entity::Get().IsAlive(Player) && radartoggle)
                    Memory::Get().Write<bool>(Player + hazedumper::netvars::m_bSpotted, true);

                // "chams" XDD
                //Visuals::Get().Chams(Player, Entity::Get().IsEnemy(Player, Memory::Get().Local));

                // glow
                if (glowtoggle)
                    Visuals::Get().Glow(Player, Entity::Get().IsEnemy(Player, Memory::Get().Local));
            }
        }
    }
    FreeLibraryAndExitThread(static_cast<HMODULE>(base), 1);
    return 1;
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        DisableThreadLibraryCalls(hModule);

        auto Handle = CreateThread(nullptr, 0, OnDllAttach, hModule, 0, nullptr);
        if (Handle)
            CloseHandle(Handle);

        break;
    }
    case DLL_PROCESS_DETACH:
        Utils::CloseConsole();
        break;
    }
    return TRUE;
}

