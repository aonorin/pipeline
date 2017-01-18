// Copyright (c) 2011-2016, NVIDIA CORPORATION. All rights reserved.
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


#include <dp/sg/renderer/rix/gl/inc/ResourceVertexAttributeSet.h>
#include <dp/sg/core/VertexAttributeSet.h>

namespace dp
{
  namespace sg
  {
    namespace renderer
    {
      namespace rix
      {
        namespace gl
        {

          ResourceVertexAttributeSetSharedPtr ResourceVertexAttributeSet::get( const dp::sg::core::VertexAttributeSetSharedPtr &vertexAttributeSet, const ResourceManagerSharedPtr& resourceManager )
          {
            assert( vertexAttributeSet );
            assert( resourceManager );

            ResourceVertexAttributeSetSharedPtr resourceVertexAttributeSet = resourceManager->getResource<ResourceVertexAttributeSet>( reinterpret_cast<size_t>(vertexAttributeSet.operator->()) );   // Big Hack !!
            if ( !resourceVertexAttributeSet )
            {
              resourceVertexAttributeSet = std::shared_ptr<ResourceVertexAttributeSet>( new ResourceVertexAttributeSet( vertexAttributeSet, resourceManager ) );
              resourceVertexAttributeSet->m_vertexAttributesHandle = resourceManager->getRenderer()->vertexAttributesCreate();
              resourceVertexAttributeSet->update();
            }
            return resourceVertexAttributeSet;
          }

          ResourceVertexAttributeSet::ResourceVertexAttributeSet( const dp::sg::core::VertexAttributeSetSharedPtr &vertexAttributeSet, const ResourceManagerSharedPtr& resourceManager )
            : ResourceManager::Resource( reinterpret_cast<size_t>( vertexAttributeSet.operator->() ), resourceManager )   // Big Hack !!
            , m_vertexAttributeSet( vertexAttributeSet )
          {
            resourceManager->subscribe( this );
          }

          ResourceVertexAttributeSet::~ResourceVertexAttributeSet()
          {
            m_resourceManager->unsubscribe( this );
          }

          dp::sg::core::HandledObjectSharedPtr ResourceVertexAttributeSet::getHandledObject() const
          {
            return std::static_pointer_cast<dp::sg::core::HandledObject>(m_vertexAttributeSet);
          }

          void ResourceVertexAttributeSet::update()
          {
            /** copy over vertex data **/
            dp::rix::core::Renderer *renderer = m_resourceManager->getRenderer();

            std::vector<dp::rix::core::VertexFormatInfo>  vertexInfos;
            dp::rix::core::VertexDataSharedHandle vertexData = renderer->vertexDataCreate();

            unsigned int numVertices = m_vertexAttributeSet->getVertexAttribute( dp::sg::core::VertexAttributeSet::AttributeID::POSITION ).getVertexDataCount();

            std::vector<ResourceBufferSharedPtr> resourceBuffers;

            unsigned int currentStream = 0;
            unsigned int nonStreamedAttributes = 0;

            std::vector<dp::sg::core::BufferSharedPtr> streams;

            for ( unsigned int i = 0; i < 16; ++i ) // FIXME Must match MAX_ATTRIBUTES, but that is only defined in inc\RendererAPI\RendererGL.h
            {
              dp::sg::core::VertexAttribute va = m_vertexAttributeSet->getVertexAttribute( static_cast<dp::sg::core::VertexAttributeSet::AttributeID>(i) );

              if ( va.getBuffer() && va.getVertexDataCount() == numVertices )
              {
                ResourceBufferSharedPtr resourceBuffer = ResourceBuffer::get(va.getBuffer(), m_resourceManager );
                resourceBuffers.push_back( resourceBuffer );

                // detect streams only with base offset == 0
                // a more sophisticated detection would try to find buffers with the same stride
                // whose offsets are all in the range [min(buffers.offset), min(buffers.offset)+stride-attributeSize]
                if (  va.getVertexDataOffsetInBytes() < va.getVertexDataStrideInBytes()
                   && getSizeOf(va.getVertexDataType()) != va.getVertexDataStrideInBytes())
                {
                  std::vector<dp::sg::core::BufferSharedPtr>::iterator it = std::find( streams.begin(), streams.end(), va.getBuffer() );
                  if ( it == streams.end() )
                  {
                    currentStream = static_cast<unsigned int>(streams.size());
                    streams.push_back( va.getBuffer() );
                  }
                  else
                  {
                    currentStream = static_cast<unsigned int>(std::distance( streams.begin(), it ));
                  }
                }
                else
                {
                  currentStream = streams.size() + nonStreamedAttributes;
                  ++nonStreamedAttributes;
                }

                size_t attributeOffset = va.getVertexDataOffsetInBytes() % va.getVertexDataStrideInBytes();
                vertexInfos.push_back( dp::rix::core::VertexFormatInfo( i, va.getVertexDataType(), va.getVertexDataSize(), false, currentStream, attributeOffset, va.getVertexDataStrideInBytes()));

                renderer->vertexDataSet( vertexData, currentStream, resourceBuffer->m_bufferHandle, va.getVertexDataOffsetInBytes() - attributeOffset, numVertices );
              }

            }
            dp::rix::core::VertexFormatDescription vertexFormatDescription(vertexInfos.empty() ? nullptr : &vertexInfos[0], vertexInfos.size());
            dp::rix::core::VertexFormatSharedHandle vertexFormat = renderer->vertexFormatCreate( vertexFormatDescription );

            renderer->vertexAttributesSet( m_vertexAttributesHandle, vertexData, vertexFormat );
            // keep a reference to the new resourceBuffers and remove reference to the old ones
            m_resourceBuffers.swap( resourceBuffers );
          }

        } // namespace gl
      } // namespace rix
    } // namespace renderer
  } // namespace sg
} // namespace dp

