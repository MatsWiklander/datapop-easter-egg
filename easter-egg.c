#include <c64.h>

const char sinlen_x = 221;
const char sintab_x[] = {128, 131, 135, 138, 142, 146, 149, 153, 156,
                         160, 163, 167, 170, 174, 177, 180, 184, 187,
                         190, 193, 196, 199, 202, 205, 208, 211, 213,
                         216, 219, 221, 224, 226, 228, 230, 232, 234,
                         236, 238, 240, 242, 243, 245, 246, 247, 249,
                         250, 251, 252, 252, 253, 254, 254, 254, 255,
                         255, 255, 255, 255, 255, 254, 254, 253, 253,
                         252, 251, 250, 249, 248, 247, 245, 244, 242,
                         241, 239, 237, 235, 233, 231, 229, 227, 225,
                         222, 220, 217, 215, 212, 209, 206, 204, 201,
                         198, 195, 192, 188, 185, 182, 179, 175, 172,
                         168, 165, 162, 158, 154, 151, 147, 144, 140,
                         137, 133, 129, 126, 122, 118, 115, 111, 108,
                         104, 101, 97, 93, 90, 87, 83, 80, 76, 73,
                         70, 67, 63, 60, 57, 54, 51, 49, 46, 43, 40,
                         38, 35, 33, 30, 28, 26, 24, 22, 20, 18, 16,
                         14, 13, 11, 10, 8, 7, 6, 5, 4, 3, 2, 2, 1,
                         1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 3, 3, 4,
                         5, 6, 8, 9, 10, 12, 13, 15, 17, 19, 21, 23,
                         25, 27, 29, 31, 34, 36, 39, 42, 44, 47, 50,
                         53, 56, 59, 62, 65, 68, 71, 75, 78, 81, 85,
                         88, 92, 95, 99, 102, 106, 109, 113, 117,
                         120, 124};
const char sinlen_y = 197;
const char sintab_y[] = {129, 132, 134, 137, 139, 142, 144, 146, 149,
                         151, 154, 156, 159, 161, 163, 165, 168, 170,
                         172, 174, 176, 178, 180, 182, 184, 186, 187,
                         189, 191, 192, 194, 195, 196, 198, 199, 200,
                         201, 202, 203, 204, 205, 205, 206, 206, 207,
                         207, 208, 208, 208, 208, 208, 208, 208, 207,
                         207, 207, 206, 206, 205, 204, 203, 203, 202,
                         201, 199, 198, 197, 196, 194, 193, 191, 190,
                         188, 186, 185, 183, 181, 179, 177, 175, 173,
                         171, 169, 166, 164, 162, 160, 157, 155, 153,
                         150, 148, 145, 143, 140, 138, 135, 133, 130,
                         128, 125, 123, 120, 118, 115, 113, 110, 108,
                         105, 103, 101, 98, 96, 94, 92, 89, 87, 85,
                         83, 81, 79, 77, 75, 73, 72, 70, 68, 67, 65,
                         64, 62, 61, 60, 59, 57, 56, 55, 55, 54, 53,
                         52, 52, 51, 51, 51, 50, 50, 50, 50, 50, 50,
                         50, 51, 51, 52, 52, 53, 53, 54, 55, 56, 57,
                         58, 59, 60, 62, 63, 64, 66, 67, 69, 71, 72,
                         74, 76, 78, 80, 82, 84, 86, 88, 90, 93, 95,
                         97, 99, 102, 104, 107, 109, 112, 114, 116,
                         119, 121, 124, 126};
char *const sprites = $2000;
char *SCREEN = $0400;
char *TEXT = "travellers - ep version 04:22    "
             "asteroid - ep version 05:53    "
             "the end - ep version 05:18    "
             "space operators - ep version 04:12    "
             "mot nya varldar - ep version 05:49    "
             "weltraum - anymachine remix 05:20    "
             "sterne - 808dotpop mix 08:41    "
             "sterne - the metroland constellation remix 05:01    "
             "weltraum - independent state remix 06:25    ";

void main()
{
    init();
    do
    {
        // Wait for raster
        do
        {
        } while (VICII->RASTER != $fe);
        do
        {
        } while (VICII->RASTER != $ff);
        ++VICII->BG_COLOR;
        scroll_soft();
        anim();
        --VICII->BG_COLOR;
    } while (true);
}

// Soft-scroll using $d016 - trigger bit-scroll/char-scroll when needed
char scroll = 7;
void scroll_soft()
{
    if ((scroll -= 4) == $ff)
    {
        scroll = 7;
        scroll_bit();
    }
    VICII->CONTROL2 = scroll;
}

// Scroll the next bit from the current char onto the screen - trigger next char if needed
char *current_chargen = CHARGEN;
char current_bit = 1;
void scroll_bit()
{
    current_bit = current_bit / 2;

    if (current_bit == 0)
    {
        unsigned int c = next_char();
        current_chargen = CHARGEN + c * 8;
        current_bit = $80;
    }

    scroll_hard();

    asm {
        sei
    }

    *PROCPORT = $32;

    char *sc = SCREEN + 1 * 40 + 39;
    for (char r : 0..7)
    {
        char bits = current_chargen[r];
        char b = ' ';
        if ((bits & current_bit) != 0)
        {
            b = 128 + ' ';
        }
        *sc = b;
        sc += 40;
    }

    *PROCPORT = $37;

    asm { 
        cli
    }
}

