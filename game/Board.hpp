/*

    Paddle and Ball Classes for the game Pong.
    Ball also contains game control logic.

*/

#ifndef _BOARD_BLOCK
#define _BOARD_BLOCK

#include <vector>

#include "olcPixelGameEngine.hpp"

namespace Board
{

const olc::Pixel BORDER_COLOR      = olc::DARK_GREY;
const olc::Pixel BACKGROUND_COLOR  = olc::VERY_DARK_BLUE;
const olc::Pixel PLAY_OBJECT_COLOR = olc::GREY;

class Rectangle
{
public:
    Rectangle() = default;
    olc::PixelGameEngine* game;

    olc::vf2d  pos;
    float      speed;
    olc::vi2d  size;
    olc::Pixel color = PLAY_OBJECT_COLOR;

    // Used to determine collisions.
    enum  Corners {TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT};
    enum  Edges   {LEFT, TOP, RIGHT, BOTTOM, NO_EDGE};
    float edges[4];

    void UpdateEdges();
    bool CollidingWith(Rectangle&);
    bool Contains(olc::vf2d);

    Edges KeepInbound();
    void  Draw() { game->FillRect(pos, size, color); }
};

class Paddle : public Rectangle
{
public:
    Paddle() = default;
    Paddle(olc::PixelGameEngine*, float, bool*, bool*);

    int  score = 0;

private:
    bool *upButton, *downButton;

public:
    void Update(float);
};

class Ball : public Rectangle
{
public:
    Ball() = default;
    Ball(olc::PixelGameEngine*, bool*);

private:
    olc::vf2d startingPos;
    float     startingSpeed, speedDelta;
    olc::vf2d velocity;
    bool*     serveButton;
    int       maxScore;

    std::vector<Paddle> paddles;
    enum    Players {P_LEFT, P_RIGHT};
    Players nextServe, winner;

    enum   States {SERVE, WIN, PLAY};
    States state = SERVE;

    /* Used to prevent the same button press
    from being registered twice. */
    bool pressing;

public:
    void Update(float);
    void AddPaddle(Paddle& p) { paddles.push_back(p); }

private:
    void reset() { pos = startingPos; speed = startingSpeed; }

    void BounceOn(Paddle&);

    void DrawScore();
    void DrawServeMessage();
    void DrawWinMessage();
};

}

#endif
