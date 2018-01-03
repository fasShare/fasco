#include <syscall.h>
#include <boost/make_shared.hpp>

#include <McoPool.h>
#include <MutexLocker.h>

#define gettid() (::syscall(SYS_gettid))

moxie::McoPool *moxie::McoPool::instance_ = nullptr;

boost::shared_ptr<Continuation> moxie::McoPool::getCurMco() {
    auto tid = gettid();
    boost::shared_ptr<Continuation> ret;
    {
        MutexLocker lock(mutex_);
        auto iter = curMcos_.find(tid);
        if (iter == curMcos_.end()) {
            ret = nullptr;
        } else {
            ret = iter->second;
        }
    }
    if (!ret) {
        ret = getMainMco();
    }
    return ret;
}

void moxie::McoPool::setCurMco(boost::shared_ptr<Continuation> mco) {
    auto tid = gettid();
    {
        MutexLocker lock(mutex_);
        curMcos_[tid] = mco;
    }
}

boost::shared_ptr<Continuation> moxie::McoPool::getMcoRoutine(int fd) {
    auto tid = gettid();
    {
        MutexLocker lock(mutex_);
        auto umap = fdMcos_.find(tid); 
        if (umap == fdMcos_.end()) {
            fdMcos_[tid] = std::unordered_map<int, boost::shared_ptr<Continuation>>();
            return nullptr;
        }
        auto fdmap = umap->second;
        auto ret = fdmap.find(fd);
        if (ret == fdmap.end()) {
            return nullptr;
        }
        return ret->second;
    }
}

bool moxie::McoPool::removeMcoRoutine(int fd) {
	auto tid = gettid();
	{
		MutexLocker lock(mutex_);
		auto umap = fdMcos_.find(tid);
		if (umap != fdMcos_.end()) {
			int ret = umap->second.erase(fd);
			return ret == 1;
		}
		return false;
	}
}

bool moxie::McoPool::setMcoRoutine(int fd, boost::shared_ptr<Continuation> co) {
    auto tid = gettid();
    {
        MutexLocker lock(mutex_);
        auto umap = fdMcos_.find(tid);
        if (umap == fdMcos_.end()) {
            fdMcos_[tid] = std::unordered_map<int, boost::shared_ptr<Continuation>>();
        }
        fdMcos_[tid][fd] = co;
        return true;
    }
}

boost::shared_ptr<Continuation> moxie::McoPool::getMainMco() {
    auto tid = gettid();
    {
        MutexLocker lock(mutex_);
        auto iter = mainMco_.find(tid);
        if (iter == mainMco_.end()) {
            mainMco_[tid] = boost::make_shared<Continuation>(MainMco());
            curMcos_[tid] = mainMco_[tid];
            return mainMco_[tid];
        }
        return iter->second;
    }
}
