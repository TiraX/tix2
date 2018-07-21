/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
#ifndef BOOST_SMART_PTR_DETAIL_SP_NULLPTR_T_HPP_INCLUDED
#define BOOST_SMART_PTR_DETAIL_SP_NULLPTR_T_HPP_INCLUDED

	// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

	//  detail/sp_nullptr_t.hpp
	//
	//  Copyright 2013 Peter Dimov
	//
	//  Distributed under the Boost Software License, Version 1.0.
	//  See accompanying file LICENSE_1_0.txt or copy at
	//  http://www.boost.org/LICENSE_1_0.txt

#include <cstddef>

//#if !defined( BOOST_NO_CXX11_NULLPTR )

	namespace boost
	{

		namespace detail
		{

#if !defined( BOOST_NO_CXX11_DECLTYPE ) && ( ( defined( __clang__ ) && !defined( _LIBCPP_VERSION ) ) || defined( __INTEL_COMPILER ) )

			typedef decltype(nullptr) sp_nullptr_t;

#else

			typedef std::nullptr_t sp_nullptr_t;

#endif

		} // namespace detail

	} // namespace boost

//#endif // !defined( BOOST_NO_CXX11_NULLPTR )

#endif  // #ifndef BOOST_SMART_PTR_DETAIL_SP_NULLPTR_T_HPP_INCLUDED

	template<class T> class IInstrusivePtr
	{
	private:

		typedef IInstrusivePtr this_type;

	public:

		typedef T element_type;

		IInstrusivePtr() : px( 0 )
		{
		}

		IInstrusivePtr( T * p, bool add_ref = true ): px( p )
		{
			if( px != 0 && add_ref ) intrusive_ptr_add_ref( px );
		}

		template<class U>

		IInstrusivePtr( IInstrusivePtr<U> const & rhs )
		: px( rhs.get() )
		{
			if( px != 0 ) intrusive_ptr_add_ref( px );
		}

		IInstrusivePtr(IInstrusivePtr const & rhs): px( rhs.px )
		{
			if( px != 0 ) intrusive_ptr_add_ref( px );
		}

		~IInstrusivePtr()
		{
			if( px != 0 ) intrusive_ptr_release( px );
		}

		template<class U> IInstrusivePtr & operator=(IInstrusivePtr<U> const & rhs)
		{
			this_type(rhs).swap(*this);
			return *this;
		}

		IInstrusivePtr & operator=(IInstrusivePtr const & rhs)
		{
			this_type(rhs).swap(*this);
			return *this;
		}

		IInstrusivePtr & operator=(T * rhs)
		{
			this_type(rhs).swap(*this);
			return *this;
		}

		void reset()
		{
			this_type().swap( *this );
		}

		void reset( T * rhs )
		{
			this_type( rhs ).swap( *this );
		}

		void reset( T * rhs, bool add_ref )
		{
			this_type( rhs, add_ref ).swap( *this );
		}

		T * get() const
		{
			return px;
		}

		T * detach()
		{
			T * ret = px;
			px = 0;
			return ret;
		}

		T & operator*() const
		{
			TI_ASSERT( px != 0 );
			return *px;
		}

		T * operator->() const
		{
			TI_ASSERT( px != 0 );
			return px;
		}

		void swap(IInstrusivePtr & rhs)
		{
			T * tmp = px;
			px = rhs.px;
			rhs.px = tmp;
		}

	private:

		T * px;
	};

	template<class T, class U> inline bool operator==(IInstrusivePtr<T> const & a, IInstrusivePtr<U> const & b)
	{
		return a.get() == b.get();
	}

	template<class T, class U> inline bool operator!=(IInstrusivePtr<T> const & a, IInstrusivePtr<U> const & b)
	{
		return a.get() != b.get();
	}

	template<class T, class U> inline bool operator==(IInstrusivePtr<T> const & a, U * b)
	{
		return a.get() == b;
	}

	template<class T, class U> inline bool operator!=(IInstrusivePtr<T> const & a, U * b)
	{
		return a.get() != b;
	}

	template<class T, class U> inline bool operator==(T * a, IInstrusivePtr<U> const & b)
	{
		return a == b.get();
	}

	template<class T, class U> inline bool operator!=(T * a, IInstrusivePtr<U> const & b)
	{
		return a != b.get();
	}
	
//#if !defined( BOOST_NO_CXX11_NULLPTR )
	template<class T> inline bool operator==(IInstrusivePtr<T> const & p, boost::detail::sp_nullptr_t)
	{
		return p.get() == 0;
	}

	template<class T> inline bool operator==(boost::detail::sp_nullptr_t, IInstrusivePtr<T> const & p)
	{
		return p.get() == 0;
	}

	template<class T> inline bool operator!=(IInstrusivePtr<T> const & p, boost::detail::sp_nullptr_t)
	{
		return p.get() != 0;
	}

	template<class T> inline bool operator!=(boost::detail::sp_nullptr_t, IInstrusivePtr<T> const & p)
	{
		return p.get() != 0;
	}
//#endif !defined( BOOST_NO_CXX11_NULLPTR )

	template<class T> inline bool operator<(IInstrusivePtr<T> const & a, IInstrusivePtr<T> const & b)
	{
		return std::less<T *>()(a.get(), b.get());
	}

	template<class T> void swap(IInstrusivePtr<T> & lhs, IInstrusivePtr<T> & rhs)
	{
		lhs.swap(rhs);
	}

	// mem_fn support

	template<class T> T * get_pointer(IInstrusivePtr<T> const & p)
	{
		return p.get();
	}

	template<class T, class U> IInstrusivePtr<T> static_pointer_cast(IInstrusivePtr<U> const & p)
	{
		return static_cast<T *>(p.get());
	}

	template<class T, class U> IInstrusivePtr<T> const_pointer_cast(IInstrusivePtr<U> const & p)
	{
		return const_cast<T *>(p.get());
	}

	template<class T, class U> IInstrusivePtr<T> dynamic_pointer_cast(IInstrusivePtr<U> const & p)
	{
		return dynamic_cast<T *>(p.get());
	}
} // namespace tix
