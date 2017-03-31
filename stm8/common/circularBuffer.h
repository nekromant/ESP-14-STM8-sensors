#pragma once
#include <stddef.h>
#include "type_traits.h"

template<size_t SIZE, typename DATA_T = unsigned char>
class CircularBuffer
{
public:
	typedef typename stdx::SelectSizeForLength<SIZE>::type INDEX_T;

private:
	static_assert((SIZE&(SIZE - 1)) == 0, "SIZE must be a power of 2");
	DATA_T _data[SIZE];
	volatile INDEX_T _readCount;
	volatile INDEX_T _writeCount;
	static const INDEX_T _mask = SIZE - 1;
public:

	bool Write(DATA_T value)
	{
		if(IsFull())
			return false;
		_data[_writeCount++ & _mask] = value;
		return true;
	}

	bool Read(DATA_T &value)
	{
		if(IsEmpty())
			return false;
		value = _data[_readCount++ & _mask];
		return true;
	}

	DATA_T First()const
	{
		return operator[](0);
	}

	DATA_T Last()const
	{
		return operator[](Count());
	}

	DATA_T& operator[] (INDEX_T i)
	{
		if(IsEmpty() || i > Count())
			return DATA_T();
		return _data[(_readCount + i) & _mask];
	}

	const DATA_T operator[] (INDEX_T i) const
	{
		if(IsEmpty() || i > Count())
			return DATA_T();
		return _data[(_readCount + i) & _mask];
	}

	bool IsEmpty() const
	{
		INDEX_T temp = _readCount;
		return _writeCount == temp;
	}

	bool IsFull() const
	{
		INDEX_T temp = _readCount;
		return ((_writeCount - temp) & (INDEX_T)~(_mask)) != 0;
	}

	INDEX_T Count() const
	{
		return (_writeCount - _readCount) & _mask;
	}

	void Clear()
	{
		_readCount = 0;
		_writeCount = 0;
	}

	INDEX_T Size()
	{
		return SIZE;
	}
};
