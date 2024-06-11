#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define FACTOR 2
#define ROBOT_NUM 10
#define BERTH_NUM 10
#define BOAT_NUM 5
#define N 210
#define n 200
#define MAX_QUEUE_SIZE 41000
const long long INF = 2147483637LL;
const int dx[] = {0, 0, -1, 1};
const int dy[] = {1, -1, 0, 0};

struct Point {
    int x;
    int y;
};

struct QueueNode {
    struct Point data;
};

struct Queue {
    struct QueueNode items[MAX_QUEUE_SIZE];
    int front;
    int rear;
};

struct Item {
    struct Point pos;
    int startFrame;
    int val;
    struct Robot* rob;  // 哪个机器人持有这个item
    struct Item* next;
};

struct Robot {
    int status;            // 机器人状态
    int goods;             // 是否携带货物
    struct Point prePos;   // 上一帧的位置
    struct Point nowPos;   // 这一帧的位置
    struct Point nextPos;  // 下一帧的位置
    int waitTimes;         // 在同一个位置呆了多久了，用来预测碰撞
    struct Item* item;     // 目标item
    int itemValue;         // 持有的item的价值
};

struct Berth {
    struct Point pos;
    int transportTime;  // 运输时间
    int lodaingSpeed;   // 装货速度
    int items;          // 货物数量
    int itemsValue;     // 货物价值
    int choosed;        // 是否已经被某个船选中
    int id;             // 泊位id
};

struct Boat {
    int pos;       // 在哪个泊位上
    int status;    // 状态，运输、装载、等待
    int items;     // 货物数量
    int capacity;  // 容量
};

int isEmpty(struct Queue* queue);
void enqueue(struct Queue* queue, struct Point pt);
struct Point dequeue(struct Queue* queue);

struct List* createList();
void listAdd(struct List* list, struct Item item);

void init();
int input();

// bfs搜索最短路径
void bfs(struct Point start, long long dist[N][N]);
int handleCrash(int robotId);
int getNextDir(int robotId, int nowFrame);
void handleRobot(int robotId, int nowFrame);
void handleBoat(int boatId, int nowFrame);
int getWhichBerth(struct Point pos);
void freshItem(int nowFrame);

char map[N][N];
struct Robot robot[ROBOT_NUM];
struct Berth berth[BERTH_NUM];
struct Boat boat[BOAT_NUM];
long long berthDist[BERTH_NUM][N][N];
struct Item* items;
#endif  // __MAIN_H__