char *nxt = TEXT;
// Find the next char of the scroll text
char next_char()
{
    char c = *nxt;
    if (c == 0)
    {
        nxt = TEXT;
        c = *nxt;
    }
    nxt++;
    return c;
}

void scroll_hard()
{
    // Hard scroll
    for (char i = 0; i != 39; i++)
    {
        (SCREEN + 40 * 0)[i] = (SCREEN + 40 * 0)[i + 1];
        (SCREEN + 40 * 1)[i] = (SCREEN + 40 * 1)[i + 1];
        (SCREEN + 40 * 2)[i] = (SCREEN + 40 * 2)[i + 1];
        (SCREEN + 40 * 3)[i] = (SCREEN + 40 * 3)[i + 1];
        (SCREEN + 40 * 4)[i] = (SCREEN + 40 * 4)[i + 1];
        (SCREEN + 40 * 5)[i] = (SCREEN + 40 * 5)[i + 1];
        (SCREEN + 40 * 6)[i] = (SCREEN + 40 * 6)[i + 1];
        (SCREEN + 40 * 7)[i] = (SCREEN + 40 * 7)[i + 1];
    }
}

// Fill the screen with one char
void fillscreen(char *screen, char fill)
{
    for (char *cursor = screen; cursor < screen + 1000; cursor++)
    {
        *cursor = fill;
    }
}

void init()
{
    VICII->BG_COLOR = BLUE;
    VICII->BORDER_COLOR = BLUE;

    fillscreen(SCREEN, $20);
    fillscreen(COLORRAM, LIGHT_GREY);

    place_sprites();
    gen_sprites();
}

char sin_idx_x = 0;
char sin_idx_y = 0;

void anim()
{
    char xidx = sin_idx_x;
    char yidx = sin_idx_y;
    char j2 = 12;
    char x_msb = 0;
    for (char j : 0..6)
    {
        unsigned int x = (unsigned int)$1e + sintab_x[xidx];
        x_msb = x_msb * 2 | > x;
        SPRITES_XPOS[j2] = < x;
        SPRITES_YPOS[j2] = sintab_y[yidx];
        xidx = xidx + 10;
        if (xidx >= sinlen_x)
        {
            xidx = xidx - sinlen_x;
        }
        yidx = yidx + 8;
        if (yidx >= sinlen_y)
        {
            yidx = yidx - sinlen_y;
        }
        j2 = j2 - 2;
    }
    VICII->SPRITES_XMSB = x_msb;

    // Increment sin indices
    if (++sin_idx_x >= sinlen_x)
    {
        sin_idx_x = 0;
    }
    if (++sin_idx_y >= sinlen_y)
    {
        sin_idx_y = 0;
    }
}

void place_sprites()
{
    VICII->SPRITES_ENABLE = 127;   // %01111111;
    VICII->SPRITES_EXPAND_X = 127; // %01111111;
    VICII->SPRITES_EXPAND_Y = 127; // %01111111;
    char *sprites_ptr = SCREEN + $3f8;
    char spr_id = (char)((unsigned int)sprites / $40);
    char spr_x = 60;
    char j2 = 0;
    char col = $5;
    for (char j : 0..6)
    {
        sprites_ptr[j] = spr_id++;
        SPRITES_XPOS[j2] = spr_x;
        SPRITES_YPOS[j2] = 80;
        SPRITES_COLOR[j] = WHITE;
        spr_x = spr_x + 32;
        col = col ^ ($7 ^ $5);
        j2++;
        j2++;
    }
}

void gen_sprites()
{
    char cml[] = "datapop";
    char *spr = sprites;
    for (char i : 0..6)
    {
        gen_chargen_sprite(cml[i], spr);
        spr = spr + $40;
    }
}

// Generate a sprite from a C64 CHARGEN character (by making each pixel 3x3 pixels large)
// - c is the character to generate
// - sprite is a pointer to the position of the sprite to generate
void gen_chargen_sprite(char ch, char *sprite)
{
    char *chargen = CHARGEN + ((unsigned int)ch) * 8;

    asm {
        sei
    }

    *PROCPORT = $32;

    for (char y : 0..7)
    {
        // current chargen line
        char bits = chargen[y];
        // current sprite char
        char s_gen = 0;
        // #bits filled into current sprite char
        char s_gen_cnt = 0;
        for (char x : 0..7)
        {
            // Find the current chargen pixel (c)
            char c = 0;
            if ((bits & $80) != 0)
            {
                c = 1;
            }
            // generate 3 pixels in the sprite char (s_gen)
            for (char b : 0..2)
            {
                s_gen = s_gen * 2 | c;
                if (++s_gen_cnt == 8)
                {
                    // sprite char filled - store and move to next char
                    sprite[0] = s_gen;
                    sprite[3] = s_gen;
                    sprite[6] = s_gen;
                    sprite++;
                    s_gen = 0;
                    s_gen_cnt = 0;
                }
            }
            // move to next char pixel
            bits = bits * 2;
        }
        // move 3 lines down in the sprite (already moved 1 through ++)
        sprite = sprite + 6;
    }

    *PROCPORT = $37;

    asm {
        cli
    }
}
