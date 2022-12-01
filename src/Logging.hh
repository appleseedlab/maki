#pragma once
#include <string>

namespace cpp2c
{
    static const constexpr bool Debug = false;
    static const constexpr char *delim = "\t";
    inline std::string fmt(std::string s) { return s; }
    inline std::string fmt(const char *s) { return std::string(s); }
    inline std::string fmt(bool b) { return b ? "T" : "F"; }
    inline std::string fmt(unsigned int i) { return std::to_string(i); }

    template <typename T>
    inline void print(T t) { llvm::outs() << fmt(t) << "\n"; }

    template <typename T1, typename T2, typename... Ts>
    inline void print(T1 t1, T2 t2, Ts... ts)
    {
        llvm::outs() << fmt(t1) << delim;
        print(t2, ts...);
    }

    inline void delimit() { print(delim); }

    template <typename... Ts>
    inline void debug(Ts... ts)
    {
        if (Debug)
            print(ts...);
    }

} // namespace cpp2c
