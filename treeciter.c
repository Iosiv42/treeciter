#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdbool.h>
#include <wchar.h>
#include <wctype.h>
#include <locale.h>
#include <unistd.h>

#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define RESET   "\033[0m"

#define BOLD_BLUE    "\033[34;1m"

void wcslwr(wchar_t *str) {
  if (str == NULL) return;

  while (*str) {
    *str = towlower(*str);
    str++;
  }
}

// Merges two subarrays of arr[].
// First subarray is arr[left..mid]
// Second subarray is arr[mid+1..right]
void merge(struct dirent **arr, int left, int mid, int right) {
  int i, j, k;
  int n1 = mid - left + 1;
  int n2 = right - mid;

  // Create temporary arrays
  struct dirent *leftArr[n1], *rightArr[n2];

  // Copy data to temporary arrays
  for (i = 0; i < n1; i++)
    leftArr[i] = arr[left + i];
  for (j = 0; j < n2; j++)
    rightArr[j] = arr[mid + 1 + j];

  // Merge the temporary arrays back into arr[left..right]
  i = 0;
  j = 0;
  k = left;
  while (i < n1 && j < n2) {
    size_t left_req_size = mbstowcs(NULL, leftArr[i]->d_name, 0) + 1;
    size_t right_req_size = mbstowcs(NULL, rightArr[j]->d_name, 0) + 1;
    wchar_t *left_w = malloc(left_req_size * sizeof(wchar_t));
    wchar_t *right_w = malloc(right_req_size * sizeof(wchar_t));

    mbstowcs(left_w, leftArr[i]->d_name, left_req_size);
    mbstowcs(right_w, rightArr[j]->d_name, right_req_size);

    wcslwr(left_w);
    wcslwr(right_w);

    wchar_t *left_w_cpy = left_w;
    wchar_t *right_w_cpy = right_w;

    if (left_w[0] == L'.') {
      left_w_cpy++;
    }
    if (right_w[0] == L'.') {
      right_w_cpy++;
    }

    // printf("comparing %s and %s\n", leftArr[i]->d_name, rightArr[i]->d_name);
    int cmp = wcscmp(left_w_cpy, right_w_cpy);

    if (cmp <= 0) {
      arr[k] = leftArr[i];
      i++;
    }
    else {
      arr[k] = rightArr[j];
      j++;
    }
    k++;

    free(left_w);
    free(right_w);
  }

  // Copy the remaining elements of leftArr[], if any
  while (i < n1) {
    arr[k] = leftArr[i];
    i++;
    k++;
  }

  // Copy the remaining elements of rightArr[], if any
  while (j < n2) {
    arr[k] = rightArr[j];
    j++;
    k++;
  }
}

// The subarray to be sorted is in the index range [left-right]
void mergeSort(struct dirent **arr, int left, int right) {
  if (left < right) {
    // Calculate the midpoint
    int mid = left + (right - left) / 2;

    // Sort first and second halves
    mergeSort(arr, left, mid);
    mergeSort(arr, mid + 1, right);

    // Merge the sorted halves
    merge(arr, left, mid, right);
  }
}

void print_dir(
  char *dir, uint depth, uint depth_limit, char *prefix,
  bool list_all, uint *dirs_count, uint *files_count
) {
  struct dirent *de;
  DIR *dr = opendir(dir);

  if (dr == NULL) {
    printf("Couldn't open provided directory `%s`", dir);
    return;
  }

  struct dirent **entries = malloc(sizeof(struct dirent*));
  uint entries_capacity = 1;
  uint entries_count = 0;

  while ((de = readdir(dr)) != NULL) {
    char *entry_name = de->d_name;
    if (!strcmp(entry_name, ".") || !strcmp(entry_name, "..")) {
      continue;
    }

    if (!list_all && entry_name[0] == '.') {
      continue;
    }

    if (entries_count == entries_capacity) {
      entries_capacity *= 2;
      entries = realloc(entries, entries_capacity * sizeof(struct dirent*));
    }

    entries[entries_count] = malloc(sizeof(struct dirent));
    if (entries[entries_count] != NULL) {
      memcpy(entries[entries_count], de, sizeof(struct dirent));
      entries_count += 1;
    }
  }

  if (entries_count > 0) {
    mergeSort(entries, 0, entries_count - 1);
  }

  for (uint idx = 0; idx < entries_count; idx++) {
    printf("%s", prefix);

    if (idx != entries_count - 1) {
      printf("├");
    } else {
      printf("└");
    }
    printf("── ");

    struct dirent *entry = entries[idx];
    if (entry->d_type == DT_DIR) {
      printf(BOLD_BLUE "%s\n" RESET, entry->d_name);
      (*dirs_count)++;
    } else {
      printf("%s\n", entry->d_name); 
      (*files_count)++;
    }

    if (entries[idx]->d_type == DT_DIR && depth < depth_limit) {
      char *entry_name = entry->d_name;
      uint dir_len = strlen(dir);
      uint entry_name_len = strlen(entry_name);
      char *new_dir = malloc((dir_len + 1 + entry_name_len + 1) * sizeof(char));
      memcpy(new_dir, dir, dir_len * sizeof(char));
      new_dir[dir_len] = '/';
      memcpy(new_dir + dir_len + 1, entry_name, entry_name_len);
      new_dir[dir_len + 1 + entry_name_len] = '\0';

      char *new_prefix;
      if (idx != entries_count - 1) {
        const char *p = "│   ";
        new_prefix = malloc((strlen(prefix) + strlen(p) + 1) * sizeof(char));
        strcpy(new_prefix, prefix);
        strcat(new_prefix, p);
      } else {
        const char *p = "    ";
        new_prefix = malloc((strlen(prefix) + strlen(p) + 1) * sizeof(char));
        strcpy(new_prefix, prefix);
        strcat(new_prefix, p);
      }
      print_dir(
        new_dir, depth + 1, depth_limit, new_prefix, list_all,
        dirs_count, files_count
      );
      free(new_dir);
      free(new_prefix);
    }
  }

  for (uint idx = 0; idx < entries_count; idx++) {
    free(entries[idx]);
  }
  free(entries);
  closedir(dr);
}

int main(int argc, char* argv[]) {
  setlocale(LC_ALL, "");

  char* dir_path;
  uint depth_limit = UINT_MAX;
  bool list_all = false;

  int opt;
  while ((opt = getopt(argc, argv, "L:a")) != -1) {
    switch (opt) {
      case 'L':
        ;
        int tmp = strtol(optarg, NULL, 10);
        if (tmp > 0) {
          depth_limit = tmp;
        } else {
          printf("Depth limit has to be >= 1\n");
          return 0;
        }
        break;

      case 'a':
        list_all = true;
        break;

      default:
        break;
    }
  }

  if (optind < argc) {
    dir_path = argv[optind];
  } else {
    dir_path = ".";
  }

  printf("%s\n", dir_path);

  uint dirs_count[1] = {0};
  uint files_count[1] = {0};
  print_dir(dir_path, 1, depth_limit, "", list_all, dirs_count, files_count);

  printf("\n%d directories, %d files\n", *dirs_count, *files_count);

  return 0;
}
