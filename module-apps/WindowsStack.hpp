#pragma once

#include <memory>
#include <map>
#include <vector>
#include <string>
#include <AppWindow.hpp>

namespace app
{

    class Application;

    class WindowsStack
    {
        Application *parent;

      public:
        WindowsStack(Application *parent) : parent(parent)
        {}

        std::vector<std::string> stack;
        std::map<std::string, std::unique_ptr<gui::AppWindow>> windows;

        std::map<std::string, std::unique_ptr<gui::AppWindow>>::const_iterator begin() const
        {
            return std::begin(windows);
        }

        std::map<std::string, std::unique_ptr<gui::AppWindow>>::const_iterator end() const
        {
            return std::end(windows);
        }

        [[nodiscard]] auto getParent() const
        {
            return parent;
        }

        auto push(const std::string &name, std::unique_ptr<gui::AppWindow> window)
        {
            windows[name] = std::move(window);
            stack.push_back(name);
        }

        gui::AppWindow *get(const std::string &name)
        {
            auto ret = windows.find(name);
            return ret == std::end(windows) ? nullptr : ret->second.get();
        }
    };
} // namespace app
