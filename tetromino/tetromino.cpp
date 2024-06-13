// tetromino.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//


#include <string>
#include <Windows.h>
#include <set>
#include <format>
#include <Xinput.h>
#include <mmsystem.h>

#pragma comment(lib, "Xinput.lib")
#pragma comment(lib, "winmm.lib")

using namespace std;

wstring tetromino[7];
int nFieldWidth = 12;
int nFieldHeight = 18;
int nScreenWidth = 80;
int nScreenHeight = 30;

unsigned char* pField = nullptr;

const DWORD DOWN_PRESSED = 1 << 0;

const DWORD LEFT_PRESSED = 1 << 1;

const DWORD RIGHT_PRESSED = 1 << 2;

const DWORD ROTATE_PRESSED = 1 << 3;

DWORD GetKeyState()
{

    DWORD result = 0;

    XINPUT_STATE state;
    ZeroMemory(&state, sizeof(XINPUT_STATE));

    // 使用第一个控制器（控制器索引为0）
    DWORD dwResult = XInputGetState(0, &state);
    if (dwResult == ERROR_SUCCESS) {
        // 控制器已连接
        if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) {
            result |= LEFT_PRESSED;
        }
        if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) {
            result |= RIGHT_PRESSED;
        }
        if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) {
            result |= DOWN_PRESSED;
        }
        if (state.Gamepad.wButtons & (XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_A)) {
            result |= ROTATE_PRESSED;
        }
    }
    return result;
}

int Rotate(int px, int py, int r)
{
    switch (r % 4)
    {
    case 0: return py * 4 + px;
    case 1: return 12 + py - (px * 4);
    case 2: return 15 - (py * 4) - px;
    case 3: return 3 - py + (px * 4);
    }
    return 0;
}

struct Timer {
    LARGE_INTEGER frequency;
    LARGE_INTEGER start;
    const double desiredInterval = 16.67; // 60 FPS, 1000ms / 60 = 16.67ms per frame
    int frameCount = 0;
    set<int> bombs, cleans;

    Timer()
    {
        // 获取计时器的频率
        QueryPerformanceFrequency(&frequency);

        // 获取初始时间
        QueryPerformanceCounter(&start);
    }

    int GetFrameCount()
    {
        return frameCount;
    }

    bool ForTimeSlice(int n)
    {
        return frameCount % n == 0;
    }

    void BombTick(int n)
    {
        bombs.insert(frameCount + n);
    }

    bool Bombing()
    {
        bool ret = bombs.contains(frameCount);
        if (!ret) return false;
        bombs.erase(frameCount);
        return true;
    }

    void CleanTick(int n)
    {
        cleans.insert(frameCount + n);
    }

    bool Cleaning()
    {
        bool ret = cleans.contains(frameCount);
        if (!ret) return false;
        cleans.erase(frameCount);
        return true;
    }

    void Wait() {
        frameCount += 1;
        LARGE_INTEGER end;
        // 获取当前时间
        QueryPerformanceCounter(&end);

        // 计算已过去的时间（毫秒）
        double elapsedTime = static_cast<double>(end.QuadPart - start.QuadPart) * 1000.0 / frequency.QuadPart;

        // 判断是否需要等待
        if (elapsedTime < desiredInterval) {
            DWORD sleepTime = static_cast<DWORD>(desiredInterval - elapsedTime);
            Sleep(sleepTime);
        }

        // 更新起始时间
        QueryPerformanceCounter(&start);
    }
};

Timer timer;

bool DoesPieceFit(int nTetromino, int nRotation, int nPosX, int nPosY)
{
    for (int px = 0; px < 4; px += 1)
    {
        for (int py = 0; py < 4; py += 1)
        {
            int pi = Rotate(px, py, nRotation);
            int fi = (nPosY + py) * nFieldWidth + (nPosX + px);
            if (nPosX + px >= 0 && nPosX + px < nFieldWidth)
            {
                if (nPosY + py >= 0 && nPosY + py < nFieldHeight)
                {
                    if (tetromino[nTetromino][pi] == L'X' && pField[fi]) return false;
                }
            }
        }
    }
    return true;
}

