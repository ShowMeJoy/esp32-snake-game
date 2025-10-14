#include "snake.h"
#include <stdlib.h>

// // Инициализация структуры змейки
// struct Snake* initsnake() {
//     struct Snake *psnake = malloc(sizeof(struct Snake));
//     if (!psnake) return NULL;
//     psnake->dir = DIR_LEFT;
//     psnake->length = 4;

//     // Создаем 4 узла + циклический список
//     struct SnakeNode *n1 = malloc(sizeof(struct SnakeNode));
//     struct SnakeNode *n2 = malloc(sizeof(struct SnakeNode));
//     struct SnakeNode *n3 = malloc(sizeof(struct SnakeNode));
//     struct SnakeNode *n4 = malloc(sizeof(struct SnakeNode));
//     if (!n1 || !n2 || !n3 || !n4) {
//         free(n1); free(n2); free(n3); free(n4); free(psnake);
//         return NULL;
//     }

//     /* Кординаты появления змейки с учетом того, 
//      * что дисплей имеет разрешение 128 x 64.
//      * Если заменю дисплей на больший, стоит пересмотреть.
//      */
//     n1->y = 3; n1->x=32;
//     n2->y = 3; n2->x=40;
//     n3->y = 3; n3->x=48;
//     n4->y = 3; n4->x=56;

//     // Связь: голова = n1, tail = n4 -> head
//     n1->front = n2;
//     n2->front = n3;
//     n3->front = n4;
//     n4->front = n1;

//     psnake->head = n1;
//     return psnake;
// } 

// struct SnakeNode *newsnakenode(struct SnakeNode **ppsnode, int x, int y) {
//     *ppsnode = (struct SnakeNode *)malloc(sizeof(struct SnakeNode));
//     (*ppsnode)->y = y;
//     (*ppsnode)->x = x;
//     (*ppsnode)->front = NULL;

//     return *ppsnode;
// }

// /* 
//  * Движение змейки
//  * Структура TSnake представляет собой обратный связный список с соединенными головой и хвостом.
//  * Пример: [a]<-[b]<-[c]<-[d]    a - голова
//  *          |              ^     Когда змейка двигается, только голова указывает на d,
//  *          `--------------'     и координаты (y,x) d изменяются на позицию, куда движется голова.
//  */
// void movesnake(struct Snake *psnake) {
//     int hy = psnake->head->y;
//     int hx = psnake->head->x;

//     psnake->head = GetSnakeTail(psnake);

//     switch (psnake->dir) {
//         case DIR_UP:
//             psnake->head->x = hx; 
//             psnake->head->y = hy - 1;
//             break;

//         case DIR_DOWN:
//             psnake->head->x = hx;
//             psnake->head->y = hy + 1;
//             break;

//         case DIR_LEFT:
//             psnake->head->x = hx + 1;
//             psnake->head->y = hy;
//             break;

//         case DIR_RIGHT:
//             psnake->head->x = hx - 1;
//             psnake->head->y = hy;
//             break;

//         default:
//             break;
//     }
// }

// void destroysnake(struct Snake *psnake) {
//     struct SnakeNode *psnode = GetSnakeTail(psnake);
//     struct SnakeNode *ptmp = NULL;

//     for (int i = 0; i < psnake->length; ++i) {
//         ptmp = psnode;
//         psnode = psnode->front;
//         free(ptmp);
//     }

//     free(psnake);
//     psnake = NULL;
// }

