/*

    Paddle and Ball Classes for the game Pong.
    Ball also contains game control logic.

*/

#include <vector>

#include "olcPixelGameEngine.hpp"

namespace Board
{

class Rectangle
{
public:
    Rectangle() = default;

protected:
    olc::PixelGameEngine* game;

    olc::vf2d  pos;
    float      speed;
    olc::vi2d  size;
    olc::Pixel color = olc::GREY;

protected:
    std::vector<olc::vf2d> GetCorners();
    bool IsCollidingWith(Rectangle&);
    void Draw() { game->FillRect(pos, size, color); }
};

class Paddle : public Rectangle
{
public:
    Paddle() = default;
    Paddle(olc::PixelGameEngine*, float, bool*, bool*);

private:
    // Instance variables.
    bool     *upButton, *downButton;
    int       score = 0;

public:
    void Update(float);

private:
    int  IncrementScore() { score++; return score; }
    void ResetScore()     { score=0;}
};

class Ball : public Rectangle
{
public:
    Ball() = default;
    Ball(olc::PixelGameEngine*, bool*);

private:
    // Ball and game.
    olc::vf2d startingPos;
    olc::vf2d velocity;
    bool*     serveButton;
    int       maxScore;

    // Paddles.
    std::vector<Paddle> paddles;
    enum Paddles {LEFT, RIGHT};

    // Game state variables.
    int    nextServe, winner;
    enum   States {SERVE, WIN, PLAY};
    States state = SERVE;

public:
    void Update(float);

    void AddPaddle(Paddle& p) { paddles.push_back(p); }
};

}
