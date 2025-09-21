#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "u8g2.h"
#include "muiplusplus.hpp"
#include "muipp_u8g2.hpp"
#include "muipp_tpl.hpp"
#include "pico/multicore.h"
#include <Versatile_RotaryEncoder.h>
#include "u8g2functions.h"

// Define GPIOs for rotary encoder and buttons
#define ROTARY_clk 20
#define ROTARY_dt 21
#define ROTARY_sw 19

Versatile_RotaryEncoder encoder = Versatile_RotaryEncoder(ROTARY_clk, ROTARY_dt, ROTARY_sw);

// Define RST pin for display
#define SH1107_RST_PIN GPIO_PIN_7

// I2C Configuration
#define I2C_PORT i2c0
#define I2C_BAUDRATE 100000
#define I2C_SDA 4
#define I2C_SCL 5

// Fonts
#define MAIN_MENU_FONT u8g2_font_bauhaus2015_tr
#define SMALL_TEXT_FONT u8g2_font_glasstown_nbp_t_all

// Create screen
u8g2_t u8g2;

// Our menu container
MuiPlusPlus muiplus;

// Flag indicating that we entered menu screen
bool inMenu = false;

// Flag that indicates a screen refresh is required
bool refreshScreen = true;

// Messages for non-menu display operations
const char *incr = "incr button";
const char *decr = "decr button";
const char *ok = "ok button";
const char *anyk = "Press any key";
const char *quitmenu = "menu closed";

const char *stub_text = anyk;

// Forward declarations
void screen_render();
void setup_buttons();
void setup_menu();

// Button State
volatile bool btn_ok_pressed = false;
volatile bool btn_incr_pressed = false;
volatile bool btn_decr_pressed = false;
volatile bool btn_ok_long_press = false;
volatile bool buttons_init = false;
volatile uint8_t rotate_event = 0;

void handleRotate(int8_t rotation)
{
    if (rotation > 0)
        rotate_event = 2; // CW
    else
        rotate_event = 1; // CCW
}

void handlePressRelease()
{
    btn_ok_pressed = true;
}

void handleLongPressRelease()
{
    btn_ok_long_press = true;
}

void handle_events(void)
{
    // 0 = not pushed, 1 = pushed
    if (btn_ok_pressed)
    {
        if (inMenu)
        {
            if (muiplus.muiEvent(mui_event(mui_event_t::enter)).eid == mui_event_t::quitMenu)
            {
                stub_text = quitmenu;
                inMenu = false;
            }
        }
        else
        {
            stub_text = ok;
        }
        refreshScreen = true;
        // printf("ok click\n");
        btn_ok_pressed = false;
    }

    // 0 = not pushed, 1 = pushed
    if (btn_ok_long_press)
    {
        inMenu = true;
        refreshScreen = true;
        // printf("long press ok\n");
        btn_ok_long_press = false;
    }

    // 0 = not turning, 1 = CW, 2 = CCW
    if (rotate_event == 1)
    {
        if (inMenu)
        {
            muiplus.muiEvent(mui_event(mui_event_t::moveDown));
        }
        else
        {
            stub_text = incr;
        }
        refreshScreen = true;
        // printf("btn increment\n");
        rotate_event = 0;
    }

    if (rotate_event == 2)
    {
        if (inMenu)
        {
            muiplus.muiEvent(mui_event(mui_event_t::moveUp));
        }
        else
        {
            stub_text = decr;
        }
        refreshScreen = true;
        // printf("btn decrement\n");
        rotate_event = 0;
    }
}

int main()
{
    stdio_init_all();

    // Initialize I2C
    i2c_init(I2C_PORT, I2C_BAUDRATE);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Initialize u8g2 display
    // u8g2_Setup_sh1107_64x128_f(&u8g2, U8G2_R1, u8x8_byte_hw_i2c, u8x8_gpio_and_delay_raspberrypi_pico);
    //u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_hw_i2c, u8x8_gpio_and_delay_hw_i2c);
    u8g2_Setup_sh1106_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_hw_i2c, u8x8_gpio_and_delay_hw_i2c);
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);
    u8g2_SendBuffer(&u8g2);

    setup_menu();
    setup_buttons();

    // printf("Press any keys once to see it's actions\n");
    // printf("Long Press OK key to open menu\n");

    while (true)
    {
        encoder.ReadEncoder(); // Do the encoder reading and processing
        handle_events();
        screen_render();
        //       sleep_ms(100);
    }
}

void setup_buttons()
{
    encoder.setHandleRotate(handleRotate);
    encoder.setHandlePressRelease(handlePressRelease);
    encoder.setHandleLongPressRelease(handleLongPressRelease);
    encoder.setHandlePress(handle_events);
    buttons_init = true;
}

void screen_render()
{
    if (!refreshScreen)
        return;

    u8g2_ClearBuffer(&u8g2);

    if (inMenu)
    {
        // printf("Render menu\n");
        muiplus.render();
    }
    else
    {
        // printf("Render welcome screen\n");
        u8g2_SetFont(&u8g2, SMALL_TEXT_FONT);
        u8g2_DrawStr(&u8g2, 0, u8g2_GetDisplayHeight(&u8g2) / 2, stub_text);
    }

    u8g2_SendBuffer(&u8g2);
    refreshScreen = false;
}

void setup_menu()
{
    muiItemId root_page = muiplus.makePage("Simple page");
    muiplus.addMuippItem(new MuiItem_U8g2_PageTitle(u8g2, muiplus.nextIndex(), MAIN_MENU_FONT), root_page);

    muiItemId quit_idx = muiplus.nextIndex();
    auto quitbtn = new MuiItem_U8g2_ActionButton(
        u8g2,
        quit_idx,
        mui_event_t::quitMenu,
        "Quit menu",
        SMALL_TEXT_FONT,
        u8g2_GetDisplayWidth(&u8g2) / 2,
        u8g2_GetDisplayHeight(&u8g2) / 2,
        muipp::text_align_t::center,
        muipp::text_align_t::bottom);
    muiplus.addMuippItem(quitbtn, root_page);

    muiplus.addMuippItem(
        new MuiItem_U8g2_BackButton(u8g2, muiplus.nextIndex(), "<Back", SMALL_TEXT_FONT),
        root_page);

    muiplus.menuStart(root_page);
}
