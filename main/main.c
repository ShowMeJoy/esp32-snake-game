#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include <stdio.h>
#include <stdlib.h>

#include "font8x8_basic.h"
#include "ssd1306.h"

#define ADC_UNIT ADC_UNIT_1
#define ADC_X_CHANNEL ADC_CHANNEL_6 // GPIO34
#define ADC_Y_CHANNEL ADC_CHANNEL_7 // GPIO35
#define BTN_GPIO 32 

void app_main(void) {
    SSD1306_t dev = {0};
    i2c_master_init(&dev, 21, 22, -1);
    ssd1306_init(&dev, 128, 64);
    ssd1306_clear_screen(&dev, 0);


    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT
    };
    adc_oneshot_new_unit(&init_cfg, &adc1_handle);

    adc_oneshot_chan_cfg_t channel_cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_6
    };
    adc_oneshot_config_channel(adc1_handle, ADC_X_CHANNEL, &channel_cfg);
    adc_oneshot_config_channel(adc1_handle, ADC_Y_CHANNEL, &channel_cfg);

    gpio_config_t io_conf_in = {
        .pin_bit_mask = 1ULL << BTN_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE
    };
    gpio_config(&io_conf_in);

    int position_y = 0;
    int position_x = 0;
    const char* sign = "S";
    while (1) {
        int x, y;
        
        adc_oneshot_read(adc1_handle, ADC_X_CHANNEL, &x);
        adc_oneshot_read(adc1_handle, ADC_Y_CHANNEL, &y);
        int button = gpio_get_level(BTN_GPIO);

        if (y <= 3400) {
            position_y -= 1;
            if (position_y < 0) position_y = 0;
            ssd1306_clear_screen(&dev, 0);
        }

        if (y >= 3600) {
            position_y += 1;
            if (position_y >= 8) position_y = 0;
            ssd1306_clear_screen(&dev, 0);
        }

        if (x <= 3300) {
            position_x -= 8;
            if (position_x < 0) position_x = 0;
            ssd1306_clear_screen(&dev, 0);
        }

        if (x >= 3500) {
            position_x += 8;
            if (position_x >= 128) position_x = 0;
            ssd1306_clear_screen(&dev, 0);
        }

        ssd1306_display_text_box1(&dev, position_y, position_x, sign, 1, 1, false, 0);

        printf("x = %d, y = %d\n", position_x, position_y);
        ESP_LOGI("JOYSTICK", "X=%d  Y=%d  BTN=%d\n", x, y, button);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
