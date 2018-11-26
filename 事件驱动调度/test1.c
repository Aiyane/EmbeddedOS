/*
姓名: 张志强
班级: 软件zy1602
学号: 0121609361627
实验1: 事件驱动调度
test1.c
*/
#include <stdio.h>
#include <unistd.h> // signal函数和alarm函数
#include <signal.h> // 信号量SIGALRM

typedef unsigned int uint16_t; // 标志位类型
typedef unsigned char uint8_t; // 代替int型

uint16_t task_flags = 0; // 任务标志位
uint8_t n = 1;           // 第n个激活的事件

void eventPost() {                   // 中断函数
  printf("E%d is actived\n", 7 - n); // 执行目标操作
  task_flags |= (0x01 << 7 - n);     // 激活当前事件
  n++;                               // 取下一个事件
  alarm(n);                          // 重新定义中断时间
}

int main() {
  uint16_t fb;                // 标志位
  signal(SIGALRM, eventPost); // 绑定中断函数
  alarm(1);                   // 定义初始中断时间
  sleep(1);                   // 激活初始中断函数
  while (1) {
    for (uint8_t i = 0, fb = 0x01; i < 16; i++, fb = fb << 1) {
      if ((task_flags & fb) != 0) {          // 选择已激活的事件
        int copy_i = i + 1;            
        while (--copy_i) {                   // 在while内执行 
          printf("Task_%d is running\n", i); // 执行事件内操作
          if (n > 6)                         // 六个事件完成
            return 0;                        // 退出
          usleep(600);                       // 休眠0.6秒
        }
        task_flags &= ~(0x01 << i);          // 清除标志位
      }
    }
  }
}
