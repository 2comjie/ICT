#include "main.h"

int isEmpty(struct Queue* queue) {
    return queue->front == queue->rear;
}

void enqueue(struct Queue* queue, struct Point pt) {
    queue->items[queue->rear].data = pt;
    queue->rear = (queue->rear + 1) % MAX_QUEUE_SIZE;
}

struct Point dequeue(struct Queue* queue) {
    struct Point pt = queue->items[queue->front].data;
    queue->front = (queue->front + 1) % MAX_QUEUE_SIZE;
    return pt;
}

void bfs(struct Point start, long long dist[N][N]) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            dist[i][j] = INF;

    static struct Queue q;
    q.front = 0;
    q.rear = 0;

    enqueue(&q, start);
    dist[start.x][start.y] = 0;

    while (!isEmpty(&q)) {
        struct Point cur = dequeue(&q);
        for (int i = 0; i < 4; i++) {
            int nx = cur.x + dx[i];
            int ny = cur.y + dy[i];
            if (nx >= 0 && nx < n && ny >= 0 && ny < n && map[nx][ny] != '#' && map[nx][ny] != '*' && dist[nx][ny] == INF) {
                dist[nx][ny] = dist[cur.x][cur.y] + 1;
                struct Point p;
                p.x = nx;
                p.y = ny;
                enqueue(&q, p);
            }
        }
    }
}

int handleCrash(int robotId) {
    // static unsigned int offset = 0;
    // offset = (offset + 3) % 4;
    int offset = rand() % 5;  // 仙法:左右横跳
    if (offset == 4) return -1;
    for (int i = 0; i < 4; ++i) {
        int dir = (i + offset) % 4;
        int nx = robot[robotId].nowPos.x + dx[dir];
        int ny = robot[robotId].nowPos.y + dy[dir];
        int flag = 1;
        if (nx >= 0 && nx < n && ny >= 0 &&  //
            ny < n && map[nx][ny] != '*' &&  //
            map[nx][ny] != '#') {
            for (int j = 0; j < ROBOT_NUM; ++j) {
                if (j == robotId) continue;
                if (robot[j].nextPos.x == nx && robot[j].nextPos.y == ny) {
                    flag = 0;
                    break;
                }
            }
            if (flag) {
                return dir;
            }
        }
    }
    return -1;
}

int getNextDir(int robotId, int nowFrame) {
    static long long dist[N][N];
    bfs(robot[robotId].nowPos, dist);

    // 可能发生碰撞了
    if (robot[robotId].waitTimes >= 3) {
        int dir = handleCrash(robotId);
        if (dir != -1) return dir;
    }

    struct Point targetPos;  // 目标位置
    //  携带货物
    if (robot[robotId].goods != 0) {
        // 寻找最近的港口
        long long minDist = INF;  // 离目标最近的距离
        for (int i = 0; i < BERTH_NUM; ++i) {
            for (int j = 0; j < 3; ++j) {
                for (int k = 0; k < 3; ++k) {
                    if (minDist > dist[berth[i].pos.x + j][berth[i].pos.y + k]) {
                        minDist = dist[berth[i].pos.x + j][berth[i].pos.y + k];
                        targetPos.x = berth[i].pos.x + j;
                        targetPos.y = berth[i].pos.y + k;
                    }
                }
            }
        }
        if (minDist == INF) {
            return -1;
        }
    } else if (robot[robotId].item == NULL) {
        long long maxValue = -INF;
        struct Item* tmp = NULL;
        struct Item* item = items->next;
        while (item != NULL) {
            if (item->rob != NULL) {
                item = item->next;
                continue;
            }
            int restTime = nowFrame - item->startFrame;
            int distance = dist[item->pos.x][item->pos.y];
            if (restTime < distance) {
                item = item->next;
                continue;
            }
            long long minDist = INF;
            for (int i = 0; i < BERTH_NUM; ++i) {
                if (minDist > berthDist[i][item->pos.x][item->pos.y]) {
                    minDist = berthDist[i][item->pos.x][item->pos.y];
                }
            }

            long long value = item->val * FACTOR - distance - minDist;
            if (value > maxValue) {
                tmp = item;
                maxValue = value;
            }
            item = item->next;
        }
        if (maxValue == -INF || tmp == NULL) {
            return -1;
        }
        tmp->rob = &robot[robotId];
        robot[robotId].item = tmp;
        targetPos.x = tmp->pos.x;
        targetPos.y = tmp->pos.y;
    } else if (robot[robotId].item != NULL) {
        targetPos = robot[robotId].item->pos;
        if (dist[targetPos.x][targetPos.y] == INF) {
            robot[robotId].item->rob = NULL;
            robot[robotId].item = NULL;
            long long maxValue = -INF;
            struct Item* tmp = NULL;
            struct Item* item = items->next;
            while (item != NULL) {
                if (item->rob != NULL) {
                    item = item->next;
                    continue;
                }
                int restTime = nowFrame - item->startFrame;
                int distance = dist[item->pos.x][item->pos.y];
                if (restTime < distance) {
                    item = item->next;
                    continue;
                }

                long long minDist = INF;
                for (int i = 0; i < BERTH_NUM; ++i) {
                    if (minDist > berthDist[i][item->pos.x][item->pos.y]) {
                        minDist = berthDist[i][item->pos.x][item->pos.y];
                    }
                }
                // 价值 = 货物价值*FACTOR-距离-货物到最近的码头的距离
                long long value = item->val * FACTOR - distance - minDist;
                if (value > maxValue) {
                    tmp = item;
                    maxValue = value;
                }
                item = item->next;
            }
            if (maxValue == -INF || tmp == NULL) {
                return -1;
            }
            tmp->rob = &robot[robotId];
            robot[robotId].item = tmp;
            targetPos.x = tmp->pos.x;
            targetPos.y = tmp->pos.y;
        }
    }

    // 到达目的地
    if (targetPos.x == robot[robotId].nowPos.x  //
        && targetPos.y == robot[robotId].nowPos.y) {
        return 4;
    }

    if (dist[targetPos.x][targetPos.y] == INF)
        return -1;

    int x = targetPos.x;
    int y = targetPos.y;
    while (dist[x][y] > 1) {
        for (int dir = 0; dir < 4; dir++) {
            int nx = x + dx[dir];
            int ny = y + dy[dir];
            if (nx >= 0 && nx < n && ny >= 0 && ny < n && dist[nx][ny] == dist[x][y] - 1) {
                x = nx;
                y = ny;
                break;
            }
        }
    }

    for (int i = 0; i < ROBOT_NUM; ++i) {
        if (i == robotId) continue;
        if (robot[i].nextPos.x == x && robot[i].nextPos.y == y) {
            return -1;
        }
    }

    for (int dir = 0; dir < 4; dir++) {
        int nx = x - dx[dir];
        int ny = y - dy[dir];
        if (nx == robot[robotId].nowPos.x && ny == robot[robotId].nowPos.y) {
            return dir;
        }
    }
    return -1;
}

