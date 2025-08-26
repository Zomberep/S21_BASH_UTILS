#include "grep.h"

int main(int argc, char* argv[]) {
  if (argc == 1) return 0;
  Flags flags = {0};    // for saving flags
  mini_int error = 0;   // save errors
  unsigned file_c = 0;  // number of files
  char **patterns = NULL, **files = NULL, **f_files = NULL;
  counting_option(argc, argv, &flags.e, "-e");  // we know the number of "e"
  counting_option(argc, argv, &flags.f, "-f");  // we know the number of "f"
  if (!flags.e) {
    patterns = (char**)malloc(sizeof(char*));
    flags.e = 0;
  } else
    patterns = (char**)malloc(sizeof(char*) * flags.e);
  if (flags.f) {
    f_files = (char**)malloc(sizeof(char*) * flags.f);
    error = (f_files == NULL);
  }

  error = (patterns == NULL);  // memory allocation error
  if (!error) {
    getting_args(argc, argv, &flags, &error, patterns, f_files);
    error = (error == 2);  // incorrect option error
    if (!error) {
      if (flags.f)
        patterns =
            getting_patterns_from_files(&flags, patterns, f_files, &error);
      if (flags.c || flags.l) {
        flags.n = 0;
        flags.o = 0;
      }
      if (flags.l) flags.c = 0;
      if (!error && (!flags.o || !flags.v)) {
        files = getting_files(argc, argv, flags.e, &file_c, patterns);
        flags.e += (flags.e == 0);
        grep(files, file_c, &flags, patterns, &error);
      }
    }
  }
  if (error != 3 && flags.f) {
    for (unsigned i = flags.f - 1; i < flags.e; ++i) free(patterns[i]);
  }
  if (patterns != NULL) free(patterns);
  if (f_files != NULL) free(f_files);

  return 0;
}

mini_int is_dir(char* file_name) {
  DIR* dir = opendir(file_name);
  mini_int flag = (dir != NULL);
  if (dir != NULL) closedir(dir);
  return flag;
}

char** getting_patterns_from_files(Flags* flags, char** patterns,
                                   char** f_files, mini_int* error) {
  unsigned j = 0, count = (flags->e == 0) ? 0 : flags->e, first_occur = 0;
  while (!(*error) && j < flags->f) {
    FILE* file = fopen(f_files[j++], "r");
    if (file == NULL) {
      printf("grep: %s: No such file or directory\n", f_files[j - 1]);
      *error = 3;
    } else if (is_dir(f_files[j - 1])) {
      printf("grep: %s: Is a directory\n", f_files[j - 1]);
      if (file != NULL) fclose(file);
      *error = 3;
    } else {
      mini_int file_end = 0;
      char* str = NULL;
      while (!(*error) && (!file_end)) {
        str = getting_str(file, &file_end, error);
        if (!(*error) && (str[0] != '\0' || !file_end) &&
            !uniq(str, patterns, count)) {
          if (!first_occur) first_occur = count + 1;
          if (count < flags->e)
            patterns[count++] = str;
          else {
            *error = ((patterns = realloc(
                           patterns, sizeof(char*) * (count + 5))) == NULL);
            flags->e += 5;
            if (!(*error)) patterns[count++] = str;
          }
        } else
          free(str);
      }
      fclose(file);
    }
  }
  flags->f = first_occur;
  flags->e = count;
  return patterns;
}

void imm_output(Flags* flags, char* str, char* file, unsigned num,
                unsigned file_c) {
  if (file_c > 1 && !flags->h) printf("%s:", file);
  if (flags->n) printf("%u:", num);
  puts(str);
}

void end_output(Flags* flags, unsigned file_c, char* files[], unsigned* count) {
  for (unsigned i = 0; (i < file_c); ++i)
    if (flags->l && (count[i] != UINT_MAX) && (count[i]))
      printf("%s\n", files[i]);
    else if (flags->c && (count[i] != UINT_MAX)) {
      if (file_c > 1 && !flags->h) printf("%s:", files[i]);
      printf("%u\n", count[i]);
    }
}

