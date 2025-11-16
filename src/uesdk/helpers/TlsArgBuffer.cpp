#include <uesdk/helpers/TlsArgBuffer.hpp>

#include <cstdalign>
#include <stdexcept>

namespace SDK
{
    TlsArgBuffer::TlsArgBuffer(size_t RequiredSize)
    {
        if (RequiredSize <= kBufferSize && g_BufferIndex < kMaxDepth) {
            AllocAsTLS(RequiredSize);
            return;
        }

        if (RequiredSize <= kMaxStackAllocSize) {
            AllocAsStack(RequiredSize);
            return;
        }

        AllocAsHeap(RequiredSize);
    }

    TlsArgBuffer::~TlsArgBuffer()
    {
        if (!m_Valid)
            return;

        switch (m_Type) {
        case TLSBufAllocType::TLS:
            g_BufferIndex--;
            break;

        case TLSBufAllocType::Stack:
            _freea(m_Raw);
            break;

        case TLSBufAllocType::Heap:
            delete[] m_Raw;
            break;
        }
    }

    uint8_t* TlsArgBuffer::GetData()
    {
        return m_Data;
    }

    void TlsArgBuffer::AllocAsTLS(size_t RequiredSize)
    {
        m_Type = TLSBufAllocType::TLS;
        m_Raw = g_BufferStack[g_BufferIndex];

        size_t TotalSize = RequiredSize + kAlignment;

        void* Aligned = m_Raw;
        if (!std::align(kAlignment, RequiredSize, Aligned, TotalSize))
            throw std::bad_alloc();

        m_Data = static_cast<uint8_t*>(Aligned);
        m_Size = RequiredSize;

        std::memset(m_Data, 0, m_Size);
        m_Valid = true;

        // Delay incrementing to avoid incrementing on failure
        g_BufferIndex++;
    }

    void TlsArgBuffer::AllocAsStack(size_t RequiredSize)
    {
        m_Type = TLSBufAllocType::Stack;

        size_t TotalSize = RequiredSize + kAlignment;
        m_Raw = static_cast<uint8_t*>(_malloca(TotalSize));
        if (!m_Raw)
            throw std::bad_alloc();

        void* Aligned = m_Raw;
        if (!std::align(kAlignment, RequiredSize, Aligned, TotalSize))
            throw std::bad_alloc();

        m_Data = static_cast<uint8_t*>(Aligned);
        m_Size = RequiredSize;

        std::memset(m_Data, 0, m_Size);
        m_Valid = true;
    }

    void TlsArgBuffer::AllocAsHeap(size_t RequiredSize)
    {
        m_Type = TLSBufAllocType::Heap;
        m_Data = new uint8_t[RequiredSize];
        m_Raw = m_Data;
        m_Size = RequiredSize;

        std::memset(m_Data, 0, m_Size);
        m_Valid = true;
    }
}
