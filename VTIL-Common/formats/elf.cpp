// Copyright (c) 2021 Peter Meerwald-Stadler and contributors of the VTIL Project   
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
#include "elf.hpp"
#include "../io/asserts.hpp"
#include <cstring>
#include "../math/bitwise.hpp"

namespace vtil
{
#pragma pack(push, 4)
	// Magic constants
	//
	static constexpr uint32_t ELF_MAGIC = 0x464c457f; // "\x7fELF"

	// Class / Format
	//
	enum class format_id : uint8_t
	{
		elf32 = 0x01,
		elf64 = 0x02,
	};

	// Endiness
	//
	enum class endiness_id : uint8_t
	{
		little = 0x01,
		big = 0x02,
	};

	// OS ABI
	//
	enum class osabi_id : uint8_t
	{
		systemv = 0x00,
		hpux = 0x01,
		netbsd = 0x02,
		linnux = 0x03,
	};

	// Object File Type
	//
	enum class type_id : uint16_t
	{
		none = 0x0000,
		rel = 0x0001,
		exec = 0x0002,
		dyn = 0x0003,
	};

	// Machine
	//
	enum class machine_id : uint16_t
	{
		none = 0x0000,
		amd64 = 0x003e,
	};

	// File header
	//
	struct file_header_t
	{
		uint32_t magic;
		format_id format;
		endiness_id endiness;
		uint8_t version;
		osabi_id os_abi;
		uint8_t abi_version;
		uint8_t _pad[7];
		type_id type;
		machine_id machine;
	};

	// Program header
	//
	struct program_header_t
	{
		//type;
		//flags;
	};

#pragma pack(pop)

	// Implement the interface requirements:
	//
	size_t elf_image::get_section_count() const
	{
		// Get the section count from file header.
		//
        return 0;
	}

	section_descriptor elf_image::get_section( size_t index ) const
	{
		return {
		};
	}

	void elf_image::modify_section( size_t index, const section_descriptor& desc )
	{
	}

	uint64_t elf_image::next_free_rva() const
	{
        return 0;
	}

	void elf_image::add_section( section_descriptor& in_out, const void* data, size_t size )
	{
    }

	void elf_image::enum_relocations( const function_view<bool( const relocation_descriptor& )>& fn ) const
	{
    }

	bool elf_image::is_elf64() const
	{
        if ( !is_valid() )
            return false;

		return true;
	}

	uint64_t elf_image::get_image_base() const
	{
		// Get the image base from optional header.
		//

		return 0;
	}

	size_t elf_image::get_image_size() const
	{
		// Get the image size from optional header.
		//

		return 0;
	}

	bool elf_image::has_relocations() const
	{
		// Relocs must not be stripped and basereloc should exist.
		//

		return false;
	}

	std::optional<uint64_t> elf_image::get_entry_point() const
	{
		// Get the entry point from optional header, return nullopt if zero.
		//

		return std::nullopt;
	}

	bool elf_image::is_valid() const
	{
		// Get image boundaries and the file header.
		//
		const void* data = cdata();
		const void* data_limit = ( char* ) cdata() + size();
		auto file_header = ( const file_header_t* ) cdata();
		
		// Validate DOS header.
		//
		if ( size() < sizeof(file_header->magic) || file_header->magic != ELF_MAGIC ) 
			return false;

		// TODO: Validate more data...
		//
		return true;
	}
	
};
