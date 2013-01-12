#ifndef __HIVE_FUSE_H
#define __HIVE_FUSE_H

#define __need_timespec
#include <time.h>
#include <fuse.h>

int fuse_getattr(const char* path, struct stat* stbuf);
int fuse_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi);
int fuse_open(const char* path, struct fuse_file_info* fi);
int fuse_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi);

struct fuse_operations hive_fuse_oper;

#endif