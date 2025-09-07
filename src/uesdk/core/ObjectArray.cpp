#include <uesdk/ObjectArray.hpp>
#include <uesdk/UnrealObjects.hpp>
#include <cstdint>

namespace SDK
{
    UObject* Chunked_TUObjectArray::GetByIndex(const int32_t Index) const
    {
        if (Index < 0 || Index > NumElements)
            return nullptr;

        const int32_t ChunkIndex = Index / ElementsPerChunk;
        const int32_t InChunkIdx = Index % ElementsPerChunk;

        return GetDecrytedObjPtr()[ChunkIndex][InChunkIdx].Object;
    }
    UObject* Fixed_TUObjectArray::GetByIndex(const int32_t Index) const
    {
        if (Index < 0 || Index > NumElements)
            return nullptr;

        return GetDecrytedObjPtr()[Index].Object;
    }

    TUObjectArray::TUObjectArray(bool IsChunked, void* Objects)
        : m_IsChunked(IsChunked)
        , m_ChunkedObjects(nullptr)
        , m_FixedObjects(nullptr)
    {
        if (IsChunked)
            m_ChunkedObjects = reinterpret_cast<Chunked_TUObjectArray*>(Objects);
        else
            m_FixedObjects = reinterpret_cast<Fixed_TUObjectArray*>(Objects);
    }
    int32_t TUObjectArray::Num()
    {
        if (m_IsChunked && m_ChunkedObjects)
            return m_ChunkedObjects->Num();
        else if (!m_IsChunked && m_FixedObjects)
            return m_FixedObjects->Num();

        return 0;
    }
    UObject* TUObjectArray::GetByIndex(int32_t Index)
    {
        if (m_IsChunked && m_ChunkedObjects)
            return m_ChunkedObjects->GetByIndex(Index);
        else if (!m_IsChunked && m_FixedObjects)
            return m_FixedObjects->GetByIndex(Index);

        return nullptr;
    }
}
