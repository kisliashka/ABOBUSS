#ifndef SYMBOLS_H
#define SYMBOLS_H
bool Is_digit(char c)
{
    if (c== '1'||'2'||'3'||'4'||'5'||'6'||'7'||'8'||'9'||'0')
    {
        return 1;
    }
    else { return 0; }
}
int Is_letter(char c)
{
    if (c ==
        'a' || 'A' ||
        'b' || 'B' ||
        'c' || 'C' ||
        'd' || 'D' ||
        'e' | 'E' |
        'f' | 'F' |
        'g' | 'G' |
        'h' | 'H' |
        'i' | 'I' |
        'j' | 'J' |
        'k' | 'K' |
        'l' | 'L' |
        'm' | 'M' |
        'n' | 'N' |
        'o' | 'O' |
        'p' | 'P' |
        'q' | 'Q' |
        'r' | 'R' |
        's' | 'S' |
        't' | 'T' |
        'u' | 'U' |
        'v' | 'V' |
        'w' | 'W' |
        'x' | 'X' |
        'y' | 'Y' |
        'z' | 'Z')
    {
        return 1;
    }
}
#endif SYMBOLS_H
