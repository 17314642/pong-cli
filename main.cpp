#include <iostream>
#include <string>
#include <Windows.h>
#include <thread>
#include <chrono>
#include <vector>
#include <random>
#include <algorithm>

int Width, Height;
std::vector<std::vector<char>> PresentBuffer, FutureBuffer;

void GetConsoleSize();
void FillFrameBuffer();
void DrawBoundingBox();
void DrawBall(int x, int y);
void DrawPaddle(int x, int y);
void DrawCharacter(int x, int y, const char ch);
void DrawString(int x, int y, const std::string str);
void GameLogic();
void RenderFrame();

int main()
{
    system("cls");
    
    GetConsoleSize();
    FillFrameBuffer();

    while (true)
    {
        DrawBoundingBox();
        GameLogic();
        DrawString(2, 0, "Pong");

        RenderFrame();

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    return 0;
}

// https://stackoverflow.com/a/23370070
void GetConsoleSize()
{
    static CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    Width = (int)(csbi.srWindow.Right - csbi.srWindow.Left + 1);
    Height = (int)(csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
}

void FillFrameBuffer()
{
    PresentBuffer.resize(Height);

    for (int i = 0; i < PresentBuffer.size(); i++)
    {
        PresentBuffer[i].resize(Width + 1); // One extra character for string termination
        std::fill(PresentBuffer[i].begin(), PresentBuffer[i].end(), ' ');
        
        PresentBuffer[i][PresentBuffer[i].size() - 1] = '\0';
    }

    FutureBuffer = PresentBuffer;
}

void DrawBoundingBox()
{
    for (int y = 0; y < Height; y++)
    {
        if (y == 0 || y == Height - 1)
        {
            for (int x = 0; x < Width; x++)
                DrawCharacter(x, y, '-');
        }
    }
}

void DrawString(int x, int y, const std::string str)
{
    for (int i = 0; i < str.length(); i++)
    {
        if (x + i < Width && y < Height)
            DrawCharacter(x + i, y, str[i]);
    }
}

void DrawBall(int x, int y)
{
    DrawCharacter(x - 1, y, '(');
    DrawCharacter(x, y, '*');
    DrawCharacter(x + 1, y, ')');
}

void DrawPaddle(int x, int y)
{
    DrawCharacter(x - 1, y, '|');
    DrawCharacter(x, y, ' ');
    DrawCharacter(x + 1, y, '|');

    DrawCharacter(x - 1, y + 1, '|');
    DrawCharacter(x, y + 1, ' ');
    DrawCharacter(x + 1, y + 1, '|');

    DrawCharacter(x - 1, y - 1, '|');
    DrawCharacter(x, y - 1, ' ');
    DrawCharacter(x + 1, y - 1, '|');

    DrawCharacter(x - 1, y + 2, '\\');
    DrawCharacter(x, y + 2, '_');
    DrawCharacter(x + 1, y + 2, '/');

    DrawCharacter(x - 1, y - 2, '_');
    DrawCharacter(x, y - 2, '_');
    DrawCharacter(x + 1, y - 2, '_');
}

void DrawCharacter(int x, int y, const char ch)
{
    // If passed (0, 0) coords, add 1 to it, so program won't go into undefined behavior because of negative array indexing.
    if (x == 0)
        x += 1;
    
    if (y == 0)
        y += 1;

    FutureBuffer[y - 1][x - 1] = ch;
}

#define DEFAULT_BALL_SPEED 40

void GameLogic()
{
    // https://en.cppreference.com/w/cpp/numeric/random
    static std::random_device r;

    static std::default_random_engine e1(r());
    static std::uniform_int_distribution<int> uniform_dist(0, 1);

    static auto start = std::chrono::system_clock::now();
    static auto ClockSinceLastScore = std::chrono::system_clock::now();

    static int BallX = Width / 2;
    static int BallY = Height / 2;

    static int Paddle1_X = 6;
    static int Paddle1_Y = 20;

    static int Paddle2_X = Width - 6;
    static int Paddle2_Y = 20;

    static int Score_1 = 0;
    static int Score_2 = 0;
    
    static bool MovingUp = uniform_dist(e1) == 0 ? true : false;
    static bool MovingLeft = uniform_dist(e1) == 1 ? true : false;

    static int BallSpeed = DEFAULT_BALL_SPEED; // in ms.

    auto CurrentTime = std::chrono::system_clock::now();
    if (std::chrono::duration<int, std::chrono::system_clock::period>(CurrentTime - start).count() >= BallSpeed * 10000)
    {
        if (std::chrono::duration<int, std::chrono::system_clock::period>(CurrentTime - ClockSinceLastScore).count() >= 15000 * 10000) // 15 seconds
        {
            BallSpeed -= 3;

            if (BallSpeed < 1)
                BallSpeed = 1;

            ClockSinceLastScore = std::chrono::system_clock::now();
        }

        // If collided with ceiling or floor
        if (BallY <= 2 || BallY >= Height - 2)
            MovingUp = !MovingUp;

        // If collided with walls
        if (BallX <= 2 || BallX >= Width - 2)
        {
            ClockSinceLastScore = std::chrono::system_clock::now();
            BallSpeed = DEFAULT_BALL_SPEED;

            if (BallX <= 2)
            {
                if (Score_2 < 9999)
                    Score_2 += 1;
                else
                    Score_2 = 0;
            }
            else
            {
                if (Score_1 < 9999)
                    Score_1 += 1;
                else
                    Score_1 = 0;
            }

            BallX = Width / 2;
            BallY = Height / 2;
            
            MovingUp = uniform_dist(e1) == 0 ? true : false;
            MovingLeft = uniform_dist(e1) == 1 ? true : false;
        }

        // If collided with paddles
        if (((BallX >= Paddle1_X - 1 && BallX <= Paddle1_X + 1) && (BallY >= Paddle1_Y - 2 && BallY <= Paddle1_Y + 2)) ||
            ((BallX >= Paddle2_X - 1 && BallX <= Paddle2_X + 1) && (BallY >= Paddle2_Y - 2 && BallY <= Paddle2_Y + 2)))
            MovingLeft = !MovingLeft;

        if (MovingUp)
            BallY--;
        else
            BallY++;

        if (MovingLeft)
            BallX--;
        else
            BallX++;


        // Clamp values or program will crash trying to write outside of FrameBuffer vector bounds.
        BallX = std::clamp(BallX, 2, Width - 2);
        BallY = std::clamp(BallY, 2, Height - 2);

        start = std::chrono::system_clock::now();
    }

    if (GetAsyncKeyState(0x57)) // W
    {
        if (Paddle1_Y - 2 != 1)
            Paddle1_Y--;
    }

    if (GetAsyncKeyState(0x53)) // S
    {
        if (Paddle1_Y + 2 != Height - 2)
            Paddle1_Y++;
    }

    if (GetAsyncKeyState(VK_UP)) // Arrow Up
    {
        if (Paddle2_Y - 2 != 1)
            Paddle2_Y--;
    }

    if (GetAsyncKeyState(VK_DOWN)) // Arrow Down
    {
        if (Paddle2_Y + 2 != Height - 2)
            Paddle2_Y++;
    }

    DrawBall(BallX, BallY);
    DrawPaddle(Paddle1_X, Paddle1_Y);
    DrawPaddle(Paddle2_X, Paddle2_Y);
    DrawString(1, Height - 1, std::string("Score: " + std::to_string(Score_1)));
    DrawString(Width - 11, Height - 1, std::string("Score: " + std::to_string(Score_2)));
    DrawString(Width - 15, 0, std::string("BallSpeed: " + std::to_string(BallSpeed)));
}

void SetCursorPosition(int x, int y)
{
    static const HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    std::cout.flush();
    COORD coord = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(hOut, coord);
}

void RenderFrame()
{
    for (int y = 0; y < Height; y++)
    {
        for (int x = 0; x < Width; x++)
        {
            if (FutureBuffer[y][x] != PresentBuffer[y][x])
            {
                SetCursorPosition(x, y);
                std::cout << FutureBuffer[y][x];
            }
        }
    }
    PresentBuffer = FutureBuffer;
    
    for (auto& column : FutureBuffer)
    {
        for (auto& ch : column)
            ch = ' ';
    }
}