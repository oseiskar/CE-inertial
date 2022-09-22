#include "pico/stdlib.h"

#include <test.h>
#include <scha63x-runner.h>

int main() 
{
    stdio_init_all();
     
    sleep_ms(1000);
    
    func();
    scha63x_runner();
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    while (1) 
    {
        gpio_put(LED_PIN, 1);
        sleep_ms(250);
        gpio_put(LED_PIN, 0);
        sleep_ms(250);
        gpio_put(LED_PIN, 1);
        sleep_ms(250);
        gpio_put(LED_PIN, 0);
        sleep_ms(750);
    }
}
