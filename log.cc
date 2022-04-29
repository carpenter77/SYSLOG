#include "log.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <tuple>
#include <functional>
#include <ctime>
#include "string.h"
#include "stdarg.h"
#include <cassert>
#include "util.h"
// Created by think on 2022/4/15.
//
using namespace syslog;

const char* LogLevel::ToString(LogLevel::Level level) {
        #define xx(level,v){\
            if(level==v)\
                return #level;\
        }
        xx(DEBUG,level)
        xx(INFO,level)
        xx(WARN,level)
        xx(ERROR,level)
        xx(FATAL,level)
        return "UNKNOWN";
        #undef xx
    }
    
LogLevel::Level LogLevel::FromString(const std::string& str) {
#define XX(level, v)   \
    if(str == #v) { \
        return LogLevel::level; }
        XX(DEBUG, debug);
        XX(INFO, info);
        XX(WARN, warn);
        XX(ERROR, error);
        XX(FATAL, fatal);

        XX(DEBUG, DEBUG);
        XX(INFO, INFO);
        XX(WARN, WARN);
        XX(ERROR, ERROR);
        XX(FATAL, FATAL);
        return LogLevel::UNKNOW;
#undef XX
    }
    
    LogEventWrap::LogEventWrap(LogEvent::eventPtr ptr):m_event(ptr) {}
    LogEventWrap::~LogEventWrap() {
    }
    std::stringstream& LogEventWrap::getSS() {
        return m_event->getSS();
    }

    LogAppender::LogAppender(LogLevel::Level level){
        m_level=level;
    }
    void LogAppender::setFormat(LogFormat::ptr val) {
        //MutexType::Lock lock(m_mutex);
        m_format = val;
        if(m_format) {
            m_hasFormatter = true;
        } else {
            m_hasFormatter = false;
        }
    }
    LogFormat::ptr LogAppender::getFormat() const{
        //加锁
        return m_format;
    }

    std::string LogFormat::format(LogLevel::Level level, LogEvent::eventPtr event) {
        std::stringstream ss;
        for(auto& i : items) {
            i->format(ss,  event);
        }
        return ss.str();
    }
    std::ostream& LogFormat::format(std::ostream& ofs,LogLevel::Level level, LogEvent::eventPtr event) {
        for(auto item: items){
            item->format(ofs,event);
        }
        return ofs;
    }
class PlainFormatItem : public LogFormat::FormatItem
{
public:
    explicit PlainFormatItem(const std::string& str) : m_str(str) {}
    void format(std::ostream& out, LogEvent::eventPtr ev) override
    {
        out << m_str;
    }

private:
    std::string m_str;
};
class LevelFormatItem : public LogFormat::FormatItem
{
public:
    void format(std::ostream& out, LogEvent::eventPtr ev) override
    {
        out << LogLevel::ToString(ev->getLevel());
    }
};

class FilenameFormatItem : public LogFormat::FormatItem
{
public:
    void format(std::ostream& out, LogEvent::eventPtr ev) override
    {
        out << ev->getFile();
    }
};

class LineFormatItem : public LogFormat::FormatItem
{
public:
    void format(std::ostream& out, LogEvent::eventPtr ev) override
    {
        out << ev->getLine();
    }
};

class ThreadIDFormatItem : public LogFormat::FormatItem
{
public:
    void format(std::ostream& out, LogEvent::eventPtr ev) override
    {
        out << ev->getThreadId();
    }
};

class FiberIDFormatItem : public LogFormat::FormatItem
{
public:
    void format(std::ostream& out, LogEvent::eventPtr ev) override
    {
        out << ev->getFiberId();
    }
};

class TimeFormatItem : public LogFormat::FormatItem
{
public:
    explicit TimeFormatItem(const std::string& str = "%Y-%m-%d %H:%M:%S")
        : m_time_pattern(str)
    {
        if (m_time_pattern.empty())
        {
            m_time_pattern = "%Y-%m-%d %H:%M:%S";
        }
    }
    void format(std::ostream& out, LogEvent::eventPtr ev) override
    {
        struct tm time_struct
        {
        };
        time_t time_l = ev->getTime();
        localtime_r(&time_l, &time_struct);
        char buffer[64]{0};
        strftime(buffer, sizeof(buffer),
                 m_time_pattern.c_str(), &time_struct);
        out << buffer;
    }

private:
    std::string m_time_pattern;
};

class ContentFormatItem : public LogFormat::FormatItem
{
public:
    void format(std::ostream& out, LogEvent::eventPtr ev) override
    {
        out << ev->getContent();
    }
};

class NewLineFormatItem : public LogFormat::FormatItem
{
public:
    void format(std::ostream& out, LogEvent::eventPtr ev) override
    {
        out << std::endl;
    }
};

class PercentSignFormatItem : public LogFormat::FormatItem
{
public:
    void format(std::ostream& out, LogEvent::eventPtr ev) override
    {
        out << '%';
    }
};

