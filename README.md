# pongoled

This is a little personal project of mine to reproduce
the classic Atari game Pong using an nRF52 DK.

A side goal for this project is to build a small driver
for the SSD1306 OLED device. I also hope to use this
repo as a starting point for future game emulations
(my next will be Asteroids!). Finally, I hope to gain some
experience with the Zephyr RTOS by completing this project.

(ditching VS Code for nvim, so updates may be sparse for a bit)

### Setup

Since I've moved away from using VS Code, we have to do everything
in the command line. Luckily this isn't *too* complicated, just a bit
of a pain.

Once you clone this repository, you're going to want to run the following
in order:

1. Create a Python 3.12 virtual environment with `python3.12 -m venv ./ncs/.venv`
2. Initialize the virtual environment with `source ./ncs/.venv/bin/activate`
3. Install west in the virtual environment with `pip install west`
4. Initialize the ncs west workspace with `west init -m https://github.com/nrfconnect/sdk-nrf --mr main ncs`
5. Once successful, `cd` into `ncs` and run `west update` (you should be prompted to do this)
6. This will take some time. Once done, `source zephyr/zephyr-env.sh`
7. Install zephyr requirements with `pip install -r zephyr/scripts/requirements.txt`
8. Copy `pongoled` into `ncs` (or clone `pongoled` at this point)
9. **Hopefully this step will work!** Build with `west build -b nrf52dk/nrf52832 -p always pongoled`

[!NOTE]
It's probably better to run steps 1-7 before cloning this repository. That leaves you
with a west workspace you can use whenever you need it. I believe that Zephyr would call
this a *T2* Zephyr topology!
