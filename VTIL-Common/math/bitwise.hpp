// Copyright (c) 2020 Can Boluk and contributors of the VTIL Project   
// All rights reserved.   
//    
// Redistribution and use in source and binary forms, with or without   
// modification, are permitted provided that the following conditions are met: 
//    
// 1. Redistributions of source code must retain the above copyright notice,   
//    this list of conditions and the following disclaimer.   
// 2. Redistributions in binary form must reproduce the above copyright   
//    notice, this list of conditions and the following disclaimer in the   
//    documentation and/or other materials provided with the distribution.   
// 3. Neither the name of VTIL Project nor the names of its contributors
//    may be used to endorse or promote products derived from this software 
//    without specific prior written permission.   
//    
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE   
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE  
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE   
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR   
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF   
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS   
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN   
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)   
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE  
// POSSIBILITY OF SUCH DAMAGE.        
//
#pragma once
#include <stdint.h>
#include <math.h>
#include <optional>
#include <type_traits>
#include <numeric>
#include "../util/reducable.hpp"
#include "../io/asserts.hpp"
#include "../util/type_helpers.hpp"
#include "../util/intrinsics.hpp"

// Declare the type we will used for bit lenghts of data.
// - We are using int instead of char since most operations will end up casting
//   this value to an integer anyway and since char does not provide us any intrinsic
//   safety either this only hurts us in terms of performance.
//
using bitcnt_t = int;

namespace vtil::math
{
	// Narrows the given type in a safe manner.
	//
	template<Integral T, Integral T2> requires( sizeof( T ) <= sizeof( T2 ) )
	__forceinline static constexpr T narrow_cast( T2 o )
	{
		if constexpr ( std::is_signed_v<T2> ^ std::is_signed_v<T> )
			dassert( 0 <= o && o <= std::numeric_limits<T>::max() );
		else
			dassert( std::numeric_limits<T>::min() <= o && o <= std::numeric_limits<T>::max() );

		return ( T ) o;
	}
	template<Integral T, Integral T2> requires( sizeof( T ) <= sizeof( T2 ) )
	__forceinline static constexpr T narrow_cast_s( T2 o )
	{
		return ( T ) std::clamp( 
			o, 
			std::min( ( T2 ) std::numeric_limits<T>::min(), std::numeric_limits<T2>::min() ), 
			( T2 ) std::numeric_limits<T>::max() 
		);
	}

	// Extracts the sign bit from the given value.
	//
	template<Integral T>
	__forceinline static constexpr bool sgn( T type ) { return bool( type >> ( ( sizeof( T ) * 8 ) - 1 ) ); }

	// Micro-optimized implementation to trick MSVC into actually optimizing it, like Clang :).
	//
	namespace impl
	{
		__forceinline static constexpr bitcnt_t rshiftcnt( bitcnt_t n )
		{
			// If MSVC x86-64:
			//
#if MS_COMPILER && AMD64_TARGET && !defined(__INTELLISENSE__)
			// SAR/SHR/SHL will ignore anything besides [x % 64], which lets us 
			// optimize (64 - n) into [-n] by substracting modulo size {64}.
			//
			if ( !std::is_constant_evaluated() )
				return -n & 63;
#endif
			return 64 - n;
		}
	};

