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

const char* bool_to_color(bool value) {
    return value ? "\x1B[32mtrue\033[0m" : "\x1B[31mfalse\033[0m";
}

struct KeyAction {
    int keyCode;
    std::function<void()> action;
};

KeyAction actions[] = {
    { VK_F1, []() { AimBot::Get().mSmoothAmt--; } },
    { VK_F2, []() { AimBot::Get().mSmoothAmt++; } },
    { VK_F3, []() { AimBot::Get().trigger_delay -= 5; } },
    { VK_F4, []() { AimBot::Get().trigger_delay += 5; } },
    { VK_F5, []() { aimtoggle = !aimtoggle; } },
    { VK_F6, []() { glowtoggle = !glowtoggle; } },
    { VK_F7, []() { bhoptoggle = !bhoptoggle; } },
    { VK_F8, []() { radartoggle = !radartoggle; } },
    { VK_F9, []() { flashtoggle = !flashtoggle; } },
};

DWORD WINAPI OnDllAttach(LPVOID base)
{
    Utils::OpenConsole();
    Utils::ResizeConsole(320, 310);

    while (!(GetAsyncKeyState(VK_F10) & 1))
    {
        system("CLS");
        if (!Utils::GetPid("csgo.exe"))
            Utils::Print("Waiting for CS:GO");

        while (!Utils::GetPid("csgo.exe") && !(GetAsyncKeyState(VK_F10))) Sleep(100);

        Memory::Get().Init();

        if (!Memory::Get().Browser)
            Utils::Print("Waiting for serverbrowser.dll");

       while (!Memory::Get().Browser && !(GetAsyncKeyState(VK_F10)))
       {
           Sleep(100);
           Memory::Get().Init();
       }

        shouldupdate = !shouldupdate;
        while (Utils::GetPid("csgo.exe") && !(GetAsyncKeyState(VK_F10)))
        {
            for (auto& ka : actions) {
                if (GetAsyncKeyState(ka.keyCode) & 1) {
                    Sleep(100);
                    ka.action();
                    shouldupdate = !shouldupdate;
                }
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
                printf("F5 aim         [%s]\n", bool_to_color(aimtoggle));
                printf("F6 glow        [%s]\n", bool_to_color(glowtoggle));
                printf("F7 bhop        [%s]\n", bool_to_color(bhoptoggle));
                printf("F8 radar       [%s]\n", bool_to_color(radartoggle));
                printf("F9 low flash   [%s]\n", bool_to_color(flashtoggle));
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
        if (GetAsyncKeyState(VK_F10))
        {
            FreeLibraryAndExitThread(static_cast<HMODULE>(base), 1);
            return 1;
        }
    }
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

