#include <errno.h>
#include "hive_fuse.h"

struct fuse_operations hive_fuse_oper =
{
    .getattr = fuse_getattr,
    .readdir = fuse_readdir,
    .open = fuse_open,
    .read = fuse_read,
};

int fuse_getattr(const char* path, struct stat* stbuf)
{
    return -ENOENT;
}

int fuse_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi)
{
    return -EINVAL;
}

int fuse_open(const char* path, struct fuse_file_info* fi)
{
    return -EINVAL;
}

int fuse_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi)
{
    return -EINVAL;
}
