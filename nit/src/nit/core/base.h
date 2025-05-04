#pragma once

#ifdef _WIN32
#ifdef _WIN64
#define NIT_PLATFORM_WINDOWS
#else
        #error "x86 Builds are not supported!"
#endif
#elif defined(__APPLE__) || defined(__MACH__)
    #define NIT_PLATFORM_APPLE
    #error "Apple is not supported!"
#elif defined(__ANDROID__)
    #define NIT_PLATFORM_ANDROID
    #error "Android is not supported!"
#elif defined(__linux__)
    #define NIT_PLATFORM_LINUX
#else
    #error "Unknown platform!"
#endif


//#TODO_asuarez build a real log system
#include <cstdio>
#include <stack>
#define NIT_PRINT(...) \
printf(__VA_ARGS__)

#define NIT_PRINTLN(...) \
NIT_PRINT(__VA_ARGS__); NIT_PRINT("\n")

#if defined NIT_DEBUG || NIT_RELEASE

#define NIT_LOG_TRACE(...) \
NIT_PRINT("\x1B[96m"); NIT_PRINTLN(__VA_ARGS__); NIT_PRINT("\033[0m")

#define NIT_LOG_WARN(...) \
NIT_PRINT("\x1B[93m"); NIT_PRINTLN(__VA_ARGS__); NIT_PRINT("\033[0m")

#define NIT_LOG_ERR(...) \
NIT_PRINT("\x1B[91m"); NIT_PRINTLN(__VA_ARGS__); NIT_PRINT("\033[0m")

#define NIT_EDITOR_ENABLED

#else

#define NIT_LOG(...)
#define NIT_LOG_TRACE(...) 
#define NIT_LOG_WARN(...)
#define NIT_LOG_ERR(...)

#endif

#ifdef NIT_EDITOR_ENABLED
#define NIT_IF_EDITOR_ENABLED(_LINE) _LINE
#define NIT_IMGUI_ENABLED
#else
#define NIT_IF_EDITOR_ENABLED(_LINE)
#endif

#ifdef NIT_DEBUG
#if defined(NIT_PLATFORM_WINDOWS)
    #define NIT_DEBUGBREAK() __debugbreak()
#elif defined(NIT_PLATFORM_LINUX)
    #include <signal.h>
    #define NIT_DEBUGBREAK() raise(SIGTRAP)
#else
    #error "Platform doesn't support debugbreak yet!"
#endif
#define NIT_ENABLE_CHECKS
#else
#define NIT_DEBUGBREAK()
#endif

#ifdef NIT_ENABLE_CHECKS
#define NIT_CHECK_MSG(_CONDITION, ...) \
if (!(_CONDITION)) { NIT_LOG_ERR(__VA_ARGS__); NIT_DEBUGBREAK(); }

#define NIT_CHECK(_CONDITION) \
if (!(_CONDITION)) {  NIT_DEBUGBREAK(); }
#else
#define NIT_CHECK_MSG(_CONDITION, ...)
#define NIT_CHECK(_CONDITION)
#endif


using i16 = short;
using i32 = int;
using i64 = long long;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;

using f32 = float;
using f64 = double;

inline constexpr u32 U32_MAX = std::numeric_limits<u32>::max();
inline constexpr f32 F32_MAX = std::numeric_limits<f32>::max();
inline constexpr f32 F32_EPSILON = std::numeric_limits<f32>::epsilon();

using VoidFunc = void(*)();

template <typename T>
using Function = std::function<T>;

template <typename T, size_t N>
using FixedArray = std::array<T, N>;

template <typename T>
using Array = std::vector<T>;

template <typename T>
using UnorderedSet = std::unordered_set<T>;

template <typename K, typename V>
using OrderedMap = std::map<K, V>;

template <typename K, typename V>
using Map = std::unordered_map<K, V>;

template <typename T>
using Queue = std::queue<T>;

template <typename T>
using Stack = std::stack<T>;

template <typename T>
using Set = std::set<T>;

template <typename K, typename V>
using Pair = std::pair<K, V>;

template <std::size_t N>
using Bitset = std::bitset<N>;

using String = std::string;

using StringStream = std::stringstream;

using IStringStream = std::istringstream;

using OutputFile = std::ofstream;

using InputFile = std::ifstream;

using Path = std::filesystem::path;

using RecursiveDirectoryIterator = std::filesystem::recursive_directory_iterator;

template <typename T>
using UniquePtr = std::unique_ptr<T>;

template <typename T, typename... Args>
constexpr UniquePtr<T> CreateUniquePtr(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template <typename T>
using SharedPtr = std::shared_ptr<T>;

template <typename T, typename... Args>
constexpr SharedPtr<T> CreateSharedPtr(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T>
using WeakPtr = std::weak_ptr<T>;

template <class OutIt, class Diff, class T>
OutIt FillRaw(OutIt destination, const Diff count, const T& value) { return std::fill_n(destination, count, value); }

inline bool Replace(String& str, const String& from, const String& to)
{
    u64 start_pos = str.find(from);

    if (start_pos == String::npos)
    {
        return false;
    }

    str.replace(start_pos, from.length(), to);

    return true;
}

inline Path GetWorkingDirectory()
{
    return std::filesystem::current_path();
}

namespace nit { struct Window; }

String BrowseFile(const nit::Window* window, const String& filter);
String SaveFile(const nit::Window* window, const String& filter);
String SelectFolder(const nit::Window* window, const String& title = "", const String& path = "");
String ExecuteCMD(const char* cmd);

bool StringContains(const String& text, const String& contain);

#define NIT_GRAPHICS_API_OPENGL