class TabFormatItem : public LogFormat::FormatItem
{
public:
    void format(std::ostream& out, LogEvent::eventPtr ev) override
    {
        out << '\t';
    }
};
/**
 * %p 输出日志等级
 * %f 输出文件名
 * %l 输出行号
 * %d 输出日志时间
 * %t 输出线程号
 * %F 输出协程号
 * %m 输出日志消息
 * %n 输出换行
 * %% 输出百分号
 * %T 输出制表符
 * */
thread_local static std::map<char, LogFormat::FormatItem::ptr> format_item_map{
#define FN(CH, ITEM_NAME)                 \
    {                                     \
        CH, std::make_shared<ITEM_NAME>() \
    }
    FN('p', LevelFormatItem),
    FN('f', FilenameFormatItem),
    FN('l', LineFormatItem),
    FN('d', TimeFormatItem),
    FN('t', ThreadIDFormatItem),
    FN('F', FiberIDFormatItem),
    FN('m', ContentFormatItem),
    FN('n', NewLineFormatItem),
    FN('%', PercentSignFormatItem),
    FN('T', TabFormatItem),
#undef FN
};

    LogEvent::LogEvent( LogLevel::Level level
            ,const std::string fileName,std::string content, int32_t line, uint32_t elapse
            ,uint32_t thread_id, uint32_t fiber_id,  time_t time)
            :m_file(fileName)
            ,m_content(content)
            ,m_line(line)
            ,m_elapse(elapse)
            ,m_thread_id(thread_id)
            ,m_fiber_id(fiber_id)
            ,m_time(time)
            ,m_level(level) {
    }
    void LogEvent::format(const char* fmt, ...) {
        va_list al;
        va_start(al, fmt);
        format(fmt, al);
        va_end(al);
    }

    void LogEvent::format(const char* fmt, va_list al) {
        char* buf = nullptr;
        int len = vasprintf(&buf, fmt, al);
        if(len != -1) {
            m_ss << std::string(buf, len);
            free(buf);
        }
    }

    Logger::Logger(const std::string& name)
            :m_name(name)
            ,m_level(LogLevel::DEBUG) {
        logFormat.reset(new LogFormat("[ %d %p ] %T [ %f:%l ] %T [ %t:%F ] %T [ %m ]%n"));
    }
    void Logger::setFormatter(LogFormat::ptr val) {
        //MutexType::Lock lock(m_mutex);
        logFormat = val;

        for(auto& i : m_appenders) {
          //  MutexType::Lock ll(i->m_mutex);
            if(!i->m_hasFormatter) {
                i->m_format = logFormat;
            }
        }
    }
    void Logger::setFormatter(const std::string& val) {
        std::cout << "---" << val << std::endl;
        LogFormat::ptr new_val(new LogFormat(val));
        //m_formatter = new_val;
        setFormatter(new_val);
    }
    std::string Logger::toYamlString() {
        //MutexType::Lock lock(m_mutex);
        //YAML::Node node;
        //node["name"] = m_name;
//        if(m_level != LogLevel::UNKNOW) {
//            node["level"] = LogLevel::ToString(m_level);
//        }
//        if(m_formatter) {
//            node["formatter"] = m_formatter->getPattern();
//        }
//
//        for(auto& i : m_appenders) {
//            node["appenders"].push_back(YAML::Load(i->toYamlString()));
//        }
//        std::stringstream ss;
//        ss << node;
//        return ss.str();
        return "unimplement";
    }
    LogFormat::ptr Logger::getFormatter() {
        //MutexType::Lock lock(m_mutex);
        return logFormat;
    }

    void Logger::addAppender(LogAppender::ptr appender) {
        //MutexType::Lock lock(m_mutex);
        if(!appender->getFormat()) {
          //  MutexType::Lock ll(appender->m_mutex);
            appender->m_format = logFormat;
        }
        m_appenders.push_back(appender);
    }

    void Logger::deleteAppender(LogAppender::ptr appender) {
        //MutexType::Lock lock(m_mutex);
        for(auto it = m_appenders.begin();
            it != m_appenders.end(); ++it) {
            if(*it == appender) {
                m_appenders.erase(it);
                break;
            }
        }
    }

    void Logger::clearAppenders() {
       // MutexType::Lock lock(m_mutex);
        m_appenders.clear();
    }

    void Logger::log(LogLevel::Level level, LogEvent::eventPtr event) {
        if(level >= m_level) {
            //auto self = shared_from_this();
          //  MutexType::Lock lock(m_mutex);
            if(!m_appenders.empty()) {
                for(auto& i : m_appenders) {
                    i->log(level, event);
                }
            }
        }
    }
    void Logger::debug(LogEvent::eventPtr event){
        log(LogLevel::DEBUG,event);
    }
    void Logger::info(LogEvent::eventPtr event){
        log(LogLevel::INFO,event);
    }
    void Logger::warn(LogEvent::eventPtr event){
        log(LogLevel::WARN,event);
    }
    void Logger::error(LogEvent::eventPtr event){
        log(LogLevel::ERROR,event);
    }
    void Logger::fatal(LogEvent::eventPtr event){
        log(LogLevel::FATAL,event);
    }

    FileOutAppender::FileOutAppender(const std::string& filename)
            :m_name(filename) {
        reopen();
    }

    void FileOutAppender::log(LogLevel::Level level, LogEvent::eventPtr event) {
        if(level >= m_level) {
            uint64_t now = event->getTime();
            if(now >= (last_time + 3)) {
                reopen();
                last_time = now;
            }
            //MutexType::Lock lock(m_mutex);
            //if(!(m_filestream << m_formatter->format(logger, level, event))) {
            if(!m_format->format(m_filestream, level, event)) {
                std::cout << "error" << std::endl;
            }
        }
    }

    std::string FileOutAppender::toYamlString() {
//        MutexType::Lock lock(m_mutex);
//        YAML::Node node;
//        node["type"] = "FileLogAppender";
//        node["file"] = m_name;
//        if(m_level != LogLevel::UNKNOW) {
//            node["level"] = LogLevel::ToString(m_level);
//        }
//        if(m_hasFormatter && m_format) {
//            node["formatter"] = m_format->getPattern();
//        }
        std::stringstream ss;
       // ss << node;
        return ss.str();
    }

    bool FileOutAppender::reopen() {
       // MutexType::Lock lock(m_mutex);
        if(m_filestream) {
            m_filestream.close();
        }
        return FSUtil::OpenForWrite(m_filestream, m_name, std::ios::app);
    }

    StdOutAppender::StdOutAppender(LogLevel::Level level)
        :LogAppender(level){

        }
    void StdOutAppender::log(LogLevel::Level level, LogEvent::eventPtr event) {
        if(level >= m_level) {
           // MutexType::Lock lock(m_mutex);
            m_format->format(std::cout, level, event);
        }
    }

    std::string StdOutAppender::toYamlString() {
       // MutexType::Lock lock(m_mutex);
//        YAML::Node node;
//        node["type"] = "StdoutLogAppender";
//        if(m_level != LogLevel::UNKNOW) {
//            node["level"] = LogLevel::ToString(m_level);
//        }
//        if(m_hasFormatter && m_formatter) {
//            node["formatter"] = m_formatter->getPattern();
//        }
        std::stringstream ss;
     //   ss << node;
        return ss.str();
    }

    LogFormat::LogFormat(const std::string& pattern): m_pattern(pattern){
        this->init();
    }

    //%m %m(m) %% 主要三种处理格式
    void LogFormat::init(){
        enum PARSE_STATUS
        {
            SCAN_STATUS,   // 扫描普通字符
            CREATE_STATUS, // 扫描到 %，处理占位符
        };
        PARSE_STATUS STATUS = SCAN_STATUS;
        size_t str_begin = 0, str_end = 0;
    //    std::vector<char> item_list;
        for (size_t i = 0; i < m_pattern.length(); i++)
        {
            switch (STATUS)
            {
            case SCAN_STATUS: // 普通扫描分支，将扫描到普通字符串创建对应的普通字符处理对象后填入 m_format_item_list 中
                // 扫描记录普通字符的开始结束位置
                str_begin = i;
                for (str_end = i; str_end < m_pattern.length(); str_end++)
                {
                    // 扫描到 % 结束普通字符串查找，将 STATUS 赋值为占位符处理状态，等待后续处理后进入占位符处理状态
                    if (m_pattern[str_end] == '%')
                    {
                        STATUS = CREATE_STATUS;
                        break;
                    }
                }
                i = str_end;
                items.push_back(
                    std::make_shared<PlainFormatItem>(
                        m_pattern.substr(str_begin, str_end - str_begin)));
                break;

            case CREATE_STATUS: // 处理占位符
                assert(!format_item_map.empty() && "format_item_map 没有被正确的初始化");
                auto itor = format_item_map.find(m_pattern[i]);
                if (itor == format_item_map.end())
                {
                    items.push_back(std::make_shared<PlainFormatItem>("<error format>"));
                }
                else
                {
                    items.push_back(itor->second);
                }
                STATUS = SCAN_STATUS;
                break;
            }
        }
    }
    
    LoggerManager::LoggerManager() {
        m_root.reset(new Logger);
        LogAppender::ptr appender=std::make_shared<StdOutAppender>();
        m_root->addAppender(std::move(appender));

        m_loggers[m_root->m_name] = m_root;

        init();
    }

    Logger::loggerPtr LoggerManager::getLogger(const std::string& name) {
        //MutexType::Lock lock(m_mutex);
        auto it = m_loggers.find(name);
        if(it != m_loggers.end()) {
            return it->second;
        }

        Logger::loggerPtr logger(new Logger(name));
        //logger->m_root = m_root;
        m_loggers[name] = logger;
        return logger;
    }
    void LoggerManager::init(){
        std::cout<<"not implement init func"<<std::endl;
    }