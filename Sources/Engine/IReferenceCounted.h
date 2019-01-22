/*
TiX Engine v2.0 Copyright (C) 2018~2019
By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TI_API IReferenceCounted
	{
	protected:

		//! Constructor.
		IReferenceCounted()
			: ReferenceCounter(0)
		{
		}

		//! Destructor.
		virtual ~IReferenceCounted()
		{
		}

	public:

		void grab() const
		{
			++ReferenceCounter;
		}

		bool drop() const
		{
			// someone is doing bad reference counting.
			TI_ASSERT(ReferenceCounter > 0);

			--ReferenceCounter;

			if (!ReferenceCounter)
			{
				delete this;
				return true;
			}

			return false;
		}

		unsigned int referenceCount() const
		{
			return ReferenceCounter;
		}

	protected:
		//! Called by drop just before deleting.
		/** Useful if some virtual calls are required to clean up. */
		//virtual void onDelete()
		//{
		//}
	protected:
		mutable unsigned int ReferenceCounter;
	};

	// boost::intrusive_ptr interface
	inline void intrusive_ptr_add_ref(const IReferenceCounted* obj)
	{
		obj->grab();
	}

	inline void intrusive_ptr_release(const IReferenceCounted* obj)
	{
		obj->drop();
	}

} // end namespace tix