void handleRobot(int robotId, int nowFrame) {
    if (robot[robotId].status == 0) {
        return;
    }
    // 寻找路径
    int dir = getNextDir(robotId, nowFrame);
    if (dir == 4) {
        // 到达目的地
        if (robot[robotId].goods) {  // 放下货物
            printf("pull %d\n", robotId);
            int berthId = getWhichBerth(robot[robotId].nowPos);
            if (berthId != -1) {
                berth[berthId].items++;
                berth[berthId].itemsValue += robot[robotId].itemValue;
            }
        } else {  // 拿起货物
            printf("get %d\n", robotId);
            robot[robotId].itemValue = robot[robotId].item->val;
            struct Item* pre = items;
            while (pre->next != NULL) {
                if (pre->next == robot[robotId].item) {
                    struct Item* it = pre->next;
                    pre->next = it->next;
                    free(it);
                    break;
                }
                pre = pre->next;
            }
            robot[robotId].item = NULL;
        }
        robot[robotId].nextPos.x = robot[robotId].nowPos.x;
        robot[robotId].nextPos.y = robot[robotId].nowPos.y;
        robot[robotId].waitTimes = 0;
    } else if (dir != -1) {
        printf("move %d %d\n", robotId, dir);
        robot[robotId].nextPos.x = robot[robotId].nowPos.x + dx[dir];
        robot[robotId].nextPos.y = robot[robotId].nowPos.y + dy[dir];
        robot[robotId].waitTimes = 0;
    } else {  // 没有移动，等待
        robot[robotId].nextPos.x = robot[robotId].nowPos.x;
        robot[robotId].nextPos.y = robot[robotId].nowPos.y;
        robot[robotId].waitTimes++;
    }
}

