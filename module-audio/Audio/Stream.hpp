#pragma once

#include <memory/NonCachedMemAllocator.hpp>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <utility>

namespace audio
{
    class Stream
    {
      public:
        using UniqueStreamBuffer = std::unique_ptr<std::uint8_t[], std::function<void(uint8_t[])>>;

        struct Span
        {
            std::uint8_t *data   = nullptr;
            std::size_t dataSize = 0;
        };

        class Allocator
        {
          public:
            virtual UniqueStreamBuffer allocate(std::size_t size) = 0;
        };

        enum class Event
        {
            NoEvent,
            StreamFull,
            StreamEmpty
        };

        enum class EventSourceMode
        {
            ISR,
            Thread
        };

        class EventListener
        {
          public:
            virtual void onEvent(Stream *stream, Event event, EventSourceMode source) = 0;
        };

        static constexpr auto defaultBufferingSize = 4U;

        Stream(Allocator &allocator, std::size_t blockSize, unsigned int bufferingSize = defaultBufferingSize);

        bool push(void *data, std::size_t dataSize);
        bool push(const Span &span);

        bool reserve(Span &span);

        bool peek(Span &span);

        bool pop();
        bool pop(Span &span);

        std::size_t getBlockSize() const noexcept;
        std::size_t getBlockCount() const noexcept;
        std::size_t getUsedBlockCount() const noexcept;
        bool isEmpty() const noexcept;
        bool isFull() const noexcept;

        void registerListener(EventListener &listener);

      private:
        void broadcastEvent(Event event);

        Allocator &_allocator;
        std::size_t _blockSize  = 0;
        std::size_t _blockCount = 0;
        UniqueStreamBuffer _buffer;
        UniqueStreamBuffer _emptyBuffer;
        std::list<std::reference_wrapper<EventListener>> listeners;
    };

    class StandardStreamAllocator : public Stream::Allocator
    {
      public:
        Stream::UniqueStreamBuffer allocate(std::size_t size);
    };

    class NonCacheableStreamAllocator : public Stream::Allocator
    {
      public:
        NonCacheableStreamAllocator() = default;
        Stream::UniqueStreamBuffer allocate(std::size_t size);

      private:
        NonCachedMemAllocator<uint8_t> allocator;
    };

} // namespace audio
