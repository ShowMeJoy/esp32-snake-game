#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_random.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "font8x8_basic.h"
#include "ssd1306.h"

#include "snake.h"

#define ADC_UNIT ADC_UNIT_1
#define ADC_X_CHANNEL ADC_CHANNEL_6 // GPIO34
#define ADC_Y_CHANNEL ADC_CHANNEL_7 // GPIO35
#define BTN_GPIO 32 

#define MAX_LEN 16
#define CELL_SIZE 8
#define GetNextNode(s) ((s)->head->front)

enum Direction {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
    STOP
};
struct SnakeNode {
    int y;
    int x;
    struct SnakeNode *front;
};
struct Snake {
    int length;
    struct SnakeNode *head;
    struct SnakeNode *tail;
    enum Direction dir;
};
struct Food {
    int x;
    int y;
    bool f;
};

struct SnakeNode* GetTail(struct Snake *psnake) {
    struct SnakeNode *psnode = psnake->head;
    while(psnode && psnode->front) psnode = psnode->front;
    return psnode;
}

struct Snake* initsnake() {
    struct Snake *psnake = malloc(sizeof(struct Snake));
    if (!psnake) return NULL;

    psnake->dir = DIR_RIGHT;
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
    psnake->tail = n4;
    return psnake;
}

void move_snake(struct Snake *psnake) {
    struct SnakeNode *psnode = GetNextNode(psnake);
    int prev_x = psnode->x;
    int prev_y = psnode->y;

    switch (psnake->dir) {
        case DIR_UP: psnode->y += 1; break;
        case DIR_DOWN: psnode->y -= 1; break;
        case DIR_LEFT: psnode->x -= 8; break;
        case DIR_RIGHT: psnode->x += 8; break;
        case STOP: break;
    }    

    psnode = psnode->front;
    while (psnode) {
        if (psnake->dir == STOP) break;
        int tmp_x = psnode->x;
        int tmp_y = psnode->y;
        psnode->x = prev_x;
        psnode->y = prev_y;
        prev_x = tmp_x;
        prev_y = tmp_y;
        psnode = psnode->front;
    }
}

void draw_snake(SSD1306_t *dev, struct Snake *psnake, int old_tail_y, int old_tail_x) {
    if (!psnake || !psnake->head) return;
    char* sign = "*";
    if (psnake->dir != STOP) {
        ssd1306_display_text_box1(dev, old_tail_y, old_tail_x, " ", 1, 1, false, 0);
    }
    struct SnakeNode *psnode = GetNextNode(psnake);

    while (psnode) {
        ssd1306_display_text_box1(dev, psnode->y, psnode->x, sign, 1, 1, false, 0);
        psnode = psnode->front;
    }
}

typedef struct JoystickHandle {
    adc_oneshot_unit_handle_t adc_handle;
} JoystickHandle;

JoystickHandle joystick_conf(void) {
    JoystickHandle joy;

    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT
    };
    adc_oneshot_new_unit(&init_cfg, &joy.adc_handle);

    adc_oneshot_chan_cfg_t channel_cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_6
    };
    adc_oneshot_config_channel(joy.adc_handle, ADC_X_CHANNEL, &channel_cfg);
    adc_oneshot_config_channel(joy.adc_handle, ADC_Y_CHANNEL, &channel_cfg);

    gpio_config_t io_conf_in = {
        .pin_bit_mask = 1ULL << BTN_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE
    };
    gpio_config(&io_conf_in);

    return joy;
}

void joystick_moving(struct Snake *psnake, JoystickHandle joystick) {
    int x, y;
    adc_oneshot_read(joystick.adc_handle, ADC_X_CHANNEL, &x);
    adc_oneshot_read(joystick.adc_handle, ADC_Y_CHANNEL, &y);
    int button = gpio_get_level(BTN_GPIO);

    // Также блок на разворот на 180 градусов
    if (y <= 3400 && (psnake->dir != DIR_UP)) {
        psnake->dir = DIR_DOWN;
    }
    if (y >= 3600 && (psnake->dir != DIR_DOWN)) {
        psnake->dir = DIR_UP;
    }
    if (x <= 3300 && (psnake->dir != DIR_RIGHT)) {
        psnake->dir = DIR_LEFT;
    }
    if (x >= 3500 && (psnake->dir != DIR_LEFT)) {
        psnake->dir = DIR_RIGHT;
    }   
    if (!button) {
        psnake->dir = STOP;
    }

    // ESP_LOGI("JOYSTICK", "X=%d  Y=%d  BTN=%d\n", x, y, button);
}

bool food_is_here(struct Snake *psnake, struct Food *apple) {
    struct SnakeNode *psnode = psnake->head;

    printf("spawn try: %d %d\n", apple->x, apple->y);
    printf("spawn snake: %d %d\n", psnode->x, psnode->y);

    for (int i = 0; i < psnake->length; ++i) {
        if (psnode->x == apple->x && psnode->y == apple->y) {
            apple->f = false;
            return false;
        }
        psnode = psnode->front;
    }
    return true;
}

void draw_food(SSD1306_t *dev, struct Food *apple, struct Snake *psnake) {
    char *food = "@";

    if (apple->f) {
        ssd1306_display_text_box1(dev, apple->y, apple->x, food, 1, 1, false, 0);
        return;
    }
    do {
        apple->x = (random() % (128 / 8)) * 8;
        apple->y = (random() % 8);
    } while (!food_is_here(psnake, apple));

    apple->f = true;
    ssd1306_display_text_box1(dev, apple->y, apple->x, food, 1, 1, false, 0);
}

void app_main(void) {
    srandom((unsigned) esp_random());
    SSD1306_t dev = {0};
    i2c_master_init(&dev, 21, 22, -1);
    ssd1306_init(&dev, 128, 64);
    ssd1306_clear_screen(&dev, 0);
    JoystickHandle joystick = joystick_conf();

    struct Food apple = { .f = false};
    struct Snake *psnake = initsnake();
    if (!psnake || !psnake->head) {
        ESP_LOGI("SNAKE", "Can't initialize snake head");
        return;
    }

    while (1) {
        joystick_moving(psnake, joystick);

        struct SnakeNode *psnode = GetNextNode(psnake);
        int old_tail_x = GetTail(psnake)->x;
        int old_tail_y = GetTail(psnake)->y;
        move_snake(psnake);
        draw_snake(&dev, psnake, old_tail_y, old_tail_x);

        if (psnake->head->x == apple.x && psnake->head->y == apple.y) {
            apple.f = false;
            // grow_snake(psnake) - на будущее
        }
        printf("CHECK head: x=%d y=%d | apple: x=%d y=%d | apple.f=%d\n",
                psnake->head->x, psnake->head->y, apple.x, apple.y, apple.f);

        if (psnake->head->x == apple.x && psnake->head->y == apple.y) {
            printf("EAT!\n");
            apple.f = false;
        }


        if (!apple.f) {
            draw_food(&dev, &apple, psnake);
        }
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
