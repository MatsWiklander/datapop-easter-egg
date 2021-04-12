#include <c64.h>

char *const SPRITES = $2000;
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

const char SCROLL_PIXELS_PER_FRAME = 4;
const char SPRITES_USED = 0b01111111;

const char sinus_table_length_x = 221;
const char sinus_table_x[] = {128, 131, 135, 138, 142, 146, 149, 153, 156,
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
const char sinus_table_length_y = 197;
const char sinus_table_y[] = {129, 132, 134, 137, 139, 142, 144, 146, 149,
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

void main()
{
    initialize();

    do
    {
        // Wait for raster
        do
        {
        } while (VICII->RASTER != $fe);
        do
        {
        } while (VICII->RASTER != $ff);
        soft_scroll();
        animate_sprites();
    } while (true);
}

// Soft-scroll using $d016 - trigger bit-scroll/char-scroll when needed
char scroll = 7;
void soft_scroll()
{
    if ((scroll -= SCROLL_PIXELS_PER_FRAME) == $ff)
    {
        scroll = 7;
        scroll_bit();
    }
    VICII->CONTROL2 = scroll;
}

// Scroll the next bit from the current char onto the screen - trigger next char if needed
char *current_character_generator = CHARGEN;
char current_bit = 1;
void scroll_bit()
{
    current_bit = current_bit / 2;

    if (current_bit == 0)
    {
        unsigned int character = get_next_character();
        current_character_generator = CHARGEN + character * 8;
        current_bit = $80;
    }

    hard_scroll();

    asm {
        sei
    }

    *PROCPORT = $32;

    char *sc = SCREEN + 1 * 40 + 39;
    for (char row : 0..7)
    {
        char bits = current_character_generator[row];
        char bit = ' ';
        if ((bits & current_bit) != 0)
        {
            bit = 128 + ' ';
        }
        *sc = bit;
        sc += 40;
    }

    *PROCPORT = $37;

    asm { 
        cli
    }
}

void hard_scroll()
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

char *nxt = TEXT;
// Find the next char of the scroll text
char get_next_character()
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

// Fill the screen with one char
void fill_screen(char *screen, char fill)
{
    for (char *cursor = screen; cursor < screen + 1000; cursor++)
    {
        *cursor = fill;
    }
}

void initialize()
{
    VICII->BG_COLOR = BLUE;
    VICII->BORDER_COLOR = BLUE;

    fill_screen(SCREEN, $20);
    fill_screen(COLORRAM, LIGHT_GREY);
    generate_sprites();
    // Wait for raster
    do
    {
    } while (VICII->RASTER != $fe);
    do
    {
    } while (VICII->RASTER != $ff);
    place_sprites();
}

char sinus_table_index_x = 0;
char sinus_table_index_y = 0;

void animate_sprites()
{
    char x_index = sinus_table_index_x;
    char y_index = sinus_table_index_y;
    char j2 = 12;
    char x_msb = 0;
    for (char j : 0..6)
    {
        unsigned int x = (unsigned int)$1e + sinus_table_x[x_index];
        x_msb = x_msb * 2 | > x;
        SPRITES_XPOS[j2] = < x;
        SPRITES_YPOS[j2] = sinus_table_y[y_index];
        x_index = x_index + 10;
        if (x_index >= sinus_table_length_x)
        {
            x_index = x_index - sinus_table_length_x;
        }
        y_index = y_index + 8;
        if (y_index >= sinus_table_length_y)
        {
            y_index = y_index - sinus_table_length_y;
        }
        j2 = j2 - 2;
    }
    VICII->SPRITES_XMSB = x_msb;

    // Increment sinus table indices
    if (++sinus_table_index_x >= sinus_table_length_x)
    {
        sinus_table_index_x = 0;
    }
    if (++sinus_table_index_y >= sinus_table_length_y)
    {
        sinus_table_index_y = 0;
    }
}

void place_sprites()
{

    VICII->SPRITES_ENABLE = SPRITES_USED;
    VICII->SPRITES_EXPAND_X = SPRITES_USED;
    VICII->SPRITES_EXPAND_Y = SPRITES_USED;
    char *sprites_ptr = SCREEN + $3f8;
    char spr_id = (char)((unsigned int)SPRITES / $40);
    char j2 = 0;
    char col = $5;
    for (char j : 0..6)
    {
        sprites_ptr[j] = spr_id++;
        SPRITES_XPOS[j2] = 0;
        SPRITES_YPOS[j2] = 0;
        SPRITES_COLOR[j] = WHITE;
        col = col ^ ($7 ^ $5);
        j2++;
        j2++;
    }
}

void generate_sprites()
{
    char cml[] = "datapop";
    char *sprites = SPRITES;
    for (char i : 0..6)
    {
        generate_character_generator_sprite(cml[i], sprites);
        sprites = sprites + $40;
    }
}

// Generate a sprite from a C64 CHARGEN character (by making each pixel 3x3 pixels large)
// - c is the character to generate
// - sprite is a pointer to the position of the sprite to generate
void generate_character_generator_sprite(char ch, char *sprite)
{
    char *character_generator = CHARGEN + ((unsigned int)ch) * 8;

    asm {
        sei
    }

    *PROCPORT = $32;

    for (char y : 0..7)
    {
        // current character_generator line
        char bits = character_generator[y];
        // current sprite char
        char s_gen = 0;
        // #bits filled into current sprite char
        char s_gen_cnt = 0;
        for (char x : 0..7)
        {
            // Find the current character_generator pixel (c)
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