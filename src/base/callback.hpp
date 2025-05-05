#ifndef ARANEID_BASE_CALLBACK_HPP
#define ARANEID_BASE_CALLBACK_HPP

#include <iostream>
#include <memory>
#include <queue>
#include <tuple>
#include <utility>
namespace araneid {

class CallbackBase {
 public:
  virtual ~CallbackBase() = default;
  virtual void Execute() = 0;
};

// Concrete callback class (inherits from base class)
template <typename T, typename F>
class Callback;

// Specialization for non-const member functions
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
          if constexpr (std::is_void_v<R>) {  // C++17 起支持
            (instance_->*func_)(std::forward<Args>(args)...);
          } else {
            [[maybe_unused]] auto&& result =
                (instance_->*func_)(std::forward<Args>(args)...);
          }
        },
        args_);
  }

 private:
  MemFuncPtr func_;
  T* instance_;
  std::tuple<Args...> args_;
};

// Specialization for const member functions
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

// Deduction guides (simplify template parameter deduction)
template <typename T, typename R, typename... Args>
Callback(R (T::*)(Args...), T*, Args...) -> Callback<T, R (T::*)(Args...)>;

template <typename T, typename R, typename... Args>
Callback(R (T::*)(Args...) const, T*, Args...)
    -> Callback<T, R (T::*)(Args...) const>;

}  // namespace araneid

#endif  // ARANEID_BASE_CALLBACK_HPP