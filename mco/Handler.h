#ifndef MOXIE_HANDLER_H
#define MOXIE_HANDLER_H
#include <Events.h>
#include <Timestamp.h>

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

namespace moxie {

class Handler {
public:
    static void DoHandle(Handler *handler, boost::shared_ptr<Events> events) {
        while (true) {
            handler->doHandle(events, Timestamp::now());
            if (!events->isdel()) {
                //Continuation::YieldCurMco();
            } else {
                break;
            }
        }
    }
private:
    virtual void doHandle(boost::shared_ptr<Events> events, Timestamp time) = 0;
};

}
#endif // MOXIE_HANDLER_H

