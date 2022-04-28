#ifndef  __SYS_LOG_H__
#define __SYS_LOG_H__ //定义头文件，防止重复引用
#include <string>
#include <stdint.h>
#include <cstdarg>
#include <list>
#include <memory>
#include <vector>
#include <map>
#include <ctime>
#include <fstream>
#include <sstream>
#include "util.h"

#define MAKE_LOG_EVELT(level,message) \
    std::make_shared<syslog::LogEvent>(level,__FILE__,message,__LINE__,0,syslog::getThreadId(),syslog::getFiberId(),::time(nullptr)) 

#define LOG_EVENT(logger,level,message) \
    logger->log(level,MAKE_LOG_EVELT(level,message));

#define LOG_INFO(logger,msg) LOG_EVENT(logger,syslog::LogLevel::INFO,msg)
#define LOG_DEBUG(logger,msg) LOG_EVENT(logger,syslog::LogLevel::DEBUG,msg)
#define LOG_WARN(logger,msg) LOG_EVENT(logger,syslog::LogLevel::WARN,msg)
#define LOG_FATAL(logger,msg) LOG_EVENT(logger,syslog::LogLevel::FATAL,msg)
#define LOG_ERROR(logger,msg) LOG_EVENT(logger,syslog::LogLevel::ERROR,msg)

#define LOG_FMT_EVENT(logger,level,fmt,argv...) \
    {                                \
        char* b=nullptr;             \
        int n= asprintf(&b, fmt,argv); \ 
        if (n!=-1){                  \
            LOG_EVENT(logger,level,std::string(b,n)); \
            free(b); \
        }\
    }

#define LOG_FMT_INFO(logger,fmt,arg...) LOG_FMT_EVENT(logger,syslog::LogLevel::INFO,fmt,arg)
#define LOG_FMT_DEBUG(logger,fmt,arg...) LOG_FMT_EVENT(logger,syslog::LogLevel::DEBUG,fmt,arg)
#define LOG_FMT_WARN(logger,fmt,arg...) LOG_FMT_EVENT(logger,syslog::LogLevel::WARN,fmt,arg)
#define LOG_FMT_FATAL(logger,fmt,arg...) LOG_FMT_EVENT(logger,syslog::LogLevel::FATAL,fmt,arg)
#define LOG_FMT_ERROR(logger,fmt,arg...) LOG_FMT_EVENT(logger,syslog::LogLevel::ERROR,fmt,arg)




//#define GET_ROOT_LOGGER() 
namespace syslog{//区分不同的代码空间
    //每个日志对应一个Log Event
    class Logger;
    //日志事件

    class LogLevel{
    public:
         enum Level{
                UNKNOW=0,
                DEBUG=1,
                INFO=2,
                WARN=3,
                ERROR=4,
                FATAL=5,
         };
        /*
         * @param 日志转换string
         */
        static const char* ToString(LogLevel::Level level);
        /*
         * @param str 将字符串转换成日志
         */
        static LogLevel::Level FromString(const std::string& str);
    };

    class LogEvent{
    public:
        typedef std::shared_ptr<LogEvent> eventPtr;

        LogEvent(LogLevel::Level level,const std::string file,std::string content,
                 int32_t line,uint32_t elapse,uint32_t thread_id,uint32_t fiber_id,time_t  time);

    public:
        LogLevel::Level getLevel() const { return m_level; }
        uint32_t getLine() const { return m_line; }
        uint32_t getThreadId() const { return m_thread_id; }
        uint32_t getFiberId() const { return m_fiber_id; }
        time_t getTime() const { return m_time; }
        const std::string& getContent() const { return m_content; }

        const std::string getFile(){return m_file;}
        //格式化写入的日志

    private:
        //定义文件名
        const std::string m_file= nullptr;
        //定义行号,线程号、协程号、内容.时间
        int32_t m_line=0;
        uint32_t m_thread_id=0;
        uint32_t m_fiber_id=0;
        uint64_t m_elapse=0; //消失的时间戳
        time_t   m_time;
        std::string m_content;
        //日志等级
        LogLevel::Level m_level;
    };

    class LogEventWrap{
    public:
        LogEventWrap(LogEvent::eventPtr ptr);
        ~LogEventWrap();
        LogEvent::eventPtr getEvent() const{return m_event;}
    private:
        LogEvent::eventPtr m_event;
    };

