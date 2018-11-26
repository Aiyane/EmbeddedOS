/*
姓名: 张志强
班级: 软件zy1602
学号: 0121609361627
实验2: 动态内存管理
test2.h
*/
#ifndef __TEST2_H__
#define __TEST2_H__
typedef unsigned char uint8_t;
typedef struct partition partition_t;

typedef struct memblk {
  struct memblk *next; // next指针, 用于链接空闲块
  partition_t *pt;     // pt指针, 指向所属的分区, 当一个块释放时, 将其释放至该分区中
} memblk_t;

struct partition {
  uint8_t blk_size;  // 分区中块大小
  uint8_t blk_num;   // 分区中块数量
  memblk_t *ptFreeQ; // 空闲块管理链表, 此链表链接所有空闲块
};

typedef struct malloced_memblk {
  void *point;       // 被分配内存头指针
  uint8_t size;      // 分配了多大的内存
  uint8_t left_time; // 离内存被回收还剩几秒
} malloced_memblk;

#endif