	// Implement platform-indepdenent bitwise operations.
	//
	__forceinline static constexpr bitcnt_t popcnt( uint64_t x )
	{
		// Optimized using intrinsics if not const evaluated.
		//
		if ( !std::is_constant_evaluated() )
		{
#if MS_COMPILER && AMD64_TARGET
			return ( bitcnt_t ) __popcnt64( x );
#elif __has_builtin(__builtin_popcountll)
			return ( bitcnt_t ) __builtin_popcountll( x );
#endif
		}
		bitcnt_t count = 0;
		for ( bitcnt_t i = 0; i < 64; i++, x >>= 1 )
			count += ( bitcnt_t ) ( x & 1 );
		return count;
	}
	__forceinline static constexpr bitcnt_t msb( uint64_t x )
	{
		// Optimized using intrinsics if not const evaluated.
		//
		if ( !std::is_constant_evaluated() )
		{
#if MS_COMPILER && AMD64_TARGET
			unsigned long idx;
			return _BitScanReverse64( &idx, x ) ? idx : -1;
#elif __has_builtin(__builtin_ctzll)
			return x ? 63 - __builtin_clzll( x ) : -1;
#endif
		}

		// Start scan loop, return idx if found, else -1.
		//
		for ( bitcnt_t i = 63; i >= 0; i-- )
			if ( x & ( 1ull << i ) )
				return i;
		return -1;
	}
	__forceinline static constexpr bitcnt_t lsb( uint64_t x )
	{
		// Optimized using intrinsics if not const evaluated.
		//
		if ( !std::is_constant_evaluated() )
		{
#if MS_COMPILER && AMD64_TARGET
			unsigned long idx;
			return _BitScanForward64( &idx, x ) ? idx : -1;
#elif __has_builtin(__builtin_ctzll)
			return x ? __builtin_ctzll( x ) : -1;
#endif
		}

		// Start scan loop, return idx if found, else -1.
		//
		for ( bitcnt_t i = 0; i <= 63; i++ )
			if ( x & ( 1ull << i ) )
				return i;
		return -1;
	}
	__forceinline static constexpr bool bit_test( uint64_t value, bitcnt_t n )
	{
		// _bittest64 forcefully writes to memory for no reason, let the compilers 
		// generate bt reg, reg from this as expected.
		//
		return value & ( 1ull << n );
	}
	__forceinline static constexpr bool bit_set( uint64_t& value, bitcnt_t n )
	{
		// Optimized using intrinsics if not const evaluated.
		//
#if MS_COMPILER && AMD64_TARGET
		if ( !std::is_constant_evaluated() )
			return _bittestandset64( ( long long* ) &value, n );
#endif
		const uint64_t mask = ( 1ull << n );
		bool is_set = value & mask;
		value |= mask;
		return is_set;
	}
	__forceinline static constexpr bool bit_reset( uint64_t& value, bitcnt_t n )
	{
		// Optimized using intrinsics if not const evaluated.
		//
#if MS_COMPILER && AMD64_TARGET
		if ( !std::is_constant_evaluated() )
			return _bittestandreset64( ( long long* ) &value, n );
#endif
		const uint64_t mask = ( 1ull << n );
		bool is_set = value & mask;
		value &= ~mask;
		return is_set;
	}

	// Used to find a bit with a specific value in a linear memory region.
	//
	static constexpr size_t bit_npos = ( size_t ) -1;
	template<typename T>
	static constexpr size_t bit_find( const T* begin, const T* end, bool value, bool reverse = false )
	{
		constexpr size_t bit_size = sizeof( T ) * 8;
		using uint_t = std::make_unsigned_t<T>;
		using int_t =  std::make_signed_t<T>;
		const auto scanner = reverse ? msb : lsb;

		// Generate the xor mask, if we're looking for 1, -!1 will evaluate to 0,
		// otherwise -!0 will evaluate to 0xFF.. in order to flip all bits.
		//
		uint_t xor_mask = ( uint_t ) ( -( ( int_t ) !value ) );

		// Loop each block:
		//
		size_t n = 0;
		for ( auto it = begin; it != end; it++, n += bit_size )
		{
			// Return if we could find the bit in the block:
			//
			if ( bitcnt_t i = scanner( *it ^ xor_mask ); i >= 0 )
				return n + i;
		}

		// Return invalid index.
		//
		return bit_npos;
	}

	// Used to enumerate each set bit in the integer.
	//
	template<typename T>
	static constexpr void bit_enum( uint64_t mask, T&& fn, bool reverse = false )
	{
		const auto scanner = reverse ? msb : lsb;
		while ( true )
		{
			// If scanner returns negative, break.
			//
			bitcnt_t idx = scanner( mask );
			if ( idx < 0 ) return;

			// Reset the bit and invoke the callback.
			//
			bit_reset( mask, idx );
			fn( idx );
		}
	}

