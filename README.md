# Game Boy Emulator

## How to compile

```bash
git clone https://github.com/pcineverdies/gameboy_emulator
cd gameboy_emulator
mkdir build
cd build
cmake .. [-DDEBUG=1] [-DPROFILE=1]
make
```

By adding the macro `DEBUG`, some debug information are displayed from the console.
By adding the macro `PROFILE`, the binary is compiled so that `gprof` can be used for profiling.

## How to use

```bash
./build/gameboy ./path/to/rom [fixed_fps]
```

The main program requires the path to the ROM as first argument. 
If a second argument is provided, the emulator runs at ~60FPS, otherwise it runs as fast as possible.

## Resources

- [gbops, an accurate opcode table for the Game Boy](https://izik1.github.io/gbops/index.html);
- [The EmuDev subreddit](https://www.reddit.com/r/EmuDev/) and the discord server;
- [This DAA instruction write-up](https://ehaskins.com/2018-01-30%20Z80%20DAA/);
- [This HALT-bug write-up](https://github.com/nitro2k01/little-things-gb/tree/main/double-halt-cancel);
- [Gameboy-doctor](https://github.com/robert/gameboy-doctor), to run blargg tests before the display was active;
- [This timer write-up](https://github.com/Hacktix/GBEDG/blob/master/timers/index.md);
- [This PPU write-up](https://hacktix.github.io/GBEDG/ppu/);
