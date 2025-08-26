#include "cat.h"

int main(int argc, char* argv[]) {
  mini_int err = 0;
  Flags flags = {0};
  char** files = NULL;
  int file_c = 0;

  getting_flags(argc, argv, &flags, &err);
  err = (err == 2);

  if (!err) {
    if (flags.b) flags.n = 0;
    files = getting_files(argc, argv, files, &file_c);
    cat(&flags, files, file_c);
  }
  return 0;
}

mini_int check(FILE* file) {
  char c = fgetc(file);
  fseek(file, -1, SEEK_CUR);
  return (c != EOF);
}

void cat(Flags* flags, char** files, int file_c) {
  unsigned long long counter = 1;
  mini_int last;
  int c, i = 0;
  while (i != file_c) {
    FILE* file = fopen(files[i++], "r");
    if (file == NULL) {
      printf("cat: %s: No such file or directory\n", files[i - 1]);
      continue;
    }
    if (is_dir(files[i - 1])) {
      printf("cat: %s: Is a directory\n", files[i - 1]);
      if (file != NULL) fclose(file);
      continue;
    }
    if (check(file) && (counter == 1) && (flags->b || flags->n))
      printf("%6llu\t", counter++);
    while ((c = fgetc(file)) != EOF) {
      if (c == '\n')
        case_n(flags, &counter, file, &last);
      else if (c == '\t')
        printf("%s", flags->t ? "^I" : "\t");
      else if (flags->v) {
        flag_v(c);
      } else
        putchar(c);
      last = (c == '\n') ? last + 1 : 0;
    }
    fclose(file);
  }
}

mini_int is_dir(char* file_name) {
  DIR* dir = opendir(file_name);
  mini_int flag = (dir != NULL);
  if (dir != NULL) closedir(dir);
  return flag;
}

void case_n(Flags* flags, unsigned long long* counter, FILE* file,
            mini_int* last) {
  if ((flags->s) && (flags->n)) {
    if (*last < 2) printf("%s\n%6llu\t", flags->e ? "$" : "", (*counter)++);
  } else if ((flags->s) && (flags->b)) {
    char next_c = fgetc(file);
    fseek(file, -1, SEEK_CUR);
    if (*last > 0) {
      if (next_c != '\n')
        printf("%s\n%6llu\t", flags->e ? "$" : "", (*counter)++);
    } else {
      if (next_c == '\n')
        printf("%s\n", flags->e ? "$" : "");
      else
        printf("%s\n%6llu\t", flags->e ? "$" : "", (*counter)++);
    }
  } else if (flags->s) {
    if (*last < 2) printf("%s\n", flags->e ? "$" : "");
  } else if (flags->b) {
    char next_c = fgetc(file);
    fseek(file, -1, SEEK_CUR);
    if (next_c == '\n')
      printf("%s\n", flags->e ? "$" : "");
    else
      printf("%s\n%6llu\t", flags->e ? "$" : "", (*counter)++);
  } else if (flags->n)
    printf("%s\n%6llu\t", flags->e ? "$" : "", (*counter)++);
  else
    printf("%s\n", flags->e ? "$" : "");
}

void flag_v(int c) {
  if (c < 32) {
    printf("^%c", c + 64);
  } else if (c > 127 && c < 160) {
    printf("M-^%c", c - 64);
  } else if (c >= 160) {
    printf("M-%c", c - 128);
  } else if (c == 127) {
    printf("^?");
  } else {
    printf("%c", c);
  }
}

char** getting_files(int argc, char* argv[], char** files, int* file_c) {
  int num = 0;
  for (int i = 1; (i < argc) && !(num); ++i)
    if (argv[i][0] != '-') num = i;
  if (num != 0) {
    files = argv + num;  // the first argument is ./a.out
    *file_c = argc - num;
  }
  return files;
}

void getting_flags(int argc, char* argv[], Flags* flags, mini_int* flag) {
  int option_index, par = 0;
  struct option long_options[] = {{"number-nonblank", 0, NULL, 'b'},
                                  {"number", 0, NULL, 'n'},
                                  {"squeeze-blank", 0, NULL, 's'}};
  while (!(*flag)) {
    par = getopt_long(argc, argv, "bnstevTE", long_options, &option_index);
    *flag = (par == -1);
    switch (par) {
      case 'n':
        flags->n = 1;
        break;
      case 'b':
        flags->b = 1;
        break;
      case 's':
        flags->s = 1;
        break;
      case 'T':
        flags->t = 1;
        break;
      case 't':
        flags->t = 1;
        flags->v = 1;
        break;
      case 'E':
        flags->e = 1;
        break;
      case 'e':
        flags->e = 1;
        flags->v = 1;
        break;
      case 'v':
        flags->v = 1;
        break;
      case ERROR:
        break;
      default:
        *flag = 2;
    }
  }
}