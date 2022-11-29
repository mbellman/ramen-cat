#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace Gamma {
  template<typename T>
  struct Event {
    T data;

    Event(T data): data(data) {};
  };

  // @todo? use a lock for listener binding/dispatch thread-safety
  // @todo off(), off(eventName), off(eventName, listener)
  class Signaler {
  public:
    ~Signaler() {
      for (auto& [key, listeners] : listenerMap) {
        listeners.clear();
      }

      listenerMap.clear();
    }

    template<typename T>
    void on(std::string eventName, const std::function<void(T)>& listener) {
      listenerMap[eventName].push_back([=](void* event) {
        listener(((Event<T>*)event)->data);
      });
    }

  protected:
    template<typename T>
    void signal(std::string eventName, T data) {
      Event<T> event(data);
      auto& listeners = listenerMap[eventName];

      for (auto& listener : listeners) {
        listener(&event);
      }
    }

  private:
    std::map<std::string, std::vector<std::function<void(void*)>>> listenerMap;
  };
}