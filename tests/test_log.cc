
#include <string>
#include <iostream>
#include "log.h"
#include "singleton.h"
void test_log()
{
    syslog::Logger::loggerPtr logger = std::make_shared<syslog::Logger>();
    logger->addAppender(std::make_shared<syslog::StdOutAppender>());
    const std::string file("file.txt");
    syslog::LogEvent::eventPtr event = std::make_shared<syslog::LogEvent>(syslog::LogLevel::INFO, file, "test content", __LINE__, -1, 1, 2, time(0));
    logger->log(syslog::LogLevel::INFO, event);
}

void test_log_level(){
    using namespace syslog;
    Logger::loggerPtr logger=GetInstancePtr<syslog::Logger>();
    logger->addAppender(std::make_shared<syslog::StdOutAppender>());
    LOG_DEBUG(logger,"testing debug log");
    LOG_FATAL(logger,"testing fatal log");
    LOG_FMT_INFO(logger,"test info fmt : %s,%d ","info",1);
    LOG_FMT_ERROR(logger,"test error fmt : %s,%d ","error",-1);
}
int main(){
    test_log();

    test_log_level();
    return 0;
}
