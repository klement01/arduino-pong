# arduino-pong
A remake of the classic 2-player game Pong using an Arduino Uno as a controller and olcs's PixelGameEngine.

olcPixelGameEngine: <https://github.com/OneLoneCoder/olcPixelGameEngine>


# How to build
Connect 5 pushbuttons connected to ground and to digital pins 2, 4, 6, 8 and 12. These pins are Player 1 down, Player 1 up, Serve, Player 2 down and Player 2 up, respectively.

Change the device name in pong.cpp to whatever port the Arduino is connect to.

# TODO:
Add sound using the sound extension of the PixelGameEngine: <https://github.com/OneLoneCoder/olcPixelGameEngine/blob/master/Extensions/olcPGEX_Sound.h>

Make the program automatically detect the Arduino instead of hardcording its location.
