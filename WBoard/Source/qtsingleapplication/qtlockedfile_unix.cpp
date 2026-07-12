#include "qtlockedfile.h"
#include <sys/file.h>
#include <unistd.h>
#include <errno.h>

using namespace QtLP_Private;

bool QtLockedFile::lock(LockMode mode, bool block)
{
    if (!isOpen()) {
        qWarning("QtLockedFile::lock(): file is not opened");
        return false;
    }
    if (mode == NoLock) return unlock();
    if (mode == m_lock_mode) return true;
    unlock();
    int flags = (mode == ReadLock) ? LOCK_SH : LOCK_EX;
    if (!block) flags |= LOCK_NB;
    int fd = handle();
    if (fd < 0) return false;
    int result;
    do { result = flock(fd, flags); } while (result != 0 && errno == EINTR);
    if (result == 0) { m_lock_mode = mode; return true; }
    return false;
}

bool QtLockedFile::unlock()
{
    if (!isOpen()) { qWarning("QtLockedFile::unlock(): file is not opened"); return false; }
    if (!isLocked()) return true;
    int fd = handle();
    if (fd < 0) return false;
    int result;
    do { result = flock(fd, LOCK_UN); } while (result != 0 && errno == EINTR);
    if (result == 0) { m_lock_mode = NoLock; return true; }
    return false;
}

bool QtLockedFile::isLocked() const { return m_lock_mode != NoLock; }

bool QtLockedFile::open(OpenMode mode) { return QFile::open(mode); }

QtLockedFile::QtLockedFile() : m_lock_mode(NoLock) {}

QtLockedFile::~QtLockedFile()
{
    if (isLocked()) unlock();
}
