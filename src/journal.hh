#pragma once

#include <cstdio>
#include <ctime>
#include <string_view>
#include <format.hpp>

namespace journal {

    inline void write(FILE* fp, const std::string_view str, const std::string_view prefix, const std::string_view postfix) {
        using namespace std;

        const auto tv = time(nullptr);
        char timestamp[100];
        strftime(timestamp, sizeof timestamp, "%F %T", localtime(&tv));
        fprintf(fp, "%s %s: ", prefix.data(), timestamp);
        fputs(str.data(), fp);
        fputs(postfix.data(), fp);
        fflush(fp);
    }

    template<typename Arg, typename... Args>
    inline auto critical(const std::string_view fmt, Arg&& arg, Args&&... args) -> void {
        using namespace std;

        const auto str = xfmt::format(string{fmt}, forward<Arg>(arg), forward<Args>(args)...);

        write(stderr, str, "\x1b[39;41;1mCRITICAL", "\x1b[0m\n");
    }

    template<typename Arg, typename... Args>
    inline auto error(const std::string_view fmt, Arg&& arg, Args&&... args) -> void {
        using namespace std;

        const auto str = xfmt::format(string{fmt}, forward<Arg>(arg), forward<Args>(args)...);

        write(stderr, str, "\x1b[31;1mERROR", "\x1b[0m\n");
    }

    template<typename Arg, typename... Args>
    inline auto warning(const std::string_view fmt, Arg&& arg, Args&&... args) -> void {
        using namespace std;

        const auto str = xfmt::format(string{fmt}, forward<Arg>(arg), forward<Args>(args)...);

        write(stdout, str, "\x1b[33;1mWARNING", "\x1b[0m\n");
    }

    template<typename Arg, typename... Args>
    inline auto info(const std::string_view fmt, Arg&& arg, Args&&... args) -> void {
        using namespace std;

        const auto str = xfmt::format(string{fmt}, forward<Arg>(arg), forward<Args>(args)...);

        write(stdout, str, "INFO", "\x1b[0m\n");
    }

    template<typename Arg, typename... Args>
    inline auto debug(const std::string_view fmt, Arg&& arg, Args&&... args) -> void {
        using namespace std;

        const auto str = xfmt::format(string{fmt}, forward<Arg>(arg), forward<Args>(args)...);

        write(stdout, str, "\x1b[36mDEBUG", "\x1b[0m\n");
    }
} // namespace journal
