// Copyright NVIDIA Corporation 2013
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#pragma once

#include <dp/rix/core/RiX.h>
#include <dp/util/SmartPtr.h>
#include <dp/rix/gl/inc/ProgramGL.h>

namespace dp
{
  namespace rix
  {
    namespace gl
    {

      class ParameterCacheEntryStream : public dp::util::RCObject
      {
      public:
        ParameterCacheEntryStream( size_t cacheOffset, size_t containerOffset, size_t arraySize, size_t size );
        virtual void render( void const* cache ) const = 0;
        virtual void update( void * cache, void const * container ) const = 0;

        size_t getSize() const { return m_size; }

      protected:
        dp::util::Uint32 m_arraySize;
        size_t m_cacheOffset;
        size_t m_containerOffset;
        size_t m_size;
      };

      typedef dp::util::SmartPtr<ParameterCacheEntryStream> ParameterCacheEntryStreamSharedPtr;

      ParameterCacheEntryStreamSharedPtr createParameterCacheEntryStream( dp::rix::gl::ProgramGLHandle program
                                                                          , dp::rix::core::ContainerParameterType containerParameterType
                                                                          , dp::gl::Program::Uniform const& uniform, size_t cacheOffset
                                                                          , size_t containerOffset, size_t arraySize );

      typedef std::vector<ParameterCacheEntryStreamSharedPtr> ParameterCacheEntryStreams;

      ParameterCacheEntryStreams createParameterCacheEntriesStream( dp::rix::gl::ProgramGLHandle program, dp::rix::gl::ContainerDescriptorGLHandle descriptor );

    } // namespace gl
  } // namespace rix
} // namespace dp