    //日志格式
    class LogFormat{
    public:
        typedef std::shared_ptr<LogFormat> ptr;
        //根据pattern来解析字符串
        LogFormat(const std::string& pattern);
    public:
        std::string format(LogLevel::Level level,LogEvent::eventPtr p);
        std::ostream& format(std::ostream& ofs, LogLevel::Level level, LogEvent::eventPtr event);

        std::string getPattern() const{return m_pattern;}

        class FormatItem{
        public:
            typedef std::shared_ptr<FormatItem> ptr;
            virtual void format(std::ostream& os ,LogEvent::eventPtr p)=0;
        };
    private:
        //%m .. %d，解析format
        void init();
        std::vector<FormatItem::ptr> items;
        //日志格式模版
        const std::string m_pattern;
    };
    //日志输出器
    class LogAppender{
        friend class Logger;
    public:
        typedef std::shared_ptr<LogAppender> ptr;
        //typedef Spinlock MutexType;
        explicit LogAppender(LogLevel::Level level=LogLevel::DEBUG);
        virtual void log(LogLevel::Level level,const LogEvent::eventPtr event)=0;
    public:

        virtual std::string toYamlString() = 0;
        void setFormat(LogFormat::ptr format);
        LogFormat::ptr getFormat()const;
        LogLevel::Level getLevel()const {return m_level;}
        void setLevel(LogLevel::Level level){m_level=level;}
    protected:
        LogFormat::ptr m_format;
        LogLevel::Level m_level;
        bool m_hasFormatter = false;
    };

    class LoggerManager;
    //日志器
    class Logger{
        friend class LoggerManager;
    public:
        typedef std::shared_ptr<Logger> loggerPtr;
        //typedef Spinlock MutexType;
        Logger(const std::string& name="root");

    public:
        void log(LogLevel::Level level,const LogEvent::eventPtr event);

        void debug(LogEvent::eventPtr event);
        void info(LogEvent::eventPtr event);
        void warn(LogEvent::eventPtr event);
        void error(LogEvent::eventPtr event);
        void fatal(LogEvent::eventPtr event);

        void addAppender(LogAppender::ptr ptr);
        void deleteAppender(LogAppender::ptr ptr);
        void clearAppenders();
        const std::string& getName() const { return m_name;}
        LogLevel::Level getLevel() const{return m_level;}
        LogLevel::Level setLevel(LogLevel::Level level){m_level=level;}

        void setFormatter(LogFormat::ptr val);

        void setFormatter(const std::string& val);

        LogFormat::ptr getFormatter();

        std::string toYamlString();
    private:
        //日志名称
        std::string m_name;
        //日志级别
        LogLevel::Level m_level;
        //日志目标集合
        std::list<LogAppender::ptr> m_appenders;
        //日志格式
        LogFormat::ptr  logFormat;
        //主日志器
    };

    class StdOutAppender :public LogAppender{
    public:
        typedef std::shared_ptr<StdOutAppender> ptr;
        explicit StdOutAppender(LogLevel::Level level=LogLevel::DEBUG);
        void log(LogLevel::Level level,const LogEvent::eventPtr event) override;
        std::string toYamlString() override;
    };

    class FileOutAppender: public LogAppender{
    public:
        FileOutAppender(const std::string& filename);
        typedef std::shared_ptr<FileOutAppender> ptr;
    public:
        void log(LogLevel::Level level,const LogEvent::eventPtr event) override;
        std::string toYamlString() override;
        //重新打开日志文件
        bool reopen();
    private:
        //文件路径
        std::string m_name;
        //文件流
        std::ofstream m_filestream;
        //上次打开时间
        uint64_t last_time=0;
    };

    class LoggerManager{
    public:
        //Mutex
        LoggerManager();
        Logger::loggerPtr getLogger(const std::string& name);

        //返回主日志器
        Logger::loggerPtr getRoot() {return m_root;}

        /**
        * @brief 将所有的日志器配置转成YAML String
        */
        std::string toYamlString();
    private:
        //初始化
        void init();
        /// Mutex
        //MutexType m_mutex;
        /// 日志器容器
        std::map<std::string, Logger::loggerPtr> m_loggers;
        /// 主日志器
        Logger::loggerPtr m_root;
    };

}
#endif