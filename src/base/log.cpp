#include "log.hpp"

#include <filesystem>
#include <iostream>

#include "time.hpp"

namespace araneid {
Logger::Logger() : log_level_(LOG_INFO) {
  std::filesystem::path out_dir =
      std::filesystem::path(__FILE__).parent_path().parent_path() / "out";
  if (!std::filesystem::exists(out_dir)) {
    try {
      std::filesystem::create_directories(out_dir);
    } catch (const std::filesystem::filesystem_error& e) {
      std::cerr << "Failed to create: " << e.what() << std::endl;
      return;
    }
  }
  std::filesystem::path log_file = out_dir / "araneid.log";

  if (std::filesystem::exists(log_file)) {
    std::filesystem::path backup_file =
        out_dir / ("araneid_" + Clock::Now().ToString() + ".log");
    try {
      std::filesystem::rename(log_file, backup_file);
    } catch (const std::filesystem::filesystem_error& e) {
      std::cerr << "Failed to rename existed file: " << e.what() << std::endl;
    }
  }

  std::lock_guard<std::mutex> lock(log_mutex_);
  log_stream_.open(log_file, std::ios::out);
  if (!log_stream_.is_open()) {
    std::cerr << "Failed to open log file: " << log_file << std::endl;
  } else {
    log_stream_ << "=== Logger is created at " << Clock::Now().ToString()
                << " ===" << std::endl;
  }
}

void Logger::SetLogLevel(LogLevel level) {
  std::lock_guard<std::mutex> lock(log_mutex_);
  log_level_ = level;
}

LogLevel Logger::GetLogLevel() const { return log_level_; }

void Logger::WriteLog(LogLevel level, const std::string& message) {
  std::lock_guard<std::mutex> lock(log_mutex_);
  if (level >= log_level_) {
    if (log_stream_.is_open()) {
      log_stream_ << message << std::endl;
    } else {
      std::cerr << "Log file is not open." << std::endl;
    }
  }
}

LogMessage::LogMessage(LogLevel level, const std::string& file, int line)
    : level_(level) {
  // extract the file name from the full path
  std::filesystem::path file_path(file);
  std::string file_name = file_path.filename().string();
  message_stream_ << "[" << Clock::Now().ToString() << "] "
                  << "[" << file_name << ":" << line << "] ";
  switch (level_) {
    case LOG_DEBUG:
      message_stream_ << "[DEBUG] ";
      break;
    case LOG_INFO:
      message_stream_ << "[INFO] ";
      break;
    case LOG_WARNING:
      message_stream_ << "[WARNING] ";
      break;
    case LOG_ERROR:
      message_stream_ << "[ERROR] ";
      break;
  }
}

LogMessage::~LogMessage() {
  Logger::GetInstance().WriteLog(level_, message_stream_.str());
  if (level_ == LOG_ERROR) std::exit(1);
}

}  // namespace araneid