void comp_patterns(regex_t* reg, char* patterns[], unsigned count,
                   mini_int* error, mini_int flags_i) {
  if (flags_i) {
    for (unsigned i = 0; (!(*error)) && (i < count); ++i)
      *error = (regcomp(reg + i, patterns[i], REG_ICASE) != 0);
  } else
    for (unsigned i = 0; (!(*error)) && (i < count); ++i)
      *error = (regcomp(reg + i, patterns[i], 0) != 0);
}

char* getting_str(FILE* file, mini_int* file_end, mini_int* error) {
  unsigned k = 0, max_len = start_len;
  int c;
  char* str = (char*)malloc(sizeof(char) * start_len);
  if (str == NULL) *error = 1;

  while (!(*error)) {
    c = fgetc(file);
    if (c == '\n' || c == EOF) {
      if (c == EOF) *file_end = 1;
      str[k++] = '\0';
      break;
    } else {
      str[k++] = c;
      if (k == max_len) {
        *error = ((str = realloc(str, sizeof(char) * (max_len + add))) == NULL);
        max_len += add;
      }
    }
  }
  return str;
}

void output_o(Flags* flags, char* file, unsigned file_c, unsigned num,
              char* str, regoff_t start, regoff_t end) {
  if (file_c > 1 && !flags->h) printf("%s:", file);
  if (flags->n) printf("%u:", num);
  for (regoff_t i = start; i < end; ++i) putchar(str[i]);
  putchar('\n');
}

void finding_patterns_o(regex_t* reg, char* p, regmatch_t* occur, int* pos_1,
                        int* pos_2, unsigned count) {
  for (unsigned i = 0; i < count; ++i)
    if (!regexec(&reg[i], p, 1, occur, 0)) {
      if ((occur->rm_so < *pos_1) ||
          (occur->rm_so == *pos_1 && occur->rm_eo > *pos_2)) {
        *pos_1 = occur->rm_so;
        *pos_2 = occur->rm_eo;
      }
    }
}

void processing_o(FILE* file, Flags* flags, mini_int* error, regex_t* reg,
                  unsigned file_c, char* file_name) {
  char* str = NULL;
  unsigned num = 1;
  mini_int file_end = 0;
  regmatch_t occur[1];

  while (!(*error)) {
    str = getting_str(file, &file_end, error);
    if (!(*error) && (str[0] != '\0' || !file_end)) {
      char* p = str;
      int pos_1, pos_2;
      do {
        pos_2 = (pos_1 = INT_MAX);
        finding_patterns_o(reg, p, occur, &pos_1, &pos_2, flags->e);
        if (pos_1 != INT_MAX) {
          output_o(flags, file_name, file_c, num, p, pos_1, pos_2);
          p += pos_2;
        }
      } while (pos_1 != INT_MAX);
    }
    ++num;
    if (str != NULL) free(str);
    if (file_end == 1) break;
  }
}

void finding_patterns(regex_t* reg, Flags* flags, char* str, unsigned* count,
                      char* file_name, unsigned num, unsigned file_c) {
  unsigned match = 0;
  for (unsigned i = 0; i < flags->e; ++i) {
    match += (regexec(reg + i, str, 0, NULL, 0) != 0);
  }
  if ((flags->v && (match == flags->e)) || (!flags->v && (match != flags->e))) {
    *count += 1;
    if (!flags->c && !flags->l) imm_output(flags, str, file_name, num, file_c);
  }
}

void processing_dir(FILE* file, char* file_name, mini_int flag_s) {
  if (!flag_s) printf("grep: %s: Is a directory\n", file_name);
  if (file != NULL) fclose(file);
}

