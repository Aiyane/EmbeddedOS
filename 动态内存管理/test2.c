/*
姓名: 张志强
班级: 软件zy1602
学号: 0121609361627
实验2: 动态内存管理
test2.c
*/
#include <stdio.h>
#include <stdlib.h> // malloc函数
#include <unistd.h> // signal函数和alarm函数
#include <signal.h> // 信号量SIGALRM
#include "test2.h"  // 引用的头文件

char a[900] = {0};                                                                   // 定义堆空间, 初始化0
uint8_t malloc_sizes[14] = {18, 46, 16, 35, 13, 38, 22, 32, 25, 43, 20, 39, 11, 41}; // 需要分配的内存大小
uint8_t n = 0;                                                                       // 分配第n个
malloced_memblk mlc_blk[14];                                                         // 得到的空闲的指针
partition_t *p0;                                                                     // 指向需要使用的分区
partition_t *p1;                                                                     // 分区1
partition_t *p2;                                                                     // 分区2

void putHex() {
  FILE *pFout = NULL;
  char buf[60];
  pFout = fopen("log.txt", "a");

  if (pFout == NULL) {
    printf("打开文件失败\n");
    return;
  }

  for (int i = 0; i < 30; i++) {
    for (int j = 0; j < 30; j++) {               // 每30字节一行
      sprintf(&buf[2 * j], "%x", a[i * 30 + j]); // 以16进制打印输出到文件
      sprintf(&buf[2 * j + 1], "%c", ' ');       // 以空格分开
    }
    fputs(buf, pFout);
    fputc('\n', pFout);
  }
  fputc('\n', pFout);
  fclose(pFout);
}

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
  printf("初始化完成\n");
  putHex();
}

void *OS_malloc(partition_t *pt) {
  memblk_t *mem_blk = NULL;
  if (pt->ptFreeQ != NULL) {
    mem_blk = pt->ptFreeQ;                     // 取到空闲块链表头
    pt->ptFreeQ = pt->ptFreeQ->next;           // 替换空闲块链表头
    pt->blk_num--;                             // 空闲块数量减1
    printf("分配1次\n");                        // 打印分配信息
    return (void *)mem_blk + sizeof(memblk_t); // 返回去除头部的空闲块地址
  }
  else
    printf("内存不足!\n");
}

void free_mem(uint8_t size, void *chkMem) {
  /*=====================还原内存数据=====================*/
  char *tem_point = chkMem; // 指针重新赋给char指针类型
  while (size) {            // 将获得的内存的内容全部改成序号 
    *tem_point = 0;
    tem_point++;
    size--;
  }
  /*=====================还原内存数据=====================*/
}

void OS_free(void *chkMem) {
  memblk_t *mem_blk = NULL;                                  // 释放空闲块
  mem_blk = (memblk_t *)((void *)chkMem - sizeof(memblk_t)); // 重新取到空闲块, 要减去头部大小, 指回头首
  mem_blk->next = mem_blk->pt->ptFreeQ;                      // 指向空闲块链表头
  mem_blk->pt->ptFreeQ = mem_blk;                            // 替换空闲块链表头
  mem_blk->pt->blk_num++;                                    // 空闲块数量加1
  putHex();                                                  // 打印内容到文件
}

void handle_malloc() {
  uint8_t size = malloc_sizes[n]; // 得到需要分配多少
  printf("需要%d大小\n", size);
  if (size < 14)
    p0 = p1;
  else
    p0 = p2;
  void *head = OS_malloc(p0);         // 已分配的头指针
  uint8_t getted_size = p0->blk_size; // 已分配的块大小
  while (size >= getted_size) {       // 如果大小不够, 继续分配 
    OS_malloc(p0);
    getted_size += p0->blk_size;
  }

  mlc_blk[n].point = head;       // 赋头指针
  mlc_blk[n].size = getted_size; // 赋内存大小
  mlc_blk[n].left_time = 3;      // 离回收还剩几秒
  n++;                           // 分配下一个目标
  if (n < 14)                    // 需要分配的未完成, 则更新时钟
    alarm(2);
  printf("分配完成!\n");

  /*==================修改内存数据====================*/
  char *tem_point = head; // 指针重新赋给char指针类型
  while (getted_size) {   // 将获得的内存的内容全部改成序号
    *tem_point = n;
    tem_point++;
    getted_size--;
  }
  /*==================修改内存数据====================*/
  putHex(); // 打印内容到文件
}

int main() {
  p1 = (partition_t *)malloc(sizeof(partition_t));  // 初始化分区1
  p2 = (partition_t *)malloc(sizeof(partition_t));  // 初始化分区2
  p1->blk_num = 10;                                 // 分区1块数量
  p1->blk_size = (uint8_t)(30 * sizeof(char) - 16); // 分区1块大小
  p1->ptFreeQ = (memblk_t *)a;                      // 分区1空闲块链表头指针
  p2->blk_num = 10;                                 // 分区2块数量
  p2->blk_size = (uint8_t)(60 * sizeof(char) - 16); // 分区2块大小
  p2->ptFreeQ = (memblk_t *)(&a[300]);              // 分区2空闲块链表头指针
  memSFL_partition_init(p1);                        // 初始化分区1
  memSFL_partition_init(p2);                        // 初始化分区2
  signal(SIGALRM, handle_malloc);                   // 绑定分配函数
  alarm(2);                                         // 每两秒触发分配函数

  while (1) {
    for (uint8_t i = 0; i < n + 1; i++) {                // 检测已分配的前n个
      if (mlc_blk[i].left_time == 0 && mlc_blk[i].size) {// 离被回收的时间为0时, 并且没被回收
        free_mem(mlc_blk[i].size, mlc_blk[i].point);     // 释放前还原内存内容
        OS_free(mlc_blk[i].point);                       // 释放内存
        printf("释放完%d大小的内存\n", mlc_blk[i].size);    // 打印释放信息
        mlc_blk[i].size = 0;                             // 被分配大小归0
        if (i == 13)                                     // 释放完, 退出
          return 0;
      }
      else
        mlc_blk[i].left_time--; // 离被回收的时间减1
    }                           // 经过1秒
    sleep(1);
  }
}