	// Generate a mask for the given variable size and offset.
	//
	__forceinline static constexpr uint64_t fill( bitcnt_t bit_count, bitcnt_t bit_offset = 0 )
	{
		dassert( bit_count <= 64 );

		// If bit offset is negative, substract from bit count 
		// and zero it out.
		//
		bit_count += bit_offset < 0 ? bit_offset : 0; // CMOV
		bit_offset = bit_offset < 0 ? 0 : bit_offset; // CMOV

		// Provide constexpr safety.
		//
		if ( std::is_constant_evaluated() )
			if( bit_count <= 0 || bit_offset >= 64 )
				return 0;
		
		// Create the value by two shifts.
		//
		uint64_t value = bit_count > 0 ? ~0ull : 0; // CMOV
		value >>= impl::rshiftcnt( bit_count );
		value <<= bit_offset;

		// If offset overflows, zero out, else return.
		//
		return bit_offset >= 64 ? 0 : value; // CMOV
	}

	// Fills the bits of the uint64_t type after the given offset with the sign bit.
	// - We accept an [uint64_t] as the sign "bit" instead of a for 
	//   the sake of a further trick we use to avoid branches.
	//
	__forceinline static constexpr uint64_t fill_sign( uint64_t sign, bitcnt_t bit_offset = 0 )
	{
		// The XOR operation with 0b1 flips the sign bit, after which when we subtract
		// one to create 0xFF... for (1) and 0x00... for (0).
		//
		return ( ( sign ^ 1 ) - 1 ) << bit_offset;
	}

	// Extends the given integral type into uint64_t or int64_t.
	//
	template<Integral T>
	__forceinline static constexpr auto imm_extend( T imm )
	{
		if constexpr ( std::is_signed_v<T> ) return ( int64_t ) imm;
		else                                 return ( uint64_t ) imm;
	}

	// Zero extends the given integer.
	//
	__forceinline static constexpr uint64_t zero_extend( uint64_t value, bitcnt_t bcnt_src )
	{
		dassert( 0 < bcnt_src && bcnt_src <= 64 );

		// Constexpr implementation, the VM does not like signed/overflowing shifts very much.
		//
		if ( std::is_constant_evaluated() )
		{
			return value & ( ~0ull >> ( 64 - bcnt_src ) );
		}
		else
		{
			// Shift left matching the MSB and shift right.
			//
			value <<= impl::rshiftcnt( bcnt_src );
			value >>= impl::rshiftcnt( bcnt_src );
			return value;
		}
	}

	// Sign extends the given integer.
	//
	__forceinline static constexpr int64_t sign_extend( uint64_t value, bitcnt_t bcnt_src )
	{
		dassert( 0 < bcnt_src && bcnt_src <= 64 );

		// Constexpr implementation, the VM does not like signed/overflowing shifts very much.
		//
		if ( std::is_constant_evaluated() )
		{
			value &= ( ~0ull >> ( 64 - bcnt_src ) );
			if ( bcnt_src != 1 && bcnt_src != 64 && ( value >> ( bcnt_src - 1 ) ) )
				value |= ( ~0ull << bcnt_src );
			return value;
		}
		else
		{
			// Interprete as signed, shift left matching the MSB and shift right.
			//
			int64_t signed_value = ( int64_t ) value;
			signed_value = signed_value << impl::rshiftcnt( bcnt_src );
			signed_value = signed_value >> impl::rshiftcnt( bcnt_src );

			// Check for the edge case at return to generate conditional move.
			//
			return bcnt_src == 1 ? signed_value & 1 : signed_value; // CMOV
		}
	}

	// Return value from bit-vector lookup where the result can be either unknown or constant 0/1.
	//
	enum class bit_state : int8_t
	{
		zero =    -1,
		unknown = 0,
		one =     +1,
	};

	// Bit-vector holding 0 to 64 bits of value with optional unknowns.
	//
	class bit_vector : public reducable<bit_vector>
	{
		// Number of bits this vector contains.
		//
		bitcnt_t bit_count = 0;

		// Mask for the bit that we do not know.
		// - Guaranteed to hold 0 for known bits and for all bits above bit_count.
		//
		uint64_t unknown_bits = 0;

