#ifndef GREP_H

#define GREP_H
#include <dirent.h>
#include <getopt.h>
#include <limits.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
  end = -1,
  start_len = 101,
  add = 50,
};

typedef unsigned char mini_int;

typedef struct {
  unsigned e;  // counter of e flags
  mini_int n;  // -c and -l kill this flag
  mini_int v;
  mini_int c;  // -l kill this flag
  mini_int l;  // the strongest flag
  mini_int i;
  mini_int h;  // no name files
  mini_int s;  // no errors about non-exist files
  mini_int o;  // -l and -c kill this flag, -v makes this flag stuck
  unsigned f;
} Flags;

void counting_option(int, char*[], unsigned*, char*);
mini_int is_dir(char*);
void processing_dir(FILE*, char*, mini_int);
char** getting_patterns_from_files(Flags*, char**, char**, mini_int*);
void getting_args(int, char*[], Flags*, mini_int*, char**, char**);
char** getting_files(int, char*[], unsigned, unsigned*, char**);
void grep(char**, unsigned, Flags*, char**, mini_int*);
void imm_output(Flags*, char*, char*, unsigned, unsigned);
void end_output(Flags*, unsigned, char*[], unsigned*);
void comp_patterns(regex_t*, char*[], unsigned, mini_int*, mini_int);
char* getting_str(FILE*, mini_int*, mini_int*);
void processing_o(FILE*, Flags*, mini_int*, regex_t*, unsigned, char*);
void output_o(Flags*, char*, unsigned, unsigned, char*, regoff_t, regoff_t);
mini_int uniq(char*, char**, unsigned);
void finding_patterns_o(regex_t*, char*, regmatch_t*, int*, int*, unsigned);
void finding_patterns(regex_t*, Flags*, char*, unsigned*, char*, unsigned,
                      unsigned);

#endif