void handleBoat(int boatId, int nowFrame) {
    if (boat[boatId].status == 0) return;                      // 移动 运输中
    if (boat[boatId].status == 2) return;                      // 泊位外面等待
    if (boat[boatId].status == 1 && boat[boatId].pos == -1) {  // 运输完成
        // 找货物最多的码头
        int ber = -1;
        long long maxValue = -1;
        for (int i = 0; i < BERTH_NUM; ++i) {
            if (berth[i].choosed == 0) {
                if (berth[i].itemsValue > maxValue) {
                    ber = i;
                    maxValue = berth[i].itemsValue;
                }
            }
        }
        if (ber != -1) {
            printf("ship %d %d\n", boatId, ber);
            berth[ber].choosed = 1;
        }
    } else if (boat[boatId].status == 1 && boat[boatId].pos != -1) {  // 在泊位上装货
        if (berth[boat[boatId].pos].items >= berth[boat[boatId].pos].lodaingSpeed) {
            berth[boat[boatId].pos].items -= berth[boat[boatId].pos].lodaingSpeed;
            boat[boatId].items += berth[boat[boatId].pos].items += berth[boat[boatId].pos].lodaingSpeed;
        } else {
            boat[boatId].items += berth[boat[boatId].pos].items;
            berth[boat[boatId].pos].items = 0;
        }
        if (boat[boatId].items >= boat[boatId].capacity || nowFrame >= 13000) {
            printf("go %d\n", boatId);
            berth[boat[boatId].pos].choosed = 0;
        } else if (berth[boat[boatId].pos].items = 0) {
            // 找货物最多的码头
            int ber = -1;
            long long maxValue = -1;
            for (int i = 0; i < BERTH_NUM; ++i) {
                if (berth[i].choosed == 0) {
                    if (berth[i].itemsValue > maxValue) {
                        ber = i;
                        maxValue = berth[i].itemsValue;
                    }
                }
            }
            if (ber != -1) {
                printf("ship %d %d\n", boatId, ber);
                berth[ber].choosed = 1;
            }
        }
    }
}

int getWhichBerth(struct Point pos) {
    int x = pos.x;
    int y = pos.y;
    for (int i = 0; i < BERTH_NUM; ++i) {
        if (x >= berth[i].pos.x && x <= berth[i].pos.x + 3  //
            && y >= berth[i].pos.y && y <= berth[i].pos.y + 3) {
            return i;
        }
    }
    return -1;
}

void freshItem(int nowFrame) {
    struct Item* pre = items;
    while (pre->next != NULL) {
        if (nowFrame - pre->next->startFrame > 970) {
            struct Item* it = pre->next;
            pre->next = it->next;
            if (it->rob != NULL)
                it->rob->item = NULL;
            free(it);
        } else {
            pre = pre->next;
        }
    }
}

void freshMap() {
    for (int i = 0; i < ROBOT_NUM; ++i) {
        map[robot[i].prePos.x][robot[i].prePos.y] = '.';
        map[robot[i].nowPos.x][robot[i].nowPos.y] = '#';
    }
}

void init() {
    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        scanf("%s", map[i]);
        for (int j = 0; j < n; ++j) {
            if (map[i][j] == 'A') {
                map[i][j] = '#';
            }
        }
    }

    for (int i = 0; i < BERTH_NUM; i++) {
        int idd;
        scanf("%d", &idd);
        scanf("%d%d%d%d", &berth[idd].pos.x, &berth[idd].pos.y, &berth[idd].transportTime, &berth[idd].lodaingSpeed);
        berth[idd].items = 0;
        berth[idd].choosed = 0;
        bfs(berth[idd].pos, berthDist[idd]);
    }
    int boatCapacity;
    scanf("%d", &boatCapacity);
    for (int i = 0; i < BOAT_NUM; ++i) {
        boat[i].capacity = boatCapacity;
        boat[i].items = 0;
    }
    for (int i = 0; i < ROBOT_NUM; ++i) {
        robot[i].waitTimes = 0;
        robot[i].item = NULL;
        robot[i].goods = 0;
        robot[i].status = 0;
        robot[i].prePos.x = 209;
        robot[i].prePos.y = 209;
    }

    items = (struct Item*)malloc(sizeof(struct Item));
    items->next = NULL;
    char okk[100];
    scanf("%s", okk);
    printf("OK\n");
    fflush(stdout);
}

int input() {
    int frame, money;
    scanf("%d%d", &frame, &money);
    int num;
    scanf("%d", &num);
    for (int i = 1; i <= num; i++) {
        struct Item* item = malloc(sizeof(struct Item));
        scanf("%d%d%d", &item->pos.x, &item->pos.y, &item->val);
        item->rob = NULL;
        item->startFrame = frame;
        item->next = items->next;
        items->next = item;
    }
    for (int i = 0; i < ROBOT_NUM; i++) {
        robot[i].prePos.x = robot[i].nowPos.x;
        robot[i].prePos.y = robot[i].nowPos.y;
        scanf("%d%d%d%d", &robot[i].goods, &robot[i].nowPos.x, &robot[i].nowPos.y, &robot[i].status);
    }
    for (int i = 0; i < BOAT_NUM; i++)
        scanf("%d%d\n", &boat[i].status, &boat[i].pos);
    char okk[100];
    scanf("%s", okk);
    return frame;
}

int main(void) {
    init();
    for (int i = 1; i <= 15000; i++) {
        int frame = input();
        freshItem(frame);
        freshMap();
        for (int robotId = 0; robotId < ROBOT_NUM; ++robotId) {
            handleRobot(robotId, frame);
        }
        for (int boatId = 0; boatId < BOAT_NUM; ++boatId) {
            handleBoat(boatId, frame);
        }
        puts("OK");
        fflush(stdout);
    }
    return 0;
}