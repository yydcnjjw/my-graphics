#pragma once

#include <type.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <boost/format.hpp>
#include <boost/noncopyable.hpp>
#include <utility>

namespace my {
namespace platform {
namespace glfw {

static inline auto err_str() {
  char const *str;
  glfwGetError(&str);
  return str;
}

static inline auto err_code() { return glfwGetError(nullptr); }

class Context {
public:
  Context() {
    if (glfwInit() == GLFW_FALSE) {
      throw std::runtime_error(err_str());
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwSetErrorCallback(&Context::error_cb);
  }

  void poll_events() { glfwPollEvents(); }

  ~Context() { glfwTerminate(); }

private:
  static void error_cb(int error, const char *description) {
    SPDLOG_ERROR(description);
  }
};

struct Size {
  int width;
  int height;
};

class Window : private boost::noncopyable {
public:
  using handle_type = GLFWwindow *;

  template <typename... Args> Window(Args &&...args) {
    _handle = glfwCreateWindow(std::forward<Args>(args)...);
    if (!_handle) {
      throw std::runtime_error(err_str());
    }

    glfwSetWindowUserPointer(*this, this);
    glfwSetFramebufferSizeCallback(*this, &Window::framebuffer_size_cb);
  }

  ~Window() { glfwDestroyWindow(_handle); }

  auto should_close() { return glfwWindowShouldClose(*this); }

  void swapbuffer() { glfwSwapBuffers(*this); }

  void make_glctx() { glfwMakeContextCurrent(*this); }

  operator handle_type() { return _handle; }

private:
  handle_type _handle;

  template <typename EventType, typename... Args>
  void post_event(Args &&...args) {}

  static Window *user_pointer(handle_type handle) {
    return reinterpret_cast<Window *>(glfwGetWindowUserPointer(handle));
  }

  template <typename EventType, typename... Args>
  static void post_event(handle_type handle, Args &&...args) {
    user_pointer(handle)->post_event<EventType>(std::forward<Args>(args)...);
  }

  static void framebuffer_size_cb(handle_type handle, int w, int h) {}
};

} // namespace glfw
} // namespace platform
} // namespace my
