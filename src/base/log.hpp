#ifndef ARANEID_BASE_LOG_HPP
#define ARANEID_BASE_LOG_HPP
#include <fstream>
#include <mutex>
#include <sstream>

namespace araneid {
enum LogLevel { LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERROR, LOG_FATAL };

class Logger {
 public:
  static Logger& GetInstance() {
    static Logger instance;
    return instance;
  }
  void SetLogLevel(LogLevel level);
  LogLevel GetLogLevel() const;
  void WriteLog(LogLevel level, const std::string& message);

 private:
  Logger();
  ~Logger() {
    if (log_stream_.is_open()) {
      log_stream_.close();
    }
  }
  LogLevel log_level_;
  std::ofstream log_stream_;
  std::mutex log_mutex_;
};

class LogMessage {
 public:
  LogMessage(LogLevel level, const std::string& file, int line);
  ~LogMessage();
  std::ostringstream& GetStream() { return message_stream_; }

 private:
  LogLevel level_;
  std::ostringstream message_stream_;
};

}  // namespace araneid

#define ALOG_DEBUG \
  araneid::LogMessage(araneid::LOG_DEBUG, __FILE__, __LINE__).GetStream()
#define ALOG_INFO \
  araneid::LogMessage(araneid::LOG_INFO, __FILE__, __LINE__).GetStream()
#define ALOG_WARNING \
  araneid::LogMessage(araneid::LOG_WARNING, __FILE__, __LINE__).GetStream()
#define ALOG_ERROR \
  araneid::LogMessage(araneid::LOG_ERROR, __FILE__, __LINE__).GetStream()
#define ALOG_FATAL \
  araneid::LogMessage(araneid::LOG_FATAL, __FILE__, __LINE__).GetStream()

#endif