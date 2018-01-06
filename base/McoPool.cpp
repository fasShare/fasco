#include <boost/make_shared.hpp>

#include <McoPool.h>

boost::shared_ptr<Continuation> moxie::McoPool::getMcoRoutine(int fd) {
    auto ret = fdMcos_.find(fd);
    if (ret == fdMcos_.end()) {
        return nullptr;
    }
    return ret->second;
}

bool moxie::McoPool::removeMcoRoutine(int fd) {
    int ret = fdMcos_.erase(fd);
    return ret == 1;
}

void moxie::McoPool::setMcoRoutine(int fd, boost::shared_ptr<Continuation> co) {
    fdMcos_[fd] = co;
}
