#include <new>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>

#include <Log.h>

moxie::CommonLog* moxie::CommonLog::logger_ = nullptr;

moxie::CommonLog* moxie::CommonLog::Instance() {
    if (NULL == logger_) {
        logger_ = new(std::nothrow) CommonLog();
    }
    return logger_;
}

bool moxie::CommonLog::openLog(const LogConf& conf) {
    logdir_ = conf.logdir;
    if (::access(logdir_.c_str(), F_OK|W_OK|R_OK)) {
        std::cerr << "fas has no permission of log dir "<< logdir_  << std::endl;
        return false;
	}

	google::InitGoogleLogging("");	

    noticesuffix_ = conf.noticesuffix;
	warnsuffix_ = conf.warnsuffix;

	std::string fataldir = (logdir_ + "/" + warnsuffix_).c_str();
	std::string errordir = (logdir_ + "/" + warnsuffix_).c_str();
	std::string warndir = (logdir_ + "/" + warnsuffix_).c_str();
	std::string noticedir = (logdir_ + "/" + noticesuffix_).c_str();

    google::SetLogDestination(google::GLOG_FATAL, fataldir.c_str());
    google::SetLogDestination(google::GLOG_ERROR, errordir.c_str());
    google::SetLogDestination(google::GLOG_WARNING, warndir.c_str());
    google::SetLogDestination(google::GLOG_INFO, noticedir.c_str());
    
    FLAGS_logbufsecs = conf.logbufsecs;
    FLAGS_max_log_size = conf.maxlogsize > 0 ? conf.maxlogsize : 6;
    FLAGS_stop_logging_if_full_disk = true;
    FLAGS_logtostderr = conf.logtostderr;
	FLAGS_colorlogtostderr = FLAGS_logtostderr;
	FLAGS_alsologtostderr = conf.alsologtostderr;
	FLAGS_minloglevel = conf.minlevelout;
	
	google::SetLogFilenameExtension(".log.");
	
    return true;
}

void moxie::CommonLog::closeLog() {
    google::ShutdownGoogleLogging();
}

