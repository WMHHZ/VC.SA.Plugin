#pragma once
#include <windows.h>
#include <cstdint>
#include <cstddef>
#include <initializer_list>
#include <type_traits>

namespace addr_sel
{
	template <::std::uintptr_t... addresses>
	struct addresses_list
	{};

	template <typename>
	struct is_addresses_list : ::std::false_type 
	{};

	template <>
	struct is_addresses_list<addresses_list<> > : ::std::false_type
	{};

	template<::std::uintptr_t... addresses>
	struct is_addresses_list<addresses_list<addresses...> > : ::std::true_type
	{};

	static bool list_contains_address(::std::uintptr_t address, addresses_list<>)
	{
		return false;
	}

	template <::std::uintptr_t first, ::std::uintptr_t... rest>
	static bool list_contains_address(::std::uintptr_t address, addresses_list<first, rest...>)
	{
		return ((address == first) || (list_contains_address(address, addresses_list<rest...>())));
	}

	template <int result, class... entry_points_list>
	struct init_impl;

	template <int result, class first, class... rest>
	struct init_impl<result, first, rest...>
	{
		static_assert(is_addresses_list<first>::value, "Incorrect template argument.");

		static int work(::std::uintptr_t address)
		{
			if (list_contains_address(address, first()))
			{
				return result;
			}
			else
			{
				return init_impl<result + 1, rest...>::work(address);
			}
		}
	};

	template <int result, class last>
	struct init_impl<result, last>
	{
		static_assert(is_addresses_list<last>::value, "Incorrect template argument.");

		static int work(::std::uintptr_t address)
		{
			if (list_contains_address(address, last()))
			{
				return result;
			}
			else
			{
				return -1;
			}
		}
	};

	template <class... entry_points_list>
	class address_selector
	{
	private:
		int m_index;

	private:
		static const address_selector &get_instance()
		{
			static address_selector instance;

			return instance;
		}

		void init()
		{
			std::uintptr_t base = (std::uintptr_t)GetModuleHandleW(NULL);
			IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER *)(base);
			IMAGE_NT_HEADERS *nt = (IMAGE_NT_HEADERS *)(base + dos->e_lfanew);

			DWORD ep = nt->OptionalHeader.AddressOfEntryPoint + 0x400000;

			m_index = init_impl<0, entry_points_list...>::work(ep);
		}

		address_selector()
		{
			init();
		}

	public:
		template <typename dest_type = void>
		static dest_type *select_address(::std::initializer_list<::std::uintptr_t> list)
		{
			auto index = get_instance().m_index;

			if ((index < 0) || (list.size() != sizeof...(entry_points_list)))
			{
				return nullptr;
			}

			return reinterpret_cast<dest_type *>(list.begin()[index]);
		}
	};

	typedef address_selector<addresses_list<0x667BF0>, addresses_list<0x667C40>, addresses_list<0xA402ED, 0x666BA0> > vc;
}
