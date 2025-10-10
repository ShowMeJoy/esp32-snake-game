#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "font8x8_basic.h"
#include "ssd1306.h"

#define ADC_UNIT ADC_UNIT_1
#define ADC_X_CHANNEL ADC_CHANNEL_6 // GPIO34
#define ADC_Y_CHANNEL ADC_CHANNEL_7 // GPIO35
#define BTN_GPIO 32 

#define LED_PIN GPIO_NUM_2

#define TAG "SSD1306"

void init_i2c_if_needed(SSD1306_t *dev) {
#if CONFIG_I2C_INTERFACE
    ESP_LOGI(TAG, "INTERFACE is i2c");
    ESP_LOGI(TAG, "CONFIG_SDA_GPIO=%d", CONFIG_SDA_GPIO);
    ESP_LOGI(TAG, "CONFIG_SCL_GPIO=%d", CONFIG_SCL_GPIO);
    ESP_LOGI(TAG, "CONFIG_RESET_GPIO=%d", CONFIG_RESET_GPIO);

    i2c_master_init(dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
#endif // CONFIG_I2C_INTERFACE
}



void app_main(void) {
    SSD1306_t dev = {0};
    init_i2c_if_needed(&dev);
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

    gpio_config_t io_conf_out = {
        .pin_bit_mask = (1ULL << LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&io_conf_out);


    int position = 0;
    while (1) {
        int x, y;
        
        adc_oneshot_read(adc1_handle, ADC_X_CHANNEL, &x);
        adc_oneshot_read(adc1_handle, ADC_Y_CHANNEL, &y);
        int button = gpio_get_level(BTN_GPIO);

        if (y <= 2000) {
            position += 1;
            if (position > 8) position = 0;
            ssd1306_clear_screen(&dev, 0);
        }

        ssd1306_display_text(&dev, position, "Hello", 6, 0);


        ESP_LOGI("JOYSTICK", "X=%d  Y=%d  BTN=%d", x, y, button);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
