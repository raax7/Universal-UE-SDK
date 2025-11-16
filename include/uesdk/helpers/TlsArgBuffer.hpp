#pragma once
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <malloc.h>

namespace SDK
{
    enum class TLSBufAllocType
    {
        TLS = 0,
        Stack,
        Heap
    };

    class TlsArgBuffer
    {
    public:
        TlsArgBuffer(size_t RequiredSize);

        ~TlsArgBuffer();

        uint8_t* GetData();

    private:
        void AllocAsTLS(size_t RequiredSize);

        void AllocAsStack(size_t RequiredSize);

        void AllocAsHeap(size_t RequiredSize);

    private:
        static constexpr size_t kAlignment = std::max<size_t>(16, sizeof(std::max_align_t)); // Required for SIMD instructions

        static constexpr size_t kBufferSize = 4096;
        static constexpr size_t kMaxDepth = 8;

        static constexpr size_t kMaxStackAllocSize = _ALLOCA_S_THRESHOLD - kAlignment;

        thread_local static inline uint8_t g_BufferStack[kMaxDepth][kBufferSize];
        thread_local static inline size_t g_BufferIndex = 0;

    private:
        TLSBufAllocType m_Type = TLSBufAllocType::TLS;
        uint8_t* m_Data = nullptr;
        uint8_t* m_Raw = nullptr;
        size_t m_Size = 0;
        bool m_Valid = false;
    };
}
