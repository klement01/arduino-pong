/*

    Recreation of the classic game Pong using an Arduino Uno with pushbuttons as
    a controller through a USB connection. Game built with olc's PixelGameEngine:
    <https://github.com/OneLoneCoder/olcPixelGameEngine>.

*/

#include <iostream>

#include "olcPixelGameEngine.hpp"
#include "Board.hpp"
#include "SerialOpen.hpp"
#include "../controller/controller-info.hpp"

class Pong : public olc::PixelGameEngine
{
public:
    Pong() { sAppName = "Pong"; }

private:
    /* CONTROLLER VARIABLES. */

    // Serial connection.
    char confirmation[1] = {(char)CONFIRMATION_BYTE};
    char buffer[1];
    int  port;

    // Buttons.
    bool states[NUM_BUTTONS];
    enum Buttons
    {
        LEFT_DOWN,
        LEFT_UP,
        SERVE,
        RIGHT_DOWN,
        RIGHT_UP
    };

    /* GAME VARIABLES. */

    // Paddles and ball.
    Board::Paddle left = Board::Paddle(), right = Board::Paddle();
    Board::Ball   ball = Board::Ball();

public:
	bool OnUserCreate() override
	{
        // Tries to open serial port.
        // TODO: autodetect Arduino.        
        port = SerialOpen::port("/dev/ttyACM0", BAUD_RATE);
        if (port < 0)
        {
            std::cerr << "Error " -port << " opening port." << std::endl;
            return false;
        }

        // Paddle initialization.
        float horizontalOffset = 16.0f;
        left = Board::Paddle{
            this,
            horizontalOffset,
            &states[LEFT_DOWN],
            &states[LEFT_UP]
        };
        right = Board::Paddle{
            this,
            static_cast<float>(ScreenWidth()) - horizontalOffset,
            &states[RIGHT_DOWN],
            &states[RIGHT_UP]
        };

        // Ball initialization.
        ball = Board::Ball{
            this,
            &states[SERVE]
            };
        ball.AddPaddle(left);
        ball.AddPaddle(right);

        // Renders the background.
        int        borderWidth = 4;
        olc::Pixel borderColor = olc::DARK_GREY;
        olc::Pixel bgColor     = olc::VERY_DARK_BLUE;

        int bgLayer = CreateLayer();
        SetDrawTarget(bgLayer);

        // Draws board and border.
        Clear(borderColor);
        FillRect(
            olc::vi2d{borderWidth, borderWidth},
            olc::vi2d{ScreenWidth()  - (2*borderWidth), ScreenHeight() - (2*borderWidth)},
            bgColor
        );

        // Draws vertical line across middle.
        olc::vi2d rectSize = {borderWidth, 2*borderWidth};
        for (int i = borderWidth; i < ScreenHeight()+borderWidth; i+=rectSize.y*2)
        {
            FillRect(
                olc::vi2d{(ScreenWidth()-rectSize.x) / 2, i},
                rectSize,
                borderColor
            );
        }

        // Enables background and resets target.
        EnableLayer(bgLayer, true);
        SetDrawTarget(nullptr);

        return true;
    }

	bool OnUserUpdate(float fElapsedTime) override
	{
        Clear(olc::BLANK);

        ControllerUpdate();

        ball.Update(fElapsedTime);

        return true;
	}

private:
    void ControllerUpdate()
    {
        // Updates button states array.
        read(port, buffer, 1);
        std::cout << std::endl;
        for (int i = 0; i < NUM_BUTTONS; i++)
        {
            states[i] = (buffer[0] >> i) & 0x01;
            std::cout << "Button " << i << ": " << states[i] << std::endl;
        }
        write(port, confirmation, 1);
    }
};

int main()
{
    // Initializes the game window.
    Pong game;
    int  state = game.Construct(640,480,1,1);
    if(state)
    {
        game.Start();
    }
    else
    {
        std::cerr << "Error creating game window." << state << std::endl;
        return 1;
    }
    return 0;
}
