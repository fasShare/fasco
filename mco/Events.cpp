#include <strings.h>

#include <Events.h>

moxie::Events::Events(const int fd, uint32_t events) :
    fd_(fd),
    state_(state::NEW),
    type_(EVENT_TYPE_START),
    eorigin_(events),
    mutable_(kNoneEvent) {
}

moxie::Events::Events(const Events& events) :
    Events(events.fd_, events.eorigin_) {
}

//下面这两个构造主要用与polling返回时
moxie::Events::Events(const struct epoll_event& events) :
    Events(events.data.fd, events.events){
}

moxie::Events::Events(const struct pollfd& events) :
    Events(events.fd, events.revents) {
}

bool moxie::Events::reset() {
    fd_ = -1;
    type_ = EVENT_TYPE_START;
    state_ = state::INVAILD;
    eorigin_ = kNoneEvent;
    mutable_ = kNoneEvent;
    return true;
}

int moxie::Events::getType() const {
    return type_;
}

void moxie::Events::setType(int tp) {
    type_ = tp;
}

bool moxie::Events::invaild() const {
    return state_ == state::INVAILD;
}

bool moxie::Events::ismod() const {
    return state_ == state::MOD;
}

bool moxie::Events::isdel() const {
    return state_ == state::DEL;
}

bool moxie::Events::isnew() const {
    return state_ == state::NEW;
}

bool moxie::Events::isused() const {
    return state_ == state::USED;
}

int moxie::Events::getFd() const{
    return fd_;
}

long moxie::Events::getTid() const {
    return tid_;
}

void moxie::Events::setTid(long tid) {
    tid_ = tid;
}

moxie::Events::state moxie::Events::getState() const {
    return state_;
}

void moxie::Events::setState(moxie::Events::state sta) {
    state_ = sta;
}

uint32_t moxie::Events::getEvents() const{
    return eorigin_;
}

void moxie::Events::updateEvents(uint32_t events) {
    eorigin_ = events;
    state_ = state::MOD;
}

void moxie::Events::addEvent(uint32_t event) {
    eorigin_ |= event;
    state_ = state::MOD;
}

void moxie::Events::deleteEvent(uint32_t event) {
    eorigin_ &= ~event;
    state_ = state::MOD;
}

void moxie::Events::addEventMutable(uint32_t event) {
    mutable_ |= event;
    state_ = state::MOD;
}

void moxie::Events::deleteEventMutable(uint32_t event) {
    mutable_ &= ~event;
    state_ = state::MOD;
}

void moxie::Events::setMutable(uint32_t event) {
    mutable_ = event;
}

uint32_t moxie::Events::getMutable() const {
    return mutable_;
}

bool moxie::Events::isRead() {
    return containsAtLeastOneEvents(mutable_, POLLIN | POLLPRI | POLLRDHUP);
}
bool moxie::Events::isWrite() {
    return containsAtLeastOneEvents(mutable_, POLLOUT);
}
bool moxie::Events::isError() {
    return containsAtLeastOneEvents(mutable_, POLLERR | POLLNVAL);
}
bool moxie::Events::isClose() {
    bool ret =  containsAtLeastOneEvents(mutable_, POLLHUP) 
        && !containsAtLeastOneEvents(mutable_, POLLIN);
    return ret;
}

bool moxie::Events::originRead() {
    return containsAtLeastOneEvents(eorigin_, POLLIN | POLLPRI | POLLRDHUP);
}
bool moxie::Events::originWrite() {
    return containsAtLeastOneEvents(eorigin_, POLLOUT);
}

bool moxie::Events::containsEvents(uint32_t emutable, uint32_t events) const {
    return ((emutable & events) == events);
}

bool moxie::Events::containsAtLeastOneEvents(uint32_t emutable, uint32_t events) const {
    return ((emutable & events) != 0);
}

struct epoll_event moxie::Events::epollEvents() {
    struct epoll_event events;
    bzero(&events, sizeof(struct epoll_event));
    events.data.fd = this->getFd();
    events.events = this->getEvents();
    return events;
}

struct pollfd moxie::Events::pollEvents() {
    struct pollfd events;
    bzero(&events, sizeof(struct pollfd));
    events.fd = this->getFd();
    events.events = this->getEvents();
    return events;
}

moxie::Events::~Events() {
    ::close(fd_);
    LOGGER_TRACE("Events destroyed.");
}

std::ostream& moxie::operator<<(std::ostream& os, Events& events) {
    os << "fd = " << events.getFd();
    return os;
}
