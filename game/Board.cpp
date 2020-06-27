#include <string>

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

/* ------------------------------------------------------
------------------- Paddle functions. -------------------
------------------------------------------------------ */

Paddle::Paddle(olc::PixelGameEngine* _game, float _pos_x, bool* _downButton, bool* _upButton)
{
    game = _game;

    // Initial conditions.
    speed = 400;
    size  = olc::vi2d{20,120};
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

/* ------------------------------------------------------
-------------------- Ball functions. --------------------
------------------------------------------------------ */

Ball::Ball(olc::PixelGameEngine* _game, bool* _serveButton)
{
    game = _game;

    // Initial conditions.
    startingSpeed = speed = 400;
    speedDelta    = 15;

    size = olc::vi2d{20,20};

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
        reset();
        if (!*serveButton)
            pressing = false;
        else if (*serveButton && !pressing)
        {            
            state = PLAY;

            // Generates random starting velocity, between -45° and 45°.
            int randX = 1 + rand() % 100;
            velocity    = olc::vf2d{
                static_cast<float>(randX),
                static_cast<float>(1 + rand() % randX)
            };
            velocity.y *= rand() % 2 == 1 ? 1.0f : -1.0f;
            velocity    = speed * velocity.norm();
            if (nextServe == P_RIGHT)
                velocity.x *= -1;
        }
    }

    // Play state.
    else if (state == PLAY)
    {
        pos += velocity * fElapsedTime;
        Edges oobEdge = KeepInbound();
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
                    speed += speedDelta;
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
            paddles[P_RIGHT].score++;
            if (paddles[P_RIGHT].score >= maxScore)
                state = WIN;
            else
                state = SERVE;
            break;
        case RIGHT:
            nextServe = P_RIGHT;
            winner    = P_LEFT;
            paddles[P_LEFT].score++;
            if (paddles[P_LEFT].score >= maxScore)
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
        reset();
        if (*serveButton)
        {
            for (auto& i : paddles)
            {
                i.score = 0;
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
        /* How much the paddle can push the ball and how angled
        the ball can leave after bouncing on the paddle */
        float maxPushAngle  = 60 * (3.14f/180.0f); // Radians.

        /* Angle the paddle will push the ball
        based on where it hit it, in radians. */
        float pushAngle =
            maxPushAngle
            // Signed distance between center of paddle and ball (x2).
            * (this->edges[TOP] + this->edges[BOTTOM] - p.edges[TOP] - p.edges[BOTTOM])
            // Max distance between centers (x2).
            / static_cast<float>(this->size.y + p.size.y);

        // New velocity.
        this->velocity = olc::vf2d{
            this->speed * static_cast<float>(cos(pushAngle)),
            this->speed * static_cast<float>(sin(pushAngle))
        };

        // Left side of the ball collides with right side of the paddle.
        if (contained[TOP_LEFT] || contained[BOTTOM_LEFT])
        {
            this->pos.x = p.edges[RIGHT];
        }
        // Right side of the ball collides with left side of the paddle.
        else
        {
            this->pos.x = p.edges[LEFT] - static_cast<float>(this->size.x);
            this->velocity.x *= -1;
        }
    }
}

/* ------------------------------------------------------
--------------- String drawing functions. ---------------
------------------------------------------------------ */

void DrawCenteredString
(
    olc::PixelGameEngine* game,
    float xOffset,
    float yOffset,
    std::string& s,
    olc::Pixel color,
    uint32_t scale
)
{
    /* Constants to decide how text should be shifted
    per character in order to center it. */
    float STR_Y_MULTIPLIER = 3.5;
    float STR_X_MULTIPLIER = 7.67;

    uint32_t x = static_cast<uint32_t>(
        static_cast<float>(game->ScreenWidth()) / 2
        - STR_X_MULTIPLIER * scale * static_cast<float>(s.length()) / 2
        + xOffset
    );
    uint32_t y = static_cast<uint32_t>(
        static_cast<float>(game->ScreenHeight()) / 2
        - STR_Y_MULTIPLIER * scale
        + yOffset
    );
    game->DrawString(x, y, s, color, scale);
}

void Ball::DrawScore()
{
    std::ostringstream stream;
    stream << paddles[P_LEFT].score << "\t" << paddles[P_RIGHT].score;
    std::string mes = stream.str();

    DrawCenteredString(game, 0, 0, mes, Board::BORDER_COLOR, 20);
}

void Ball::DrawServeMessage()
{
    std::ostringstream stream;
    stream << "Player ";
    if (nextServe == P_LEFT)
        stream << "1";
    if (nextServe == P_RIGHT)
        stream << "2";
    stream << ", it's your turn to serve!";
    std::string mes = stream.str();

    DrawCenteredString(game, 0, -200, mes, Board::BORDER_COLOR, 3);
}

void Ball::DrawWinMessage()
{
    std::ostringstream stream;
    stream << "Congratulations Player ";
    if (winner == P_LEFT)
        stream << "1";
    if (winner == P_RIGHT)
        stream << "2";
    stream << ", you've won!";
    std::string mes = stream.str();

    DrawCenteredString(game, 0, -200, mes, Board::BORDER_COLOR, 3);
}
