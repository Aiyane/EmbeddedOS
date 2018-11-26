/*
姓名: 张志强
班级: 软件zy1602
学号: 0121609361627
实验3: 线程间通信 
test3.c
*/
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include "test3.h" // 引用的头文件

char a[600] = {0}; // 定义堆空间, 初始化0
partition_t *p1;   // 分区1

void memSFL_partition_init(partition_t *pt) {
  uint8_t i;
  memblk_t *blk;

  for (i = 0, blk = pt->ptFreeQ; i < pt->blk_num; i++, blk = blk->next) {
    blk->next = (void *)blk + pt->blk_size + sizeof(memblk_t); // 指向下一个内存头
    blk->pt = pt;                                              // 指向所属分区
    if (i == (pt->blk_num - 1)) {                              // 最后一块赋NULL
      blk->next = NULL;
      blk->pt = NULL;
    }
  }
  printf("初始化内存分区完成\n");
}

void *OS_malloc(partition_t *pt) {
  memblk_t *mem_blk = NULL;
  if (pt->ptFreeQ != NULL) {
    mem_blk = pt->ptFreeQ;                     // 取到空闲块链表头
    pt->ptFreeQ = pt->ptFreeQ->next;           // 替换空闲块链表头
    pt->blk_num--;                             // 空闲块数量减1
    printf("分配1次内存\n");                   // 打印分配信息
    return (void *)mem_blk + sizeof(memblk_t); // 返回去除头部的空闲块地址
  }
  else
    printf("内存不足!\n");
}

void OS_free(void *chkMem) {
  memblk_t *mem_blk = NULL;                                  // 释放空闲块
  mem_blk = (memblk_t *)((void *)chkMem - sizeof(memblk_t)); // 重新取到空闲块, 要减去头部大小, 指回头首
  mem_blk->next = mem_blk->pt->ptFreeQ;                      // 指向空闲块链表头
  mem_blk->pt->ptFreeQ = mem_blk;                            // 替换空闲块链表头
  mem_blk->pt->blk_num++;                                    // 空闲块数量加1
  printf("释放内存\n");
}

typedef struct Node {
  char msg[10];      // 节点内容
  struct Node *next; // 下一个节点
  struct Node *last; // 上一个节点
} Node;

typedef struct Queue {
  Node *front; // 队列头节点指针
  Node *rear;  // 队列尾节点指针
} Queue;

Queue *queue;

Queue *initQueue() {
  Queue *queue = (Queue *)malloc(sizeof(Queue));
  if (queue == NULL)
    return NULL;                     // 如果创建不成功, 返回空
  queue->front = queue->rear = NULL; // 初始化头尾指针一致
  return queue;
}

Node *pop(Queue *queue) { // 出队列
  if (queue->rear == NULL)
    return NULL;
  Node *node = queue->rear;
  queue->rear = node->last;
  return node;
}

void push(Queue *queue, Node *node) { // 入队列
  if (queue->front == NULL) {
    queue->front = node;
    queue->rear = node;
  }
  else {
    Node *tem_font = queue->front;
    queue->front = node;
    node->next = tem_font;
    tem_font->last = node;
  }
}

void *thread1(void) {
  int i;
  char buf[10];
  for (i = 0; i < 10; i++) {
    sprintf(buf, "msg_%d\n", i + 1);
    Node *node = (Node *)OS_malloc(p1); // 给节点分配内存
    for (int j = 0; j < 10; j++)        // 将需要的信息放入内存
      node->msg[j] = buf[j];
    push(queue, node);                       // 入队列
    printf("Push msg_%d in queue\n", i + 1); // 打印信息
    sleep(2);                                // 休眠两秒
  }
}

int *thread2(void) {
  int i;
  while (1) {
    sleep(3);
    Node *node = pop(queue); // 出队列
    if (node != NULL) {
      printf("%s", node->msg); // 打印节点信息
      OS_free(node);           // 释放节点内存
    }
    if (node->msg[4] == '1' && node->msg[5] == '0')
      return 0;
  }
}

int main(int argc, const char *argv[]) {
  p1 = (partition_t *)malloc(sizeof(partition_t));  // 初始化分区1
  p1->blk_num = 10;                                 // 分区1块数量
  p1->blk_size = (uint8_t)(60 * sizeof(char) - 16); // 分区1块大小
  p1->ptFreeQ = (memblk_t *)a;                      // 分区1空闲块链表头指针
  memSFL_partition_init(p1);                        // 初始化分区1

  int ret = 0;
  pthread_t id1, id2;
  queue = initQueue();
  if (queue == NULL) {
    printf("Create queue error!\n");
    return 1;
  }

  ret = pthread_create(&id1, NULL, (void *)thread1, NULL);
  if (ret) {
    printf("Create pthread error!\n");
    return 1;
  }

  ret = pthread_create(&id2, NULL, (void *)thread2, NULL);
  if (ret) {
    printf("Create pthread error!\n");
    return 1;
  }

  pthread_join(id1, NULL);
  pthread_join(id2, NULL);

  return 0;
}
