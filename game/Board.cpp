#include <algorithm>

#include "Board.hpp"

using namespace Board;

std::vector<olc::vf2d> Rectangle::GetCorners()
{
    std::vector<olc::vf2d> corners;
    corners.push_back(pos);
    corners.push_back(pos + olc::vf2d{static_cast<float>(size.x), 0.0f});
    corners.push_back(pos + olc::vf2d{0.0f, static_cast<float>(size.y)});
    corners.push_back(pos + size);
    return corners;
}

bool Rectangle::IsCollidingWith(Rectangle&)
{
    // TODO: implement collision checking.
}

Paddle::Paddle(olc::PixelGameEngine* _game, float _pos_x, bool* _downButton, bool* _upButton)
{
    game = _game;

    // Initial conditions.
    speed = 300;
    size  = olc::vi2d{10,50};
    score = 0;

    // Sets and shifts position to account for width and height.
    pos  = olc::vf2d{_pos_x, static_cast<float>(game->ScreenHeight()) / 2.0f};
    pos -= size / 2.0f;

    downButton = _downButton;
    upButton   = _upButton;
}

void Paddle::Update(float fElapsedTime)
{
    // Moves paddle.
    if (*upButton)
    {
        pos.y -= speed * fElapsedTime;
    }
    else if (*downButton)
    {
        pos.y += speed * fElapsedTime;
    }

    // Keeps it within bounds.
    pos.y = std::max(pos.y, 0.0f);
    pos.y = std::min(pos.y, static_cast<float>(game->ScreenHeight() - size.y));

    Draw();
}

Ball::Ball(olc::PixelGameEngine* _game, bool* _serveButton)
{
    game = _game;

    // Initial conditions.
    speed = 300;
    size  = olc::vi2d{10,10};

    maxScore  = 5;
    nextServe = LEFT;
    state     = SERVE;

    // Puts the ball on the center of the screen.
    pos = startingPos = olc::vf2d{
        static_cast<float>(game->ScreenWidth()  - this->size.x)/2,
        static_cast<float>(game->ScreenHeight() - this->size.y)/2
        };

    serveButton = _serveButton;
}

void Ball::Update(float fElapsedTime)
{
    // Updates paddles.
    for (auto& i : paddles)
    {
        i.Update(fElapsedTime);
    }

    // TODO: implement game logic.
    if (state == SERVE)
    {

    }
    if (state == PLAY)
    {

    }
    if (state == WIN)
    {

    }

    Draw();
}
