#include <stdio.h>
#include "pico/stdlib.h"
#include "muimenu.hpp"


int main() {
    stdio_init_all();

    DisplayControls display;
    display.begin();

    printf("Scrolling Menu Example\n");

    while (true) {
        display.drawScreen();
    }
}

