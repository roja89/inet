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

#ifndef __INET_SEQUENCECHUNK_H_
#define __INET_SEQUENCECHUNK_H_

#include "inet/common/packet/SliceChunk.h"

namespace inet {

class SequenceChunk : public Chunk
{
  public:
    class Iterator
    {
      protected:
        const std::shared_ptr<const SequenceChunk> chunk = nullptr;
        int64_t position = 0;
        int index = 0;

      public:
        Iterator(const std::shared_ptr<const SequenceChunk>& chunk) : chunk(chunk) { }
        Iterator(const Iterator& other);

        int64_t getPosition() const { return position; }
        int getIndex() const { return index; }

        void move(int64_t byteLength);
        void seek(int64_t byteOffset);

        virtual int getStartIndex() const = 0;
        virtual int getEndIndex() const = 0;
        virtual int getIndexIncrement() const = 0;
        virtual const std::shared_ptr<Chunk>& getChunk() const = 0;
    };

    class ForwardIterator : public Iterator
    {
      public:
        ForwardIterator(const std::shared_ptr<const SequenceChunk>& chunk) : Iterator(chunk) { }
        ForwardIterator(const ForwardIterator &other) : Iterator(other) { }

        virtual int getStartIndex() const override { return 0; }
        virtual int getEndIndex() const override { return chunk->chunks.size() - 1; }
        virtual int getIndexIncrement() const override { return 1; }
        virtual const std::shared_ptr<Chunk>& getChunk() const override { return chunk->chunks[index]; }
    };

    class BackwardIterator : public Iterator
    {
      public:
        BackwardIterator(const std::shared_ptr<const SequenceChunk>& chunk) : Iterator(chunk) { }
        BackwardIterator(const ForwardIterator &other) : Iterator(other) { }

        virtual int getStartIndex() const override { return chunk->chunks.size() - 1; }
        virtual int getEndIndex() const override { return 0; }
        virtual int getIndexIncrement() const override { return -1; }
        virtual const std::shared_ptr<Chunk>& getChunk() const override { return chunk->chunks[chunk->chunks.size() - index - 1]; }
    };

  protected:
    std::vector<std::shared_ptr<Chunk>> chunks;

    void prependChunk(const std::shared_ptr<Chunk>& chunk);
    void prependChunk(const std::shared_ptr<SequenceChunk>& chunk);

    void appendChunk(const std::shared_ptr<Chunk>& chunk);
    void appendChunk(const std::shared_ptr<SliceChunk>& chunk);
    void appendChunk(const std::shared_ptr<SequenceChunk>& chunk);

  public:
    SequenceChunk() { }
    SequenceChunk(const SequenceChunk& other);

    void makeImmutable();

    const std::vector<std::shared_ptr<Chunk>>& getChunks() const { return chunks; }
    ForwardIterator createForwardIterator() const { return ForwardIterator(std::static_pointer_cast<const SequenceChunk>(shared_from_this())); }
    BackwardIterator createBackwardIterator() const { return BackwardIterator(std::static_pointer_cast<const SequenceChunk>(shared_from_this())); }

    template <typename T>
    std::shared_ptr<T> peek(const Iterator& iterator, int64_t byteLength = -1) const {
        return peekAt<T>(iterator, iterator.getPosition(), byteLength);
    }
    std::shared_ptr<SliceChunk> peek(const Iterator& iterator, int64_t byteLength = -1) const {
        return peekAt<SliceChunk>(iterator, iterator.getPosition(), byteLength);
    }

    template <typename T>
    std::shared_ptr<T> peekAt(const Iterator& iterator, int64_t byteOffset = 0, int64_t byteLength = -1) const {
        // fast path
        if (iterator.getIndex() != -1 && iterator.getIndex() != chunks.size() && byteOffset == iterator.getPosition()) {
            const auto& chunk = iterator.getChunk();
            if (byteLength == -1 || chunk->getByteLength() == byteLength) {
                if (auto castChunk = std::dynamic_pointer_cast<T>(chunk))
                    return castChunk;
            }
        }
        // linear search path
        int position = 0;
        int startIndex = iterator.getStartIndex();
        int endIndex = iterator.getEndIndex();
        int increment = iterator.getIndexIncrement();
        for (int i = startIndex; i != endIndex + increment; i += increment) {
            auto& chunk = chunks[i];
            if (byteOffset == position && (byteLength == -1 || byteLength == chunk->getByteLength())) {
                if (auto castChunk = std::dynamic_pointer_cast<T>(chunk))
                    return castChunk;
            }
            position += chunk->getByteLength();
        }
        // slow path
        assert(increment == 1 || byteLength != -1);
        return Chunk::peekAt<T>(increment == 1 ? byteOffset : getByteLength() - byteOffset - byteLength, byteLength);
    }
    std::shared_ptr<SliceChunk> peekAt(const Iterator& iterator, int64_t byteOffset = 0, int64_t byteLength = -1) const {
        return peekAt<SliceChunk>(iterator, byteOffset, byteLength);
    }

    void prepend(const std::shared_ptr<Chunk>& chunk, bool flatten = true);
    void append(const std::shared_ptr<Chunk>& chunk, bool flatten = true);

    virtual int64_t getByteLength() const override;

    virtual const char *getSerializerClassName() const override { return "inet::SequenceChunkSerializer"; }
    virtual std::string str() const override;

    friend class Packet;
    friend class PacketDescriptor;
};

} // namespace

#endif // #ifndef __INET_SEQUENCECHUNK_H_
