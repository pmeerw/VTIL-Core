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
#include "intrinsics.hpp"
#define ____SW_SX2(x, y) x##y
#define ____SW_SX1(x, y) ____SW_SX2(x, y)

#if MS_COMPILER
	#define static_warning(condition, message)                          \
       constexpr auto ____SW_SX1(____sw_, __LINE__) = []{               \
       	__pragma( warning( push ) )                                     \
       	__pragma( warning( 1:4996 ) )                                   \
       	struct [[deprecated( message )]] _ {};                          \
       	if constexpr ( !( condition ) )                                 \
              _{};                                                      \
       	__pragma( warning( pop ) )                                      \
            return 0;                                                   \
       }
#else
	#define static_warning(condition, message)                          \
       constexpr auto ____SW_SX1(____sw_, __LINE__) = []{               \
       	_Pragma("GCC diagnostic push")                                  \
       	_Pragma("-Wdeprecated")                                         \
           struct {                                           	        \
               [[deprecated( message )]] void __() const {}             \
           } _;                                                         \
           if constexpr ( !( condition ) )                              \
              _.__();                                                   \
       	_Pragma("GCC diagnostic pop")                                   \
            return 0;                                                   \
       }
#endif