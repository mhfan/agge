#pragma once

#include <agge/math.h>
#include <agge/types.h>

#include <algorithm>
#include <vector>

namespace agge
{
	template <typename T>
	class pod_vector;

	namespace tests
	{
		template <typename T, size_t n, int alignment_order = 4>
		class aligned_array
		{
		public:
			typedef T value_type;

		public:
			aligned_array();

			T *data();
			const T *data() const;
			T &operator [](size_t index);

			void fill(const T &value);

		private:
			union pod_constraint { T unused1; int unused2; };
			enum { alignment = 1 << alignment_order, alignment_mask = alignment_order ? alignment - 1 : 0 };

		private:
			aligned_array(const aligned_array &other);
			const aligned_array &operator =(const aligned_array &rhs);

			size_t aligned_offset() const;

		private:
			uint8_t _buffer[sizeof(T) * n + alignment];
		};



		template <typename T, size_t n, int alignment_order>
		inline aligned_array<T, n, alignment_order>::aligned_array()
		{	}

		template <typename T, size_t n, int alignment_order>
		inline T *aligned_array<T, n, alignment_order>::data()
		{	return reinterpret_cast<T *>(&_buffer[aligned_offset()]);	}

		template <typename T, size_t n, int alignment_order>
		inline const T *aligned_array<T, n, alignment_order>::data() const
		{	return reinterpret_cast<const T *>(&_buffer[aligned_offset()]);	}

		template <typename T, size_t n, int alignment_order>
		inline T &aligned_array<T, n, alignment_order>::operator [](size_t index)
		{	return *(data() + index);	}

		template <typename T, size_t n, int alignment_order>
		inline void aligned_array<T, n, alignment_order>::fill(const T &value)
		{	std::fill_n(data(), n, value);	}

		template <typename T, size_t n, int alignment_order>
		inline size_t aligned_array<T, n, alignment_order>::aligned_offset() const
		{	return (alignment - (reinterpret_cast<uintptr_t>(_buffer) & alignment_mask)) & alignment_mask;	}

		template <typename T, size_t n, int alignment_order>
		inline bool operator ==(const std::vector<T> &lhs, const aligned_array<T, n, alignment_order> &rhs)
		{	return lhs.size() == n && std::equal(lhs.begin(), lhs.end(), rhs.data());	}


		inline pixel32 make_pixel(uint8_t c0, uint8_t c1, uint8_t c2, uint8_t c3)
		{
			pixel32 p = { c0, c1, c2, c3 };
			return p;
		}

		template <typename T, size_t N>
		inline std::vector<T> mkvector(T (&p)[N])
		{	return std::vector<T>(p, p + N);	}

		template <typename T, size_t N>
		inline T *begin(T (&p)[N])
		{	return p;	}

		template <typename T, size_t N>
		inline T *end(T (&p)[N])
		{	return p + N;	}

		template <typename CoordT>
		inline rect<CoordT> mkrect_sized(CoordT left, CoordT top, CoordT width, CoordT height)
		{
			rect<CoordT> rc = { left, top, left + width, top + height };
			return rc;
		}

		template <typename T>
		bool equal(const T &lhs, const T &rhs)
		{	return lhs == rhs;	}

		template <>
		bool equal(const real_t &lhs, const real_t &rhs);
	}

	inline bool operator ==(const pixel32 &lhs, const pixel32 &rhs)
	{	return lhs.c0 == rhs.c0 && lhs.c1 == rhs.c1 && lhs.c2 == rhs.c2 && lhs.c3 == rhs.c3;	}

	template <typename T>
	inline bool operator ==(const point<T> &lhs, const point<T> &rhs)
	{	return tests::equal(lhs.x, rhs.x) && tests::equal(lhs.y, rhs.y);	}

	template <typename T>
	bool operator ==(const std::vector<T> &lhs, const pod_vector<T> &rhs)
	{	return lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin());	}
}
