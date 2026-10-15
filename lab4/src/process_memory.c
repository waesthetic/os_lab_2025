
/* Program to display address information about the process */
/* Adapted from Gray, J., program 1.4 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <mach-o/dyld.h>    /* _dyld_get_image_vmaddr_slide */
#include <mach-o/getsect.h> /* get_etext, get_edata, get_end */

/* Below is a macro definition */
#define SHW_ADR(ID, I) (printf("ID %s \t is at virtual address: %p\n", ID, (void *)&I))

char *cptr = "This message is output by the function showit()\n"; /* Static */
char buffer1[25];
void showit(char *p); /* Function prototype */

int main(void) {
  int i = 0; /* Automatic variable */

  /* ASLR shift: get_etext/get_edata/get_end return addresses without it */
  long slide = _dyld_get_image_vmaddr_slide(0);

  /* Printing addressing information */
  printf("\nAddress etext: %lx \n", get_etext() + slide);
  printf("Address edata: %lx \n", get_edata() + slide);
  printf("Address end  : %lx \n", get_end() + slide);

  SHW_ADR("main", main);
  SHW_ADR("showit", showit);
  SHW_ADR("cptr", cptr);
  SHW_ADR("buffer1", buffer1);
  SHW_ADR("i", i);
  strcpy(buffer1, "A demonstration\n");   /* Library function */
  write(1, buffer1, strlen(buffer1) + 1); /* System call */
  showit(cptr);

  return 0;
} /* end of main function */

/* A function follows */
void showit(char *p) {
  char *buffer2;
  SHW_ADR("buffer2", buffer2);
  if ((buffer2 = (char *)malloc((unsigned)(strlen(p) + 1))) != NULL) {
    printf("Alocated memory at %p\n", (void *)buffer2);
    strcpy(buffer2, p);    /* copy the string */
    printf("%s", buffer2); /* Didplay the string */
    free(buffer2);         /* Release location */
  } else {
    printf("Allocation error\n");
    exit(1);
  }
}
