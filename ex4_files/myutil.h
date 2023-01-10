#ifndef  _my_util_h
#define _my_util_h 

typedef struct {
   unsigned char red;
   unsigned char green;
   unsigned char blue;
} pixel;

typedef struct {
    int red;
    int green;
    int blue;
    // int num;
} pixel_sum;

int min(int a, int b);
int max(int a, int b);

int calcIndex(int i, int j, int n);


#endif

