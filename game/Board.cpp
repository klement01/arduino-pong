#include "Board.hpp"

using namespace Board;

void Rectangle::UpdateEdges()
{
    edges[LEFT]   = pos.x;
    edges[TOP]    = pos.y;
    edges[RIGHT]  = pos.x + static_cast<float>(size.x);
    edges[BOTTOM] = pos.y + static_cast<float>(size.y);
}

bool Rectangle::CollidingWith(Rectangle& r)
{
    return this->edges[LEFT]   <= r.edges[RIGHT]
        && this->edges[RIGHT]  >= r.edges[LEFT]
        && this->edges[TOP]    <= r.edges[BOTTOM]
        && this->edges[BOTTOM] >= r.edges[TOP];
}

bool Rectangle::Contains(olc::vf2d p)
{
    return this->edges[LEFT]   <= p.x
        && this->edges[RIGHT]  >= p.x
        && this->edges[TOP]    <= p.y
        && this->edges[BOTTOM] >= p.y;
}

Rectangle::Edges Rectangle::KeepInbound()
{
    /* Keeps the rectangle inbound and, if it was out
    of bounds, returns which direction it exited.
    Gives preference to OoB on the sides. */
    Edges oobValue = NO_EDGE;
    // Checks for OoB on top and bottom.
    if (pos.y < 0)
    {
        pos.y    = 0;
        oobValue = TOP;
    }
    else if (pos.y > game->ScreenHeight() - static_cast<float>(size.y))
    {
        pos.y    = game->ScreenHeight() - static_cast<float>(size.y);
        oobValue = BOTTOM;
    }
    // Checks for OoB on the sides.
    if (pos.x < 0)
    {
        pos.x    = 0;
        oobValue = LEFT;
    }
    else if (pos.x > game->ScreenWidth() - static_cast<float>(size.x))
    {
        pos.x    = game->ScreenWidth() - static_cast<float>(size.x);
        oobValue = RIGHT;
    }
    return oobValue;
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

    KeepInbound();
    Draw();
}

Ball::Ball(olc::PixelGameEngine* _game, bool* _serveButton)
{
    game = _game;

    // Initial conditions.
    speed = 300;
    size  = olc::vi2d{10,10};

    maxScore  = 5;
    nextServe = P_LEFT;
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

    // Serve state.
    if (state == SERVE)
    {
        DrawServeMessage();
        pos = startingPos;
        if (!*serveButton)
            pressing = false;
        else if (*serveButton && !pressing)
        {            
            state = PLAY;
            if (nextServe == P_LEFT)
                velocity = olc::vf2d{+speed, 0.0f};
            else if (nextServe == P_RIGHT)
                velocity = olc::vf2d{-speed, 0.0f};
        }
    }

    // Play state.
    else if (state == PLAY)
    {
        pos += velocity * fElapsedTime;
        Edges oobEdge = KeepInbound();
        int newScore;
        switch (oobEdge)
        {
        // Ball was not out of bounds:
        case NO_EDGE:
            /* If the ball is colliding with
            paddle, bounce on it. */
            this->UpdateEdges();
            for (auto& i : paddles)
            {
                i.UpdateEdges();
                if (this->CollidingWith(i))
                {
                    this->BounceOn(i);
                    break;
                }
            }
            break;
        // Ball bounces on an edge of the board:
        case TOP:
        case BOTTOM:
            velocity.y = -velocity.y;
            break;
        // Ball leaves the board through the sides:
        case LEFT:
            nextServe = P_LEFT;
            winner    = P_RIGHT;
            newScore  = paddles[P_RIGHT].IncrementScore();
            if (newScore >= maxScore)
                state = WIN;
            else
                state = SERVE;
            break;
        case RIGHT:
            nextServe = P_RIGHT;
            winner    = P_LEFT;
            newScore  = paddles[P_LEFT].IncrementScore();
            if (newScore >= maxScore)
                state = WIN;
            else
                state = SERVE;
            break;
        }
    }

    // Win state.
    else if (state == WIN)
    {
        DrawWinMessage();
        pos = startingPos;
        if (*serveButton)
        {
            for (auto& i : paddles)
            {
                i.ResetScore();
            }
            pressing = true;
            state = SERVE;
        }
    }

    DrawScore();
    Draw();
}

void Ball::BounceOn(Paddle& p)
{
    // Checks which edges of the ball are inside the paddle.
    bool contained[4];
    contained[TOP_LEFT]     = p.Contains(olc::vf2d{this->edges[LEFT],  this->edges[TOP]});
    contained[TOP_RIGHT]    = p.Contains(olc::vf2d{this->edges[RIGHT], this->edges[TOP]});
    contained[BOTTOM_LEFT]  = p.Contains(olc::vf2d{this->edges[LEFT],  this->edges[BOTTOM]});
    contained[BOTTOM_RIGHT] = p.Contains(olc::vf2d{this->edges[RIGHT], this->edges[BOTTOM]});
    // How much of the ball must be inside the paddle for it to count as a side hit:
    float tolerance = 0.1 * static_cast<float>(this->size.y);
    // If the ball collides on the top or bottom of the paddle:
    if (abs(this->edges[BOTTOM] - p.edges[TOP])    < tolerance
        || abs(this->edges[TOP] - p.edges[BOTTOM]) < tolerance)
    {
        this->velocity.y *=-1;
        // Top of the ball collides with bottom of the paddle.
        if (contained[TOP_LEFT] || contained[TOP_RIGHT])
            this->pos.y = p.edges[BOTTOM];
        // Bottom of the ball collides with top of the paddle.
        else
            this->pos.y = p.edges[TOP] - static_cast<float>(this->size.y);
    }
    // If the ball collides on the sides of the paddle:
    else
    {
        // Maximum push the paddle can give to the ball, in degrees.
        float maxAngle = 60;

        /* Angle the paddle will push the ball
        based on where it hit it, in radians. */
        float pushAngle =
            maxAngle
            // Conversion from degrees to radians.
            * (3.14f/180.0f)
            // Signed distance between center of paddle and ball (x2).
            * (this->edges[TOP] - p.edges[TOP] + this->edges[BOTTOM] - p.edges[BOTTOM])
            // Max distance between centers (x2).
            / static_cast<float>(this->size.y + p.size.y);

        // Unit vector representing the push.
        olc::vf2d pushVector = {
            static_cast<float>(cos(pushAngle)),
            static_cast<float>(sin(pushAngle))
        };

        // Left side of the ball collides with right side of the paddle.
        if (contained[TOP_LEFT] || contained[BOTTOM_LEFT])
            this->pos.x = p.edges[RIGHT];
        // Right side of the ball collides with left side of the paddle.
        else
            this->pos.x = p.edges[LEFT] - static_cast<float>(this->size.x);
            pushVector.x *= -1;

        // Calculates new velocity vector from old velocity and push.
        this->velocity.x *= -1;
        this->velocity    = this->velocity.norm() + pushVector;
        this->velocity    = this->velocity.norm() * this->speed;
    }
}

void Ball::DrawScore()
{

}

void Ball::DrawServeMessage()
{

}

void Ball::DrawWinMessage()
{

}
