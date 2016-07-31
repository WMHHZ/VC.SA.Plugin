#pragma once
#include <cstdint>
#include <type_traits>

template <typename Prototype>
class cdecl_func_wrapper;

template <typename Prototype>
class stdcall_func_wrapper;

template <typename Prototype>
class thiscall_func_wrapper;

template <typename __Ret, typename ...__Args>
class cdecl_func_wrapper<__Ret(__Args...)>
{
	void *pfunction;

public:
	cdecl_func_wrapper() :pfunction(nullptr) {}
	cdecl_func_wrapper(void *pfunc) :pfunction(pfunc) {}
	cdecl_func_wrapper(std::uintptr_t funcaddr) :pfunction((void *)funcaddr) {}
	cdecl_func_wrapper &operator=(void *pfunc) { pfunction = pfunc; }
	cdecl_func_wrapper &operator=(std::uintptr_t funcaddr) { pfunction = (void *)funcaddr; }

	__Ret operator()(__Args ... args) const
	{
		return ((__Ret(__cdecl *)(__Args...))(pfunction))(std::forward<__Args>(args)...);
	}
};

template <typename __Ret, typename ...__Args>
class stdcall_func_wrapper<__Ret(__Args...)>
{
	void *pfunction;

public:
	stdcall_func_wrapper() :pfunction(nullptr) {}
	stdcall_func_wrapper(void *pfunc) :pfunction(pfunc) {}
	stdcall_func_wrapper(std::uintptr_t funcaddr) :pfunction((void *)funcaddr) {}
	stdcall_func_wrapper &operator=(void *pfunc) { pfunction = pfunc; }
	stdcall_func_wrapper &operator=(std::uintptr_t funcaddr) { pfunction = (void *)funcaddr; }

	__Ret operator()(__Args ... args) const
	{
		return ((__Ret(__stdcall *)(__Args...))(pfunction))(std::forward<__Args>(args)...);
	}
};

template <typename __Ret, typename ...__Args>
class thiscall_func_wrapper<__Ret(__Args...)>
{
	void *pfunction;

public:
	thiscall_func_wrapper() :pfunction(nullptr) {}
	thiscall_func_wrapper(void *pfunc) :pfunction(pfunc) {}
	thiscall_func_wrapper(std::uintptr_t funcaddr) :pfunction((void *)funcaddr) {}
	thiscall_func_wrapper &operator=(void *pfunc) { pfunction = pfunc; }
	thiscall_func_wrapper &operator=(std::uintptr_t funcaddr) { pfunction = (void *)funcaddr; }

	__Ret operator()(void *__this, __Args... args) const
	{
		return ((__Ret(__thiscall *)(void *, __Args...))(pfunction))(__this, std::forward<__Args>(args)...);
	}
};
