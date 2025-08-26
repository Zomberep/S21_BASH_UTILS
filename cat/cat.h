#ifndef CAT_H

#include <dirent.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CAT_H
#define ERROR -1

typedef unsigned char mini_int;

typedef struct {
  mini_int n;
  mini_int b;
  mini_int s;
  mini_int t;
  mini_int e;
  mini_int v;
} Flags;

void getting_flags(int, char*[], Flags*, mini_int*);
mini_int is_dir(char*);
char** getting_files(int, char*[], char**, int*);
void cat(Flags*, char**, int);
void case_n(Flags*, unsigned long long*, FILE*, mini_int*);
void flag_v(int);
mini_int check(FILE*);

#endif