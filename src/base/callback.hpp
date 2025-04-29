#ifndef ARANEID_BASE_CALLBACK_HPP
#define ARANEID_BASE_CALLBACK_HPP

#include <iostream>
#include <queue>
#include <tuple>
#include <utility>
#include <memory>
namespace araneid {

class CallbackBase {
 public:
  virtual ~CallbackBase() = default;
  virtual void execute() = 0;
};

// 具体回调类（继承基类）
template <typename T, typename F>
class Callback;

// 非const成员函数特化
template <typename T, typename R, typename... Args>
class Callback<T, R (T::*)(Args...)> : public CallbackBase {
 public:
  using MemFuncPtr = R (T::*)(Args...);

  template <typename... Ts>
  Callback(MemFuncPtr func, T* instance, Ts&&... args)
      : func_(func), instance_(instance), args_(std::forward<Ts>(args)...) {}

  void Execute() override {
    std::apply(
        [this](Args&&... args) {
          [[maybe_unused]] auto&& result =
              (instance_->*func_)(std::forward<Args>(args)...);
        },
        args_);
  }

 private:
  MemFuncPtr func_;
  T* instance_;
  std::tuple<Args...> args_;
};

// const成员函数特化
template <typename T, typename R, typename... Args>
class Callback<T, R (T::*)(Args...) const> : public CallbackBase {
 public:
  using MemFuncPtr = R (T::*)(Args...) const;

  template <typename... Ts>
  Callback(MemFuncPtr func, T* instance, Ts&&... args)
      : func_(func), instance_(instance), args_(std::forward<Ts>(args)...) {}

  void Execute() override {
    std::apply(
        [this](Args&&... args) {
          [[maybe_unused]] auto&& result =
              (instance_->*func_)(std::forward<Args>(args)...);
        },
        args_);
  }

 private:
  MemFuncPtr func_;
  T* instance_;
  std::tuple<Args...> args_;
};

// 推导指引（简化模板参数推导）
template <typename T, typename R, typename... Args>
Callback(R (T::*)(Args...), T*, Args...) -> Callback<T, R (T::*)(Args...)>;

template <typename T, typename R, typename... Args>
Callback(R (T::*)(Args...) const, T*, Args...)
    -> Callback<T, R (T::*)(Args...) const>;

// 回调队列
class CallbackQueue {
 public:
  // 添加回调到队列
  void Enqueue(std::unique_ptr<CallbackBase> callback) {
    queue_.push(std::move(callback));
  }

  // 执行所有回调
  void ExecuteAll() {
    while (!queue_.empty()) {
      auto& callback = queue_.front();
      callback->execute();
      queue_.pop();
    }
  }

 private:
  std::queue<std::unique_ptr<CallbackBase>> queue_;
};

}  // namespace araneid

#endif