void grep(char** files, unsigned file_c, Flags* flags, char** patterns,
          mini_int* error) {
  unsigned j = 0;

  regex_t* reg = (regex_t*)malloc(sizeof(regex_t) * flags->e);
  unsigned* count = (unsigned*)malloc(sizeof(unsigned) * file_c);
  if (reg == NULL || count == NULL) *error = 1;
  comp_patterns(reg, patterns, flags->e, error, flags->i);

  while (!(*error) && j < file_c) {
    FILE* file = fopen(files[j], "r");
    count[j] = 0;
    if (file == NULL) {
      count[j++] = UINT_MAX;
      if (!flags->s)
        printf("grep: %s: No such file or directory\n", files[j - 1]);
      continue;
    } else if (is_dir(files[j])) {
      processing_dir(file, files[j], flags->s);
      j++;
      continue;
    }
    if (flags->o) {
      processing_o(file, flags, error, reg, file_c, files[j++]);
      fclose(file);
      continue;
    }

    char* str = NULL;
    unsigned num = 1;
    mini_int file_end = 0;

    while (!(*error)) {
      str = getting_str(file, &file_end, error);
      if (!(*error) && (str[0] != '\0' || !file_end))
        finding_patterns(reg, flags, str, &count[j], files[j], num, file_c);
      num++;
      if (str != NULL) free(str);
      if (file_end == 1 || (flags->l && count[j])) break;
    }
    fclose(file);
    j++;
  }

  if (!(*error) && (flags->l || flags->c))
    end_output(flags, file_c, files, count);
  if (reg != NULL) {
    for (unsigned i = 0; i < flags->e; ++i) regfree(reg + i);
    free(reg);
  }
  if (count != NULL) free(count);
}

char** getting_files(int argc, char* argv[], unsigned count_e, unsigned* file_c,
                     char** patterns) {
  unsigned count = 0, k_e = 0, k_f = 0;
  for (int i = 1; (i < argc) && (!count); ++i) {
    if (argv[i][0] != '-' &&
        ((k_e % 2 == 0 && k_e > 0) || (k_f % 2 == 0 && k_f > 0)) &&
        (i == (argc - 1) || argv[i + 1][0] != '-')) {
      count = i;
    } else if (argv[i][0] != '-' && strcmp(argv[i - 1], "-e") &&
               strcmp(argv[i - 1], "-f")) {
      if (!count_e) {
        *patterns = argv[i];
        count = i + 1;
      } else {
        count = i;
      }
    }
    k_e = !strcmp(argv[i], "-e") ? k_e + 1 : 0;
    k_f = !strcmp(argv[i], "-f") ? k_f + 1 : 0;
  }
  *file_c = argc - count;
  return argv + count;
}

mini_int uniq(char* curr_obj, char** objects, unsigned count) {
  mini_int flag = 0;
  for (unsigned i = 0; i < count; ++i)
    flag += (strcmp(curr_obj, objects[i]) == 0);
  return flag;
}

void getting_args(int argc, char* argv[], Flags* flags, mini_int* error,
                  char** patterns, char** f_files) {
  int c = 0;
  unsigned patt_c = 0, f_file_c = 0;

  while (!(*error)) {
    c = getopt(argc, argv, "e:nvclishof:");
    *error = (c == -1);

    switch (c) {
      case 'e':
        if (!uniq(optarg, patterns, patt_c) && (patt_c < flags->e))
          patterns[patt_c++] = optarg;
        break;
      case 'n':
        flags->n = 1;
        break;
      case 'c':
        flags->c = 1;
        break;
      case 'l':
        flags->l = 1;
        break;
      case 'i':
        flags->i = 1;
        break;
      case 'v':
        flags->v = 1;
        break;
      case 'h':
        flags->h = 1;
        break;
      case 's':
        flags->s = 1;
        break;
      case 'o':
        flags->o = 1;
        break;
      case 'f':
        if (!uniq(optarg, f_files, f_file_c) && (f_file_c < flags->f))
          f_files[f_file_c++] = optarg;
        break;
      case end:
        break;
      default:
        *error = 2;
    }
  }
  flags->e = (flags->e != 0) ? patt_c : 0;
  flags->f = (flags->f != 0) ? f_file_c : 0;
}

void counting_option(int argc, char* argv[], unsigned* count_opt,
                     char* option) {
  unsigned count = 0;
  for (int i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], option)) {
      count += 1;
      if (count % 2 != 0) *count_opt += 1;
    } else {
      count = 0;
    }
  }
}