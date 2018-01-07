#include <SigIgnore.h>
#include <signal.h>

moxie::SigIgnore::SigIgnore() {
    ::signal(SIGPIPE, SIG_IGN);
}

moxie::SigIgnore SigIgn;
