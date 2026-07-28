#pragma once

#include <stdexcept>
#include <string>
#include <utility>

#if defined(_WIN32) || defined(_WIN64)
#ifndef NOGDI
#define NOGDI
#define PATTERN_DYLIB_UNDEF_NOGDI
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#define PATTERN_DYLIB_UNDEF_WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifdef PATTERN_DYLIB_UNDEF_WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#undef PATTERN_DYLIB_UNDEF_WIN32_LEAN_AND_MEAN
#endif
#ifdef PATTERN_DYLIB_UNDEF_NOGDI
#undef NOGDI
#undef PATTERN_DYLIB_UNDEF_NOGDI
#endif
#else
#include <dlfcn.h>
#endif

class dylib {
public:
	class exception : public std::runtime_error {
	public:
		explicit exception(const std::string& message) : std::runtime_error(message) {}
	};

	dylib(const dylib&) = delete;
	dylib& operator=(const dylib&) = delete;

	dylib(dylib&& other) noexcept : handle_(other.handle_) {
		other.handle_ = nullptr;
	}

	dylib& operator=(dylib&& other) noexcept {
		if (this != &other) {
			close();
			handle_ = other.handle_;
			other.handle_ = nullptr;
		}
		return *this;
	}

	dylib(const std::string& path, const std::string& lib) {
		const std::string library_path = join_path(path, decorate_name(lib));
		handle_ = open(library_path);
		if (!handle_) {
			throw exception("Could not load library \"" + library_path + "\": " + last_error());
		}
	}

	~dylib() {
		close();
	}

	template<typename function_type>
	function_type* get_function(const std::string& name) const {
		const void* symbol = get_symbol(name);
		return reinterpret_cast<function_type*>(const_cast<void*>(symbol));
	}

private:
#if defined(_WIN32) || defined(_WIN64)
	using handle_type = HMODULE;
#else
	using handle_type = void*;
#endif

	handle_type handle_ = nullptr;

	static std::string decorate_name(const std::string& lib) {
#if defined(_WIN32) || defined(_WIN64)
		return lib + ".dll";
#elif defined(__APPLE__)
		return "lib" + lib + ".dylib";
#else
		return "lib" + lib + ".so";
#endif
	}

	static std::string join_path(const std::string& path, const std::string& lib) {
		if (path.empty()) return lib;
		const char last_character = path.back();
		if (last_character == '/' || last_character == '\\') return path + lib;
#if defined(_WIN32) || defined(_WIN64)
		return path + "\\" + lib;
#else
		return path + "/" + lib;
#endif
	}

	static handle_type open(const std::string& path) {
#if defined(_WIN32) || defined(_WIN64)
		return LoadLibraryA(path.c_str());
#else
		return dlopen(path.c_str(), RTLD_NOW);
#endif
	}

	const void* get_symbol(const std::string& name) const {
#if defined(_WIN32) || defined(_WIN64)
		FARPROC symbol = GetProcAddress(handle_, name.c_str());
		if (!symbol) {
			throw exception("Could not get function \"" + name + "\": " + last_error());
		}
		return reinterpret_cast<const void*>(symbol);
#else
		dlerror();
		void* symbol = dlsym(handle_, name.c_str());
		const char* error = dlerror();
		if (error) {
			throw exception("Could not get function \"" + name + "\": " + error);
		}
		return symbol;
#endif
	}

	static std::string last_error() {
#if defined(_WIN32) || defined(_WIN64)
		return "Windows error " + std::to_string(GetLastError());
#else
		const char* error = dlerror();
		return error ? error : "unknown error";
#endif
	}

	void close() noexcept {
		if (!handle_) return;
#if defined(_WIN32) || defined(_WIN64)
		FreeLibrary(handle_);
#else
		dlclose(handle_);
#endif
		handle_ = nullptr;
	}
};