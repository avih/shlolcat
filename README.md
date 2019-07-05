# shlolcat
lolcat rainbow colors as a POSIX shell script, with 8, 256 or true colors,
and few other tricks.

```
Usage: shlolcat [-h] [-f] [-cCOLORS] [-oO] [-sSAT] [-xDX] [-yDY] [FILE ...]
Concatenate files and print to standard output in rainbow colors.
With no FILE, or when FILE is -, read standard input.

  -h    Print this help and exit.
  -f    Force color even if standard output is not a terminal.
  -c 8|256|true: Colors to use. Default is true colors.
  -o O         : Offset the pattern (0-100). Default: random.
  -s SAT       : Saturation (0-100). Current: -s 75.
  -x DX, -y DY : horizontal/vertical color advancements (0-255), affect
                 pattern direction and density. Current: -x 10 -y 30.

Uses od, tr, sed, awk.
Home page: https://github.com/avih/shlolcat
```

### Screenshots

## Default (true colors, saturation 75):
![sgrtab screenshots using xterm](https://raw.githubusercontent.com/avih/auxiliary/master/images/shlolcat/default.png)

## De-saturated, horizontal, 256 colors `shlolcat -s40 -y0 -c256`:
![sgrtab screenshots using xterm](https://raw.githubusercontent.com/avih/auxiliary/master/images/shlolcat/custom.png)
