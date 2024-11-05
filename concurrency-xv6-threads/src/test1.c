#include "types.h"
#include "date.h"
#include "fcntl.h"
#include "fs.h"
#include "stat.h"
#include "types.h"
#include "x86.h"
#include "user.h"

#define NULL (void *)(0)

void name_print(void *arg1, void *arg2)
{
  printf(1, "test1: in the child thread with pid %d\n", getpid());
  printf(1, "test1: my name is Raghad Noser\n");
  exit();
}
void echo(void *argc, void *string)
{
  printf(1, "test1: in the a child thread with pid %d\n", getpid());
  for (int i = *(int *)argc; i>0 ; i--) {
    printf(1,"%s\n", (char *)string);
  }
  exit();
}

int main(int argc, char *argv[])
{
  printf(1, "test1: in the main thread pid %d\n", getpid());
  int i = 3;
  char *str = "Raghad";
  int th1_create = thread_create(echo, (void *)&i, (void *)str);
  printf(1, "test1: created a child thread with pid %d\n", th1_create);

  int th2_create = thread_create(name_print, NULL, NULL);
  printf(1, "test1: created a child thread with pid %d\n", th2_create);


  int th_join1 = thread_join();
  printf(1, "test1: joined a child thread with pid %d\n", th_join1);
  int th_join2 = thread_join();
  printf(1, "test1: joined a child thread with pid %d\n", th_join2);
  exit();
}
