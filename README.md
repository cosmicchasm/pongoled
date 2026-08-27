# pongoled

This is a little personal project of mine to reproduce
the classic Atari game Pong using an nRF52 DK. For those of you who
are familiar with how the original Pong game was conceived, you will
recognize this project as being rather bloated (there was no CPU on
the original Pong game... this version uses an operating system).

A side goal for this project is/was to build a small driver
for various display devices. These has its own repository and is
included as a submodule in this project. Currently **only the SSD1306
OLED device is supported at the time of the v1.0.0 release.**

I also hope to use this repo as a starting point for future game emulations
(my next will be Asteroids!). This is my first time using Zephyr RTOS so
you may see some code in here that is far from optimal. I will happily take any
form of constructive criticism from the more experienced!

### Configure
Currently the easiest configs to change (like speed of the ball or stack sizes)
are in the form of macros. These are found in `inc/pong_core.h`.

Of course, more advanced users (especially those familiar with Zephyr projects) may
be able to configure in more impressive ways.

### Build

To build Pong for a Nordic Semi discovery kit, I'd recommend you install
the nRF Connect SDK for your development platform of choice. It's relatively
easy to do both in VS Code and in the command line (if you follow the instructions
[here](https://nrfconnectdocs.nordicsemi.com/ncs/latest/nrf/installation/install_ncs.html).

1. Clone this repository somewhere on your machine.
2. Use nRF Connect SDK to build and flash it to your Nordic board of choice.
    a. This usually involves running `west build ...` and `west flash ...` if using
    the command line.
3. If your board doesn't include a Nordic part, I'd recommend you follow
[these instructions](https://docs.zephyrproject.org/latest/develop/getting_started/index.html)
to install the Zephyr SDK and build the project (this is a little harder).

### Hardware

As mentioned above, I developed this project on an nRF52 DK from Nordic Semiconductor.
This board conveniently has female headers that I can connect my SSD1306 screen to.
