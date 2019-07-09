# shlolcat
lolcat rainbow colors as a POSIX shell script, with 8, 256 or true colors,
and few other tricks.

```
Usage: shlolcat [OPTIONS]... [FILE]...
Concatenate files and print to standard output in rainbow colors.
With no FILE, or when FILE is -, read standard input.

  -h             Print this help and exit.
  -e             Strip escape sequences from input.
  -c COLORS      (8|256|true) Colors to use. Default: -c true.
  -a MIN -b MAX  (0-255) R/G/B values limits. Default: -a 80 -b 255.
  -x DX  -y DY   (0-255) char/line color advancement. Default: -x 10 -y 30.
  -o OFFSET      (0-100) Offset the pattern. Default: random.

Uses od, tr, awk.
Home page: https://github.com/avih/shlolcat
```

## Screenshots

### Default:
![sgrtab screenshots using xterm](https://raw.githubusercontent.com/avih/auxiliary/master/images/shlolcat/default.png)

### De-saturated, horizontal, 256 colors (`shlolcat -a 150 -y 0 -c 256`):
![sgrtab screenshots using xterm](https://raw.githubusercontent.com/avih/auxiliary/master/images/shlolcat/custom.png)