void PlayBackgroundMusic()
{
    PlaySound(L"time_for_adventure.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
}

void ShowConsoleCursor(bool showFlag)
{
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_CURSOR_INFO     cursorInfo;

    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = showFlag; // set the cursor visibility
    SetConsoleCursorInfo(out, &cursorInfo);
}

int main()
{
    PlayBackgroundMusic();

    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");
    tetromino[0].append(L"..X.");

    tetromino[1].append(L".X..");
    tetromino[1].append(L".XX.");
    tetromino[1].append(L".X..");
    tetromino[1].append(L"....");

    tetromino[2].append(L".X..");
    tetromino[2].append(L".XX.");
    tetromino[2].append(L"..X.");
    tetromino[2].append(L"....");

    tetromino[3].append(L"....");
    tetromino[3].append(L".XX.");
    tetromino[3].append(L".XX.");
    tetromino[3].append(L"....");

    tetromino[4].append(L"..X.");
    tetromino[4].append(L".XX.");
    tetromino[4].append(L"..X.");
    tetromino[4].append(L"....");

    tetromino[5].append(L"..X.");
    tetromino[5].append(L".XX.");
    tetromino[5].append(L".X..");
    tetromino[5].append(L"....");

    tetromino[6].append(L"..X.");
    tetromino[6].append(L".XX.");
    tetromino[6].append(L"..X.");
    tetromino[6].append(L"....");


    pField = new unsigned char[nFieldHeight * nFieldWidth];

    for (int i = 0; i < nFieldWidth; i += 1)
    {
        for (int j = 0; j < nFieldHeight; j += 1)
        {
            pField[j * nFieldWidth + i] = i == 0 || i == nFieldWidth - 1 || j == nFieldHeight - 1 ? 9 : 0;
        }
    }

    CONSOLE_SCREEN_BUFFER_INFO csbi;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    nScreenWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    nScreenHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
    for (int i = 0; i < nScreenWidth * nScreenHeight; i += 1)
    {
        screen[i] = L' ';
    }
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, nullptr, CONSOLE_TEXTMODE_BUFFER, nullptr);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    bool bGameOver = false;

    int nCurrentPiece = rand() % 7;
    int nCurrentRotation = 0;
    int nCurrentX = nFieldWidth / 2;
    int nCurrentY = 0;

    int score = 0;

    DWORD keyHoldState = 0;

    while (!bGameOver)
    {
        DWORD keyState = GetKeyState();

        for (int shift = 0; shift < 4; shift += 1)
        {
            bool pressed = (1 << shift) & keyState;
            bool hold = (1 << shift) & keyHoldState;
            if (hold && pressed)
            {
                keyState &= ~(1 << shift);
            }
            else if (hold && !pressed)
            {
                keyHoldState &= ~(1 << shift);
                OutputDebugString(L"released\n");
            }
            else if (pressed)
            {
                keyHoldState |= 1 << shift;
                OutputDebugString(L"pressed\n");
            }
        }

        if (keyState & LEFT_PRESSED)
        {
            if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX - 1, nCurrentY))
            {
                nCurrentX -= 1;
            }
        }

        if (keyState & RIGHT_PRESSED)
        {
            if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX + 1, nCurrentY))
            {
                nCurrentX += 1;
            }

        }

        if (keyState & ROTATE_PRESSED)
        {
            if (DoesPieceFit(nCurrentPiece, nCurrentRotation + 1, nCurrentX, nCurrentY))
            {
                nCurrentRotation += 1;
            }

        }

        if (keyState & DOWN_PRESSED)
        {
            if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
            {
                nCurrentY += 1;
            }

        }

        if (timer.ForTimeSlice(20))
        {
            if (DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY + 1))
            {
                nCurrentY += 1;
            }
            else
            {
                for (int px = 0; px < 4; px += 1)
                {
                    for (int py = 0; py < 4; py += 1)
                    {
                        if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
                        {
                            pField[nFieldWidth * (py + nCurrentY) + px + nCurrentX] = nCurrentPiece + 1;
                        }
                    }
                }

                for (int y = nFieldHeight - 2; y > 0; y -= 1)
                {
                    bool isFull = true;
                    for (int x = 1; x < nFieldWidth - 1; x += 1)
                    {
                        if (!pField[nFieldWidth * y + x])
                        {
                            isFull = false;
                            break;
                        }
                    }
                    if (isFull)
                    {
                        for (int x = 1; x < nFieldWidth - 1; x += 1)
                        {
                            pField[nFieldWidth * y + x] = 8;
                        }
                        timer.BombTick(20);
                        score += 1;
                    }
                }

                nCurrentPiece = rand() % 7;
                nCurrentRotation = 0;
                nCurrentX = nFieldWidth / 2;
                nCurrentY = 0;

                bGameOver = !DoesPieceFit(nCurrentPiece, nCurrentRotation, nCurrentX, nCurrentY);
            }
        }

        if (timer.Bombing())
        {
            bool bombed = false;
            for (int y = nFieldHeight - 2; y > 0; y -= 1)
            {
                if (pField[nFieldWidth * y + 1] == 8)
                {
                    if (bombed)
                    {
                        timer.BombTick(15);
                        continue;
                    }
                    for (int py = 0; py + y > 0; py -= 1)
                    {
                        for (int x = 1; x < nFieldWidth - 1; x += 1)
                        {
                            pField[nFieldWidth * (py + y) + x] = pField[nFieldWidth * (py + y - 1) + x];
                        }
                    }

                    PlaySound(L"bomb.wav", NULL, SND_FILENAME | SND_ASYNC);

                    timer.CleanTick(45);
                    
                    bombed = true;
                    
                }
            }
        }

        if (timer.Cleaning())
        {
            PlayBackgroundMusic();
        }

        for (int i = 0; i < nFieldWidth; i += 1)
        {
            for (int j = 0; j < nFieldHeight; j += 1)
            {
                screen[(j + 5) * nScreenWidth + (i + 5)] = L" ABCDEFG=#"[pField[j * nFieldWidth + i]];
            }
        }

        for (int px = 0; px < 4; px += 1)
        {
            for (int py = 0; py < 4; py += 1)
            {
                if (tetromino[nCurrentPiece][Rotate(px, py, nCurrentRotation)] == L'X')
                {
                    screen[(nCurrentY + py + 5) * nScreenWidth + (nCurrentX + px + 5)] = nCurrentPiece + L'A';
                }
            }
        }

        wstring label = format(L"Score: {}", score);

        CopyMemory(screen, label.c_str(), label.size() * sizeof(wchar_t));

        WriteConsoleOutputCharacter(hConsole, screen, nScreenHeight * nScreenWidth, { 0, 0 }, &dwBytesWritten);
        ShowConsoleCursor(false);

        timer.Wait();
    }

}
