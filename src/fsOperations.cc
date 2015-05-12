/**
 * @file fsOperations.cc
 * @author Krzysztof Trzepla
 * @copyright (C) 2015 ACK CYFRONET AGH
 * @copyright This software is released under the MIT license cited in
 * 'LICENSE.txt'
 */

#include "fsOperations.h"

#include "fsLogic.h"
#include "logging.h"
#include "oneException.h"

#include <execinfo.h>

#include <array>
#include <memory>
#include <exception>

using namespace one::client;

namespace {

template <typename... Args1, typename... Args2>
int wrap(int (FsLogic::*operation)(Args2...), Args1 &&... args)
{
    try {
        FsLogic *fsLogic =
            static_cast<FsLogic *>(fuse_get_context()->private_data);
        return (fsLogic->*operation)(std::forward<Args1>(args)...);
    }
    catch (const OneException &e) {
        LOG(ERROR) << "OneException: " << e.what();
        return -EIO;
    }
    catch (...) {
        std::array<void *, 64> trace;

        const auto size = backtrace(trace.data(), trace.size());
        std::unique_ptr<char *[]> strings {
            backtrace_symbols(trace.data(), size)
        };

        LOG(ERROR) << "Unknown exception caught at the top level. Stacktrace: ";
        for (auto i = 0; i < size; ++i)
            LOG(ERROR) << strings[i];

        return -EIO;
    }
}

extern "C" {

int wrap_access(const char *path, int mode)
{
    return wrap(&FsLogic::access, path, mode);
}
int wrap_getattr(const char *path, struct stat *statbuf)
{
    return wrap(&FsLogic::getattr, path, statbuf, true);
}
int wrap_readlink(const char *path, char *buf, size_t size)
{
    return wrap(&FsLogic::readlink, path, buf, size);
}
int wrap_mknod(const char *path, mode_t mode, dev_t dev)
{
    return wrap(&FsLogic::mknod, path, mode, dev);
}
int wrap_mkdir(const char *path, mode_t mode)
{
    return wrap(&FsLogic::mkdir, path, mode);
}
int wrap_unlink(const char *path) { return wrap(&FsLogic::unlink, path); }
int wrap_rmdir(const char *path) { return wrap(&FsLogic::rmdir, path); }
int wrap_symlink(const char *target, const char *linkpath)
{
    return wrap(&FsLogic::symlink, target, linkpath);
}
int wrap_rename(const char *oldpath, const char *newpath)
{
    return wrap(&FsLogic::rename, oldpath, newpath);
}
int wrap_chmod(const char *path, mode_t mode)
{
    return wrap(&FsLogic::chmod, path, mode);
}
int wrap_chown(const char *path, uid_t uid, gid_t gid)
{
    return wrap(&FsLogic::chown, path, uid, gid);
}
int wrap_truncate(const char *path, off_t newSize)
{
    return wrap(&FsLogic::truncate, path, newSize);
}
int wrap_utime(const char *path, struct utimbuf *ubuf)
{
    return wrap(&FsLogic::utime, path, ubuf);
}
int wrap_open(const char *path, struct fuse_file_info *fileInfo)
{
    return wrap(&FsLogic::open, path, fileInfo);
}
int wrap_read(const char *path, char *buf, size_t size, off_t offset,
    struct fuse_file_info *fileInfo)
{
    return wrap(&FsLogic::read, path, buf, size, offset, fileInfo);
}
int wrap_write(const char *path, const char *buf, size_t size, off_t offset,
    struct fuse_file_info *fileInfo)
{
    return wrap(&FsLogic::write, path, buf, size, offset, fileInfo);
}
int wrap_statfs(const char *path, struct statvfs *statInfo)
{
    return wrap(&FsLogic::statfs, path, statInfo);
}
int wrap_flush(const char *path, struct fuse_file_info *fileInfo)
{
    return wrap(&FsLogic::flush, path, fileInfo);
}
int wrap_release(const char *path, struct fuse_file_info *fileInfo)
{
    return wrap(&FsLogic::release, path, fileInfo);
}
int wrap_fsync(const char *path, int datasync, struct fuse_file_info *fi)
{
    return wrap(&FsLogic::fsync, path, datasync, fi);
}
int wrap_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
    off_t offset, struct fuse_file_info *fileInfo)
{
    return wrap(&FsLogic::readdir, path, buf, filler, offset, fileInfo);
}
int wrap_opendir(const char *path, struct fuse_file_info *fileInfo)
{
    return wrap(&FsLogic::opendir, path, fileInfo);
}
int wrap_releasedir(const char *path, struct fuse_file_info *fileInfo)
{
    return wrap(&FsLogic::releasedir, path, fileInfo);
}
int wrap_fsyncdir(
    const char *path, int datasync, struct fuse_file_info *fileInfo)
{
    return wrap(&FsLogic::fsyncdir, path, datasync, fileInfo);
}

void *init(struct fuse_conn_info *conn)
{
    return fuse_get_context()->private_data;
}

} // extern "C"
} // namespace

struct fuse_operations fuseOperations()
{
    struct fuse_operations operations = {nullptr};

    operations.init = init;
    operations.getattr = wrap_getattr;
    operations.access = wrap_access;
    operations.readlink = wrap_readlink;
    operations.readdir = wrap_readdir;
    operations.mknod = wrap_mknod;
    operations.mkdir = wrap_mkdir;
    operations.symlink = wrap_symlink;
    operations.unlink = wrap_unlink;
    operations.rmdir = wrap_rmdir;
    operations.rename = wrap_rename;
    operations.chmod = wrap_chmod;
    operations.chown = wrap_chown;
    operations.truncate = wrap_truncate;
    operations.open = wrap_open;
    operations.read = wrap_read;
    operations.write = wrap_write;
    operations.statfs = wrap_statfs;
    operations.release = wrap_release;
    operations.fsync = wrap_fsync;
    operations.utime = wrap_utime;
    operations.flush = wrap_flush;
    operations.opendir = wrap_opendir;
    operations.releasedir = wrap_releasedir;
    operations.fsyncdir = wrap_fsyncdir;

    return operations;
}