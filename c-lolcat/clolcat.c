/*
 * clolcat - `cat` in rainbow colors
 * Copyright 2019 Avi Halachmi (:avih) avihpit@yahoo.com
 * License: MIT
 * Home page: https://github.com/avih/shlolcat
 */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#define TAB 8  /* assumption about the terminal */

void usage() {
    puts("Usage: clolcat [OPTIONS]... [FILE]...");
    puts("Concatenate files and print to standard output in rainbow colors.");
    puts("With no FILE, or when FILE is -, read standard input.");
    puts("");
    puts("  -h             Print this help and exit.");
    puts("  -e             Strip escape sequences from input.");
    puts("  -c COLORS      (1|8|256|true) Colors to use. Default: -c true.");
    puts("  -a MIN -b MAX  (0-255) R/G/B values limits. Default: -a 80 -b 255.");
    puts("  -x DX  -y DY   (0-255) char/line color advancement. Default: -x 10 -y 30.");
    puts("  -o OFFSET      (0-100) Offset the pattern. Default: random.");
    puts("");
    /* puts("Requires: od, tr, cksum, date."); */
    puts("Home page: https://github.com/avih/shlolcat");
}

void exit_err(const char *s) {
    if (s) fprintf(stderr, "Error: %s\n", s);
    fprintf(stderr, "Usage: clolcat -h | [-ea:b:c:x:y:o:] [FILE]...\n");
    exit(1);
}

int int0to(int high, const char *s, int *out) {
    *out = atoi(s);
    return !((*out > 0 && *out <= high) || !strcmp(s, "0"));
}

/* POSIX-compliant geto[pt] w/ shorter func/var names. To reset: oind=opos=1; */
/* For non-POSIX optional arguments as "x::" uncomment the line with `s[2]'.  */
static char *oarg; static int oerr=1, oind=1, oopt, opos=1;
static int geto(int c, char *const v[], const char *s) {
  #define opm_(s) oerr && c ? fprintf(stderr, "%s: "s" -- %c\n", *v, oopt) : 0
    if (oind > c || !v[oind] || *v[oind] != '-' || !v[oind][1]) return -1;
    if (v[oind][1] == '-' && !v[oind][2]) return oind++, -1;
    oarg=0, oopt=v[oind][opos], c=(*s != ':'); while(*s && *s != oopt) s++;
    if (!v[oind][++opos]) oind++, opos=1;
    if (!*s || *s == ':') return opm_("illegal option"), '?';
    if (s[1] != ':') return *s;
 /* if (s[2] == ':') return opos>1 ? oarg=v[oind++]+opos, opos=1 : 0, *s; */
    if (!v[oind++]) return opm_("option requires an argument"), c ? '?' : ':';
    return oarg=v[oind-1]+(opos>1 ? opos : 0), opos=1, *s;
  #undef opm_
}

void cycle_color(int d, int low, int high, int *r, int *g, int *b, int *s) {
    switch (*s) {
    case 0: if ((*g += d) >= high) { *g = high; (*s)++; }; break;
    case 1: if ((*r -= d) <= low ) { *r = low;  (*s)++; }; break;
    case 2: if ((*b += d) >= high) { *b = high; (*s)++; }; break;
    case 3: if ((*g -= d) <= low ) { *g = low;  (*s)++; }; break;
    case 4: if ((*r += d) >= high) { *r = high; (*s)++; }; break;
    case 5: if ((*b -= d) <= low ) { *b = low;  *s = 0; }; break;
    }
}

void write_color(int colors, int r, int g, int b) {
    switch (colors) {
    case  -1: printf("\x1b[38;2;%d;%d;%dm", r, g, b); break;
    case 256: printf("\x1b[38;5;%dm", 16 + 36*(r/45) + 6*(g/45) + b/45); break;
    case   8: printf("\x1b[%dm", 30 + 4*(b>127) + 2*(g>127) + (r>127)); break;
    case   1: printf("\x1b[%dm", (r+g+b) > 383); break;
    }
}

int main(int argc, char **argv) {
    int esc = 1, colors = -1, off = -1, lo = 80, hi = 255, dx = 10, dy = 30, ab = 0;
    int R, G, B, S, e, x, o, rv;

    while ((o = geto(argc, argv, "a:b:c:eho:x:y:")) != -1) {
        switch (o) {
        case 'h': usage(); return 0;
        case 'e': esc = 0; break;
        case 'o': if (int0to(100, oarg, &off)) exit_err("bad OFFSET"); break;
        case 'a': if (int0to(255, oarg, &lo)) exit_err("bad MIN"); ab=1; break;
        case 'b': if (int0to(255, oarg, &hi)) exit_err("bad MAX"); ab=1; break;
        case 'x': if (int0to(255, oarg, &dx)) exit_err("bad DX"); break;
        case 'y': if (int0to(255, oarg, &dy)) exit_err("bad DY"); break;
        case 'c': colors = atoi(oarg);
                  if (!strcmp(oarg, "t") || !strcmp(oarg, "true")) colors = -1;
                  else if (colors == 2) colors = 256;
                  else if (colors != 1 && colors != 8 && colors != 256) exit_err("bad COLOR");
                  break;
        default : exit_err(NULL);
        }
    }
    if ((colors == 1 || colors == 8) && !ab) lo = 0, hi = 255;
    if (off < 0) srand(time(0)), off = rand() % 101;

    R = hi; G = lo; B = lo; S = 0;
    while (off--) cycle_color(16, lo, hi, &R, &G, &B, &S);

    e = 0; x = 0; rv = 0; argv += oind;
    do {
        int r = R, g = G, b = B, s = S, c;
        FILE *f = *argv && strcmp("-", *argv) ? fopen(*argv, "rb") : stdin;
        if (!f) { fprintf(stderr, "Error: cannot open '%s'\n", *argv); rv = 1; continue; }

        while (EOF != (c = fgetc(f))) {
            if (e == 1) {  /* 1st char after escape: [->CSI, (/)->stay(rom) */
                if (esc) fputc(c, stdout);
                e = (c == '[') ? 2 : (c == '(' || c == ')') ? 1 : 0;
            } else if (e == 2) {  /* inside CSI sequence */
                if (esc) fputc(c, stdout);
                if (c >= 0x40 && c < 0x7f) e = 0;
            } else if (c == 0x1b) {  /* escape */
                if (esc) fputc(c, stdout);
                e = 1;
            } else if (c == '\n') {
                fputc(c, stdout);
                cycle_color(dy, lo, hi, &R, &G, &B, &S);
                x = 0; r = R; g = G; b = B; s = S;
            } else if (c == '\t') {
                cycle_color(dx, lo, hi, &r, &g, &b, &s);
                write_color(colors, r, g, b);
                fputc(c, stdout);
                while (++x % TAB)
                    cycle_color(dx, lo, hi, &r, &g, &b, &s);
            } else if (c < 128 || (c & 64)) {  /* ascii7 or utf8 leading byte */
                cycle_color(dx, lo, hi, &r, &g, &b, &s);
                write_color(colors, r, g, b);
                fputc(c, stdout);
                x++;
            } else {  /* utf8 continuation byte */
                fputc(c, stdout);
            }
        }
        if (f != stdin)
            fclose(f);

    } while (*argv && *(++argv));

    printf("\x1b[0m");
    return rv;
}
