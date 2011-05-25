#ifndef FAT_FILE_H
#define FAT_FILE_H

#include <linux/fs.h>

extern const struct file_operations fat_dir_operations;
extern const struct file_operations fat_file_operations;  // todo to implement

#endif


