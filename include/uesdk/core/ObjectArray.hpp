#pragma once
#include <uesdk/UnrealEnums.hpp>
#include <cstdint>
#include <memory>
#include <string>

namespace SDK
{
    struct FUObjectItem
    {
        class UObject* Object;
        int32_t Flags;
        int32_t ClusterRootIndex;
        int32_t SerialNumber;
    };

    class Chunked_TUObjectArray
    {
    private:
        enum
        {
            ElementsPerChunk = 0x10000,
        };

    private:
        static inline auto DecryptPtr = [](void* ObjPtr) -> uint8_t* {
            return reinterpret_cast<uint8_t*>(ObjPtr);
        };

    private:
        FUObjectItem** Objects;
        uint8_t Pad_0[0x08];
        int32_t MaxElements;
        int32_t NumElements;
        int32_t MaxChunks;
        int32_t NumChunks;

    public:
        inline int32_t Num() const { return NumElements; }
        inline FUObjectItem** GetDecrytedObjPtr() const { return reinterpret_cast<FUObjectItem**>(DecryptPtr(Objects)); }

        class UObject* GetByIndex(const int32_t Index) const;
    };
    class Fixed_TUObjectArray
    {
    private:
        static inline auto DecryptPtr = [](void* ObjPtr) -> uint8_t* {
            return reinterpret_cast<uint8_t*>(ObjPtr);
        };

    private:
        FUObjectItem* Objects;
        int32_t MaxElements;
        int32_t NumElements;

    public:
        inline int Num() const { return NumElements; }
        inline FUObjectItem* GetDecrytedObjPtr() const { return reinterpret_cast<FUObjectItem*>(DecryptPtr(Objects)); }

        class UObject* GetByIndex(const int32_t Index) const;
    };

    /** @brief Simple wrapper to support both chunked and fixed object arrays. */
    class TUObjectArray
    {
    public:
        TUObjectArray(bool IsChunked, void* Objects);

    private:
        bool m_IsChunked;
        Chunked_TUObjectArray* m_ChunkedObjects;
        Fixed_TUObjectArray* m_FixedObjects;

    public:
        int32_t Num();
        class UObject* GetByIndex(int32_t Index);

    public:
        /**
         * @brief Finds a UObject in GObjects based off of it's full name, in the Dumper-7 style path.
         *
         * @tparam UEType - The object type to be casted to.
         * @param[in] FullName - The full target Dumper-7 style object path.
         * @param[in] RequiredType - The required EClassCastFlags.
         *
         * @return A pointer to UObject if found, else a nullptr.
         */
        template <typename UEType = class UObject>
        static UEType* FindObject(const std::string& FullName, EClassCastFlags RequiredType = CASTCLASS_None);

        /**
         * @brief Finds a UObject in GObjects based off of it's object name.
         *
         * @tparam UEType - The object type to be casted to.
         * @param[in] Name - The target object's name.
         * @param[in] RequiredType - The required EClassCastFlags.
         *
         * @return A pointer to UObject if found, else a nullptr.
         */
        template <typename UEType = class UObject>
        static UEType* FindObjectFast(const std::string& Name, EClassCastFlags RequiredType = CASTCLASS_None);

        /**
         * @brief Finds a UObject in GObjects, then finds a UObject in its outer object list.
         *
         * @tparam UEType - The object type to be casted to.
         * @param[in] Name - The target object's name.
         * @param[in] Outer - The target outer object's name.
         *
         * @return A pointer to UObject if found, else a nullptr.
         */
        template <typename UEType = class UObject>
        static UEType* FindObjectFastInOuter(const std::string& Name, const std::string& Outer);

        /** @brief Wrapper for FindObject. */
        class UClass* FindClass(const std::string& ClassFullName)
        {
            return FindObject<class UClass>(ClassFullName, CASTCLASS_UClass);
        }

        /** @brief Wrapper for FindObjectFast. */
        class UClass* FindClassFast(const std::string& ClassName)
        {
            return FindObjectFast<class UClass>(ClassName, CASTCLASS_UClass);
        }
    };

    inline std::unique_ptr<TUObjectArray> GObjects = nullptr;
}

#include <uesdk/ObjectArray.inl>