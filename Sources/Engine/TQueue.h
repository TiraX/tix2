/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	template <class T>
	class TQueue
	{
	public:
		TQueue(int32 QueueSize = 512);
		virtual ~TQueue();

		virtual void	PushBack(const T& o);
		virtual void	PopFront(T& out);
		virtual void	GetFront(T& out);

		int32		GetSize();
		void		Resize(int32 NewSize);
	private:
		int			ContainerSize;
		T*			Queue;
		int			QueueSize;

		int			Head;
		int			Tail;
	};

	template <class T>
	TQueue<T>::TQueue(int32 QueueSize)
		: ContainerSize(QueueSize)
		, QueueSize(0)
		, Head(0)
		, Tail(0)
	{
		Queue = ti_new T[QueueSize];
		memset(Queue, 0, sizeof(T) * QueueSize);
	}

	template <class T>
	TQueue<T>::~TQueue()
	{
		SAFE_DELETE_ARRAY(Queue);
	}

	template <class T>
	void TQueue<T>::PushBack(const T& o)
	{
		if (QueueSize + 1 > ContainerSize)
		{
			Resize(ContainerSize * 2);
		}
		Queue[Tail] = o;
		++Tail;
		Tail %= ContainerSize;
		++QueueSize;
	}

	template <class T>
	void TQueue<T>::PopFront(T& out)
	{
		TI_ASSERT(QueueSize - 1 >= 0);
		out = Queue[Head];
		++Head;
		Head %= ContainerSize;
		--QueueSize;
	}

	template <class T>
	void TQueue<T>::GetFront(T& out)
	{
		TI_ASSERT(QueueSize - 1 >= 0);
		out = Queue[Head];
	}

	template <class T>
	int32 TQueue<T>::GetSize()
	{
		return	QueueSize;
	}

	template <class T>
	void TQueue<T>::Resize(int32 NewSize)
	{
		TI_ASSERT(NewSize >= GetSize());
		T* OldQueue = Queue;
		Queue = ti_new T[NewSize];
		memset(Queue, 0, NewSize * sizeof(T));
		// Move old data to new queue
		if (Head < Tail)
		{
			memcpy(OldQueue + Head, Queue, GetSize() * sizeof(T));
			Head = 0;
			Tail = GetSize();
		}
		else if (Head > Tail)
		{
			memcpy(OldQueue + Head, Queue, (ContainerSize - Head) * sizeof(T));
			memcpy(OldQueue, Queue + (ContainerSize - Head), Tail * sizeof(T));
			Head = 0;
			Tail = GetSize();
		}
		ContainerSize = NewSize;
	}

	//////////////////////////////////////////////////////////////////////////

	template <class T>
	class TThreadSafeQueue : public TQueue<T>
	{
	public:
		TThreadSafeQueue(int queue_size = 512);
		virtual ~TThreadSafeQueue();

		virtual void	PushBack(const T& o);
		virtual void	PopFront(T& out);

	private:
		TMutex		Mutex;
	};

	template <class T>
	TThreadSafeQueue<T>::TThreadSafeQueue(int queue_size)
		: TQueue<T>(queue_size)
	{
	}

	template <class T>
	TThreadSafeQueue<T>::~TThreadSafeQueue()
	{
	}

	template <class T>
	void TThreadSafeQueue<T>::PushBack(const T& o)
	{
		Mutex.Lock();
		TQueue<T>::PushBack(o);
		Mutex.Unlock();
	}

	template <class T>
	void TThreadSafeQueue<T>::PopFront(T& out)
	{
		Mutex.Lock();
		TQueue<T>::PopFront(out);
		Mutex.Unlock();
	}
}