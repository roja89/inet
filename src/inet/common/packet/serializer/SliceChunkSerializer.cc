//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/common/packet/chunk/SliceChunk.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"
#include "inet/common/packet/serializer/SliceChunkSerializer.h"

namespace inet {

Register_Serializer(SliceChunk, SliceChunkSerializer);

void SliceChunkSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk, bit offset, bit length) const
{
    const auto& sliceChunk = std::static_pointer_cast<const SliceChunk>(chunk);
    Chunk::serialize(stream, sliceChunk->getChunk(), sliceChunk->getOffset() + offset, length == bit(-1) ? sliceChunk->getLength() - offset : length);
}

Ptr<Chunk> SliceChunkSerializer::deserialize(MemoryInputStream& stream, const std::type_info& typeInfo) const
{
    throw cRuntimeError("Invalid operation");
}

} // namespace