		// Value of the known bits, mask of it can be found by [::known_mask()]
		// - Guaranteed to hold 0 for unknown bits.
		//
		uint64_t known_bits = 0;

	public:
		// Default constructor, will result in invalid bit-vector.
		//
		constexpr bit_vector() = default;

		// Constructs a bit-vector where all bits are set according to the state.
		// - Declared explicit to avoid construction from integers.
		//
		constexpr explicit bit_vector( bitcnt_t bit_count ) :                                      
			bit_count( bit_count ),     unknown_bits( fill( bit_count ) ),                  known_bits( 0 ) {}
																																		  
		// Constructs a bit-vector where all bits are known.												                              
		//																									                              
		constexpr bit_vector( uint64_t value, bitcnt_t bit_count ) :                               
			bit_count( bit_count ),     unknown_bits( 0 ),                                  known_bits( value & fill( bit_count ) ) {}
																																			
		// Constructs a bit-vector where bits are partially known.											                                
		//																									                                
		constexpr bit_vector( uint64_t known_bits, uint64_t unknown_bits, bitcnt_t bit_count ) :   
			bit_count( bit_count ),     unknown_bits( unknown_bits & fill( bit_count ) ),   known_bits( known_bits & ( ~unknown_bits ) & fill( bit_count ) ) {}

		// Some helpers to access the internal state.
		//
		constexpr uint64_t value_mask() const { return fill( bit_count ); }
		constexpr uint64_t unknown_mask() const { return unknown_bits; }
		constexpr uint64_t known_mask() const { return fill( bit_count ) & ~unknown_bits; }
		constexpr uint64_t known_one() const { return known_bits; }
		constexpr uint64_t known_zero() const { return ~( unknown_bits | known_bits ); }
		constexpr bool all_zero() const { return unknown_bits == 0 && !known_bits; }
		constexpr bool all_one() const { return unknown_bits == 0 && ( known_bits == fill( bit_count ) ); }
		constexpr bool is_valid() const { return bit_count != 0; }
		constexpr bool is_known() const { return bit_count && unknown_bits == 0; }
		constexpr bool is_unknown() const { return !bit_count || unknown_bits != 0; }
		constexpr bitcnt_t size() const { return bit_count; }

		// Gets the value represented, and nullopt if vector has unknown bits.
		//
		template<typename type>
		constexpr std::optional<type> get() const
		{
			if ( is_known() )
			{
				if constexpr ( std::is_signed_v<type> )
					return ( type ) sign_extend( known_bits, bit_count );
				else
					return ( type ) zero_extend( known_bits, bit_count );
			}
			return std::nullopt;
		}
		template<bool as_signed = false, typename type = std::conditional_t<as_signed, int64_t, uint64_t>>
		constexpr std::optional<type> get() const { return get<type>(); }

		// Extends or shrinks the the vector.
		//
		constexpr bit_vector& resize( bitcnt_t new_size, bool signed_cast = false )
		{
			fassert( 0 < new_size && new_size <= 64 );

			if( signed_cast && new_size > bit_count )
			{
				known_bits = sign_extend( known_bits, bit_count );
				unknown_bits = sign_extend( unknown_bits, bit_count );
			}
			known_bits &= fill( new_size );
			unknown_bits &= fill( new_size );
			bit_count = new_size;
			return *this;
		}

		// Gets the state of the bit at the index given.
		//
		constexpr bit_state at( bitcnt_t n ) const
		{
			return bit_state( ( 2 * bit_test( known_bits, n ) ) - !bit_test( unknown_bits, n ) );
		}
		constexpr bit_state operator[]( bitcnt_t n ) const { return at( n ); }

		// Conversion to human-readable format.
		//
		std::string to_string() const
		{
			std::string out;
			for ( int n = bit_count - 1; n >= 0; n-- )
			{
				uint64_t mask = 1ull << n;
				out += ( unknown_bits & mask ) ? '?' : ( known_bits & mask ) ? '1' : '0';
			}
			return out;
		}

		// Declare reduction.
		//
		REDUCE_TO( unknown_bits, known_bits, bit_count );
	};
};