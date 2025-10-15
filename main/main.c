#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include <stdio.h>
#include <stdlib.h>

#include "font8x8_basic.h"
#include "ssd1306.h"

#include "snake.h"

#define ADC_UNIT ADC_UNIT_1
#define ADC_X_CHANNEL ADC_CHANNEL_6 // GPIO34
#define ADC_Y_CHANNEL ADC_CHANNEL_7 // GPIO35
#define BTN_GPIO 32 

#define MAX_LEN 16
#define CELL_SIZE 8
#define GetSnakeTail(s) ((s)->head->front)

enum Direction {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
};
struct SnakeNode {
    int y;
    int x;
    struct SnakeNode *front;
};
struct Snake {
    int length;
    struct SnakeNode *head;
    enum Direction dir;
};
struct Snake* initsnake() {
    struct Snake *psnake = malloc(sizeof(struct Snake));
    if (!psnake) return NULL;

    psnake->dir = DIR_LEFT;
    psnake->length = 4;

    struct SnakeNode *n1 = malloc(sizeof(struct SnakeNode));
    struct SnakeNode *n2 = malloc(sizeof(struct SnakeNode));
    struct SnakeNode *n3 = malloc(sizeof(struct SnakeNode));
    struct SnakeNode *n4 = malloc(sizeof(struct SnakeNode));
    if (!n1 || !n2 || !n3 || !n4) {
        free(n1); free(n2); free(n3); free(n4); free(psnake);
        return NULL;
    }

    n1->y = 4; n1->x = 24;
    n2->y = 4; n2->x = 32;
    n3->y = 4; n3->x = 40;
    n4->y = 4; n4->x = 48;

    n1->front = n2;
    n2->front = n3;
    n3->front = n4;
    n4->front = NULL; 

    psnake->head = n1;
    return psnake;
}

void draw_snake(SSD1306_t *dev, struct Snake *psnake, const char *sign) {
    if (!psnake || !psnake->head) return;

    struct SnakeNode *psnode = GetSnakeTail(psnake);

    while (psnode) {
        ssd1306_display_text_box1(dev, psnode->y, psnode->x, sign, 1, 1, false, 0);
        psnode = psnode->front;
    }
}

void move_snake(struct Snake *psnake) {
    struct SnakeNode *psnode = GetSnakeTail(psnake);
    int prev_x = psnode->x;
    int prev_y = psnode->y;

    switch (psnake->dir) {
        case DIR_UP: psnode->y += 1; break;
        case DIR_DOWN: psnode->y -= 1; break;
        case DIR_LEFT: psnode->x -= 8; break;
        case DIR_RIGHT: psnode->x += 8; break;
    }    

    psnode = psnode->front;
    while (psnode + 1) {
        int tmp_x = psnode->x;
        int tmp_y = psnode->y;
        psnode->x = prev_x;
        psnode->y = prev_y;
        prev_x = tmp_x;
        prev_y = tmp_y;
        psnode = psnode->front;
    }
}


void app_main(void) {
    char* sign = "S";
    SSD1306_t dev = {0};
    i2c_master_init(&dev, 21, 22, -1);
    ssd1306_init(&dev, 128, 64);
    ssd1306_clear_screen(&dev, 0);

    // Joystick
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

    struct Snake *psnake = initsnake();

    while (1) {
        
        int x, y;
        adc_oneshot_read(adc1_handle, ADC_X_CHANNEL, &x);
        adc_oneshot_read(adc1_handle, ADC_Y_CHANNEL, &y);
        int button = gpio_get_level(BTN_GPIO);
        if (y <= 3400) {
            psnake->dir = DIR_DOWN;
        }
        if (y >= 3600) {
            psnake->dir = DIR_UP;
        }
        if (x <= 3300) {
            psnake->dir = DIR_LEFT;
        }
        if (x >= 3500) {
            psnake->dir = DIR_RIGHT;
        }   

        move_snake(psnake);
        ssd1306_clear_screen(&dev, 0);
        draw_snake(&dev, psnake, sign);
        ESP_LOGI("JOYSTICK", "X=%d  Y=%d  BTN=%d\n", x, y, button);
    

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
