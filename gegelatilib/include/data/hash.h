/**
 * Copyright or © or Copr. IETR/INSA - Rennes (2020) :
 *
 * Karol Desnos <kdesnos@insa-rennes.fr> (2020)
 *
 * GEGELATI is an open-source reinforcement learning framework for training
 * artificial intelligence based on Tangled Program Graphs (TPGs).
 *
 * This software is governed by the CeCILL-C license under French law and
 * abiding by the rules of distribution of free software. You can use,
 * modify and/ or redistribute the software under the terms of the CeCILL-C
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info".
 *
 * As a counterpart to the access to the source code and rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty and the software's author, the holder of the
 * economic rights, and the successive licensors have only limited
 * liability.
 *
 * In this respect, the user's attention is drawn to the risks associated
 * with loading, using, modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean that it is complicated to manipulate, and that also
 * therefore means that it is reserved for developers and experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or
 * data to be ensured and, more generally, to use and operate it in the
 * same conditions as regards security.
 *
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL-C license and that you accept its terms.
 */

/**
* Code from MSVC 19 implementation of std::hash.
* Published under license Apache-2.0 WITH LLVM-exception
*/
#ifndef HASH_H
#define HASH_H

#include <cstddef>
#include <type_traits>

namespace Data {

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#ifndef _NODISCARD
#define _NODISCARD [[nodiscard]]
#endif

#if defined(_WIN64) || defined(__x86_64__)
	inline constexpr size_t _FNV_offset_basis = 14695981039346656037ULL;
	inline constexpr size_t _FNV_prime = 1099511628211ULL;
#else // defined(_WIN64)
	inline constexpr size_t _FNV_offset_basis = 2166136261U;
	inline constexpr size_t _FNV_prime = 16777619U;
#endif // defined(_WIN64)

	_NODISCARD inline size_t _Fnv1a_append_bytes(size_t _Val, const unsigned char* const _First,
		const size_t _Count) noexcept { // accumulate range [_First, _First + _Count) into partial FNV-1a hash _Val
		for (size_t _Idx = 0; _Idx < _Count; ++_Idx) {
			_Val ^= static_cast<size_t>(_First[_Idx]);
			_Val *= _FNV_prime;
		}

		return _Val;
	}

	template <class _Kty>
	_NODISCARD size_t _Fnv1a_append_value(
		const size_t _Val, const _Kty& _Keyval) noexcept { // accumulate _Keyval into partial FNV-1a hash _Val
		static_assert(std::is_trivial_v<_Kty>, "Only trivial types can be directly hashed.");
		return _Fnv1a_append_bytes(_Val, &reinterpret_cast<const unsigned char&>(_Keyval), sizeof(_Kty));
	}

	// FUNCTION TEMPLATE _Hash_representation
	template <class _Kty>
	_NODISCARD size_t _Hash_representation(const _Kty& _Keyval) noexcept { // bitwise hashes the representation of a key
		return _Fnv1a_append_value(_FNV_offset_basis, _Keyval);
	}

	// STRUCT TEMPLATE _Conditionally_enabled_hash
	template<class _Kty>
	struct Hash;

	template<class _Kty, bool _Enabled>
	struct _Conditionally_enabled_hash { // conditionally enabled hash base
		using argument_type = _Kty;
		using result_type = size_t;

		_NODISCARD size_t

			operator()(const _Kty& _Keyval) const
			noexcept(noexcept(Hash<_Kty>::_Do_hash(_Keyval))) /* strengthened */ {
			return Hash<_Kty>::_Do_hash(_Keyval);
		}
	};

	// STRUCT TEMPLATE hash
	template<class _Kty>
	struct Hash
		: _Conditionally_enabled_hash<_Kty,
		!std::is_const_v < _Kty> && !
		std::is_volatile_v <_Kty> && (std::is_enum_v<_Kty>
			|| std::is_integral_v <_Kty> || std::is_pointer_v <_Kty>)> {
		// hash functor primary template (handles enums, integrals, and pointers)
		static size_t _Do_hash(const _Kty& _Keyval) noexcept {
			return _Hash_representation(_Keyval);
		}
	};

	template<>
	struct Hash<float> {
		using argument_type = float;
		using result_type = size_t;
		_NODISCARD size_t

			operator()(const float _Keyval) const noexcept {
			return _Hash_representation(_Keyval == 0.0F ? 0.0F : _Keyval); // map -0 to 0
		}
	};

	template<>
	struct Hash<double> {
		using argument_type = double;
		using result_type = size_t;
		_NODISCARD size_t

			operator()(const double _Keyval) const noexcept {
			return _Hash_representation(_Keyval == 0.0 ? 0.0 : _Keyval); // map -0 to 0
		}
	};

	template<>
	struct Hash<nullptr_t> {
		using argument_type = nullptr_t;
		using result_type = size_t;
		_NODISCARD size_t

			operator()(nullptr_t) const noexcept {
			void* _Null{};
			return _Hash_representation(_Null);
		}
	};
#endif // DOXYGEN_SHOULD_SKIP_THIS
}

#endif 
