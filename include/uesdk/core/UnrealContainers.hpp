#pragma once
#include <uesdk/core/FMemory.hpp>

#include <ostream>

// Thanks to https://github.com/Fischsalat/UnrealContainers for proper TArray, FString and TArrayIterator support.

namespace SDK
{
    template <typename ArrayElementType>
    class TArray;

    template <typename SparseArrayElementType>
    class TSparseArray;

    template <typename SetElementType>
    class TSet;

    template <typename KeyElementType, typename ValueElementType>
    class TMap;

    template <typename KeyElementType, typename ValueElementType>
    class TPair;

    namespace Iterators
    {
        class FSetBitIterator;

        template <typename ArrayType>
        class TArrayIterator;

        template <class ContainerType>
        class TContainerIterator;

        template <typename SparseArrayElementType>
        using TSparseArrayIterator = TContainerIterator<TSparseArray<SparseArrayElementType>>;

        template <typename SetElementType>
        using TSetIterator = TContainerIterator<TSet<SetElementType>>;

        template <typename KeyElementType, typename ValueElementType>
        using TMapIterator = TContainerIterator<TMap<KeyElementType, ValueElementType>>;
    }

    namespace ContainerImpl
    {
        namespace HelperFunctions
        {
            inline uint32_t FloorLog2(uint32_t Value)
            {
                uint32_t pos = 0;
                if (Value >= 1 << 16) {
                    Value >>= 16;
                    pos += 16;
                }
                if (Value >= 1 << 8) {
                    Value >>= 8;
                    pos += 8;
                }
                if (Value >= 1 << 4) {
                    Value >>= 4;
                    pos += 4;
                }
                if (Value >= 1 << 2) {
                    Value >>= 2;
                    pos += 2;
                }
                if (Value >= 1 << 1) {
                    pos += 1;
                }
                return pos;
            }

            inline uint32_t CountLeadingZeros(uint32_t Value)
            {
                if (Value == 0)
                    return 32;

                return 31 - FloorLog2(Value);
            }
        }

        template <int32_t Size, uint32_t Alignment>
        struct TAlignedBytes
        {
            alignas(Alignment) uint8_t Pad[Size];
        };

        template <uint32_t NumInlineElements>
        class TInlineAllocator
        {
        public:
            template <typename ElementType>
            class ForElementType
            {
            private:
                static constexpr int32_t ElementSize = sizeof(ElementType);
                static constexpr int32_t ElementAlign = alignof(ElementType);

                static constexpr int32_t InlineDataSizeBytes = NumInlineElements * ElementSize;

            private:
                TAlignedBytes<ElementSize, ElementAlign> InlineData[NumInlineElements];
                ElementType* SecondaryData;

            public:
                ForElementType()
                    : InlineData { 0x0 }
                    , SecondaryData(nullptr)
                {
                }

                ForElementType(ForElementType&& Other)
                    : InlineData { 0x0 }
                    , SecondaryData(nullptr)
                {
                    MoveFrom(std::move(Other));
                }

                ~ForElementType()
                {
                    Free();
                }

            public:
                ForElementType& operator=(ForElementType&& Other) noexcept
                {
                    MoveFrom(std::move(Other));

                    return *this;
                }

            private:
                inline void MoveFrom(ForElementType&& Other)
                {
                    if (this == &Other)
                        return;

                    Free();

                    if (Other.SecondaryData)
                        SecondaryData = Other.SecondaryData;

                    memcpy(InlineData, Other.InlineData, InlineDataSizeBytes);

                    memset(Other.InlineData, 0x0, InlineDataSizeBytes);
                    Other.SecondaryData = nullptr;
                }

                inline void FitAllocation(const int32_t OldNumElements, const int32_t NewNumElements)
                {
                    /* No need to do anything if NewSize still fits into InlineData */
                    if (NewNumElements <= NumInlineElements) {
                        if (OldNumElements > NumInlineElements && SecondaryData) {
                            memcpy(InlineData, SecondaryData, InlineDataSizeBytes);
                            FMemory::Free(SecondaryData);
                        }

                        return;
                    }

                    /* Allocates if SecondaryData is nullptr */
                    SecondaryData = reinterpret_cast<ElementType*>(FMemory::Realloc(SecondaryData, NewNumElements * ElementSize, ElementAlign));

                    if (OldNumElements < NumInlineElements)
                        memcpy(SecondaryData, InlineData, InlineDataSizeBytes);
                }

            public:
                inline void CopyFrom(const ForElementType& Other, const int32_t OldNumElements, const int32_t NewNumElements)
                {
                    FitAllocation(OldNumElements, NewNumElements);

                    if (Other.SecondaryData) {
                        memcpy(SecondaryData, Other.SecondaryData, NewNumElements * ElementSize);
                    }
                    else {
                        memcpy(InlineData, Other.InlineData, InlineDataSizeBytes);
                    }
                }

                inline void Free()
                {
                    if (SecondaryData)
                        FMemory::Free(SecondaryData);

                    memset(InlineData, 0x0, InlineDataSizeBytes);
                }

            public:
                inline const ElementType* GetAllocation() const { return SecondaryData ? SecondaryData : reinterpret_cast<const ElementType*>(&InlineData); }

                inline uint32_t GetNumInlineBytes() const { return NumInlineElements; }
            };
        };

        class FBitArray
        {
        protected:
            static constexpr int32_t NumBitsPerDWORD = 32;
            static constexpr int32_t NumBitsPerDWORDLogTwo = 5;

        private:
            TInlineAllocator<4>::ForElementType<int32_t> Data;
            int32_t NumBits;
            int32_t MaxBits;

        public:
            FBitArray()
                : NumBits(0)
                , MaxBits(Data.GetNumInlineBytes() * NumBitsPerDWORD)
            {
            }

            FBitArray(const FBitArray& Other)
            {
                InitializeFrom(Other);
            }

            FBitArray(FBitArray&&) = default;

        public:
            FBitArray& operator=(FBitArray&&) = default;

            FBitArray& operator=(const FBitArray& Other)
            {
                InitializeFrom(Other);

                return *this;
            }

        private:
            inline void InitializeFrom(const FBitArray& Other)
            {
                if (this == &Other)
                    return;

                NumBits = Other.NumBits;
                MaxBits = Other.MaxBits;

                const int32_t OldNumElements = MaxBits / NumBitsPerDWORD;
                const int32_t NewNumElements = Other.NumBits / NumBitsPerDWORD;

                Data.CopyFrom(Other.Data, OldNumElements, NewNumElements);
            }

        private:
            inline void VerifyIndex(int32_t Index) const
            {
                if (!IsValidIndex(Index))
                    throw std::out_of_range("Index was out of range!");
            }

        public:
            inline int32_t Num() const { return NumBits; }
            inline int32_t Max() const { return MaxBits; }

            inline const uint32_t* GetData() const { return reinterpret_cast<const uint32_t*>(Data.GetAllocation()); }

            inline bool IsValidIndex(int32_t Index) const { return Index >= 0 && Index < NumBits; }

            inline bool IsValid() const { return GetData() && NumBits > 0; }

        public:
            inline bool operator[](int32_t Index) const
            {
                VerifyIndex(Index);
                return GetData()[Index / NumBitsPerDWORD] & (1 << (Index & (NumBitsPerDWORD - 1)));
            }

            inline bool operator==(const FBitArray& Other) const { return NumBits == Other.NumBits && GetData() == Other.GetData(); }
            inline bool operator!=(const FBitArray& Other) const { return NumBits != Other.NumBits || GetData() != Other.GetData(); }

        public:
            friend Iterators::FSetBitIterator begin(const FBitArray& Array);
            friend Iterators::FSetBitIterator end(const FBitArray& Array);
        };

        template <typename SparseArrayType>
        union TSparseArrayElementOrFreeListLink
        {
            SparseArrayType ElementData;

            struct
            {
                int32_t PrevFreeIndex;
                int32_t NextFreeIndex;
            };
        };

        template <typename SetType>
        class SetElement
        {
        private:
            template <typename SetDataType>
            friend class TSet;

        private:
            SetType Value;
            int32_t HashNextId;
            int32_t HashIndex;
        };
    }

    template <typename KeyType, typename ValueType>
    class TPair
    {
    private:
        KeyType First;
        ValueType Second;

    public:
        TPair(KeyType Key, ValueType Value)
            : First(Key)
            , Second(Value)
        {
        }

    public:
        inline KeyType& Key() { return First; }
        inline const KeyType& Key() const { return First; }

        inline ValueType& Value() { return Second; }
        inline const ValueType& Value() const { return Second; }
    };

    template <typename ArrayElementType>
    class TArray
    {
    private:
        template <typename SparseArrayElementType>
        friend class TSparseArray;

    protected:
        static constexpr uint64_t ElementAlign = alignof(ArrayElementType);
        static constexpr uint64_t ElementSize = sizeof(ArrayElementType);

    protected:
        ArrayElementType* Data;
        int32_t NumElements;
        int32_t MaxElements;

    public:
        TArray()
            : Data(nullptr)
            , NumElements(0)
            , MaxElements(0)
        {
        }

        TArray(int32_t Size)
            : Data(static_cast<ArrayElementType*>(FMemory::Malloc(Size * ElementSize, ElementAlign)))
            , NumElements(0)
            , MaxElements(Size)
        {
        }

        TArray(const TArray& Other)
            : Data(nullptr)
            , NumElements(0)
            , MaxElements(0)
        {
            this->CopyFrom(Other);
        }

        TArray(TArray&& Other) noexcept
            : Data(Other.Data)
            , NumElements(Other.NumElements)
            , MaxElements(Other.MaxElements)
        {
            Other.Data = nullptr;
            Other.NumElements = 0x0;
            Other.MaxElements = 0x0;
        }

        ~TArray()
        {
            Free();
        }

    public:
        TArray& operator=(TArray&& Other) noexcept
        {
            if (this == &Other)
                return *this;

            Free();

            Data = Other.Data;
            NumElements = Other.NumElements;
            MaxElements = Other.MaxElements;

            Other.Data = nullptr;
            Other.NumElements = 0x0;
            Other.MaxElements = 0x0;

            return *this;
        }

        TArray& operator=(const TArray& Other)
        {
            this->CopyFrom(Other);

            return *this;
        }

    private:
        inline int32_t GetSlack() const { return MaxElements - NumElements; }

        inline void VerifyIndex(int32_t Index) const
        {
            if (!IsValidIndex(Index))
                throw std::out_of_range("Index was out of range!");
        }

        inline ArrayElementType& GetUnsafe(int32_t Index) { return Data[Index]; }
        inline const ArrayElementType& GetUnsafe(int32_t Index) const { return Data[Index]; }

    public:
        inline void Reserve(int32_t Count)
        {
            if (GetSlack() < Count)
                MaxElements += Count;

            Data = static_cast<ArrayElementType*>(FMemory::Realloc(Data, MaxElements * ElementSize, ElementAlign));
        }

        inline void Add(const ArrayElementType& Element)
        {
            if (GetSlack() <= 0)
                Reserve(3);

            Data[NumElements] = Element;
            NumElements++;
        }

        inline void CopyFrom(const TArray& Other)
        {
            if (this == &Other || Other.NumElements == 0)
                return;

            NumElements = Other.NumElements;

            if (MaxElements >= Other.NumElements) {
                memcpy(Data, Other.Data, Other.NumElements);
                return;
            }

            Data = static_cast<ArrayElementType*>(FMemory::Realloc(Data, Other.NumElements * ElementSize, ElementAlign));
            MaxElements = Other.NumElements;
            memcpy(Data, Other.Data, Other.NumElements * ElementSize);
        }

        inline void Remove(int32_t Index)
        {
            if (!IsValidIndex(Index))
                return;

            NumElements--;

            for (int i = Index; i < NumElements; i++) {
                /* NumElements was decremented, acessing i + 1 is safe */
                Data[i] = Data[i + 1];
            }
        }

        inline void Clear()
        {
            NumElements = 0;

            if (Data)
                memset(Data, 0, NumElements * ElementSize);
        }

        inline void Free() noexcept
        {
            if (Data)
                FMemory::Free(Data);

            NumElements = 0x0;
            MaxElements = 0x0;
        }

    public:
        inline int32_t Num() const { return NumElements; }
        inline int32_t Max() const { return MaxElements; }

        inline bool IsValidIndex(int32_t Index) const { return Data && Index >= 0 && Index < NumElements; }

        inline bool IsValid() const { return Data && NumElements > 0 && MaxElements >= NumElements; }

    public:
        inline ArrayElementType& operator[](int32_t Index)
        {
            VerifyIndex(Index);
            return Data[Index];
        }
        inline const ArrayElementType& operator[](int32_t Index) const
        {
            VerifyIndex(Index);
            return Data[Index];
        }

        inline bool operator==(const TArray<ArrayElementType>& Other) const { return Data == Other.Data; }
        inline bool operator!=(const TArray<ArrayElementType>& Other) const { return Data != Other.Data; }

        inline explicit operator bool() const { return IsValid(); };

    public:
        template <typename T>
        friend Iterators::TArrayIterator<T> begin(const TArray& Array);
        template <typename T>
        friend Iterators::TArrayIterator<T> end(const TArray& Array);
    };

    class FString final : public TArray<wchar_t>
    {
    public:
        friend std::ostream& operator<<(std::ostream& Stream, const FString& Str) { return Stream << Str.ToString(); }

    public:
        using TArray::TArray;

        FString(const wchar_t* Str)
        {
            const uint32_t NullTerminatedLength = static_cast<uint32_t>(wcslen(Str) + 0x1);

            *this = FString(NullTerminatedLength);

            NumElements = NullTerminatedLength;
            memcpy(Data, Str, NullTerminatedLength * sizeof(wchar_t));
        }

    public:
        inline std::string ToString() const
        {
            if (*this) {
                std::wstring WData(Data);
                return std::string(WData.begin(), WData.end());
            }

            return "";
        }

        inline std::wstring ToWString() const
        {
            if (*this)
                return std::wstring(Data);

            return L"";
        }

    public:
        inline wchar_t* CStr() { return Data; }
        inline const wchar_t* CStr() const { return Data; }

    public:
        inline bool operator==(const FString& Other) const { return Other ? NumElements == Other.NumElements && wcscmp(Data, Other.Data) == 0 : false; }
        inline bool operator!=(const FString& Other) const { return Other ? NumElements != Other.NumElements || wcscmp(Data, Other.Data) != 0 : true; }
    };

    template <typename SparseArrayElementType>
    class TSparseArray
    {
    private:
        static constexpr uint32_t ElementAlign = alignof(SparseArrayElementType);
        static constexpr uint32_t ElementSize = sizeof(SparseArrayElementType);

    private:
        using FElementOrFreeListLink = ContainerImpl::TSparseArrayElementOrFreeListLink<ContainerImpl::TAlignedBytes<ElementSize, ElementAlign>>;

    private:
        TArray<FElementOrFreeListLink> Data;
        ContainerImpl::FBitArray AllocationFlags;
        int32_t FirstFreeIndex;
        int32_t NumFreeIndices;

    public:
        TSparseArray()
            : FirstFreeIndex(-1)
            , NumFreeIndices(0)
        {
        }

        TSparseArray(const TSparseArray&) = default;
        TSparseArray(TSparseArray&&) = default;

    public:
        TSparseArray& operator=(TSparseArray&&) = default;
        TSparseArray& operator=(const TSparseArray&) = default;

    private:
        inline void VerifyIndex(int32_t Index) const
        {
            if (!IsValidIndex(Index))
                throw std::out_of_range("Index was out of range!");
        }

    public:
        inline int32_t NumAllocated() const { return Data.Num(); }

        inline int32_t Num() const { return NumAllocated() - NumFreeIndices; }
        inline int32_t Max() const { return Data.Max(); }

        inline bool IsValidIndex(int32_t Index) const { return Data.IsValidIndex(Index) && AllocationFlags[Index]; }

        inline bool IsValid() const { return Data.IsValid() && AllocationFlags.IsValid(); }

    public:
        const ContainerImpl::FBitArray& GetAllocationFlags() const { return AllocationFlags; }

    public:
        inline SparseArrayElementType& operator[](int32_t Index)
        {
            VerifyIndex(Index);
            return *reinterpret_cast<SparseArrayElementType*>(&Data.GetUnsafe(Index).ElementData);
        }
        inline const SparseArrayElementType& operator[](int32_t Index) const
        {
            VerifyIndex(Index);
            return *reinterpret_cast<SparseArrayElementType*>(&Data.GetUnsafe(Index).ElementData);
        }

        inline bool operator==(const TSparseArray<SparseArrayElementType>& Other) const { return Data == Other.Data; }
        inline bool operator!=(const TSparseArray<SparseArrayElementType>& Other) const { return Data != Other.Data; }

    public:
        template <typename T>
        friend Iterators::TSparseArrayIterator<T> begin(const TSparseArray& Array);
        template <typename T>
        friend Iterators::TSparseArrayIterator<T> end(const TSparseArray& Array);
    };

    template <typename SetElementType>
    class TSet
    {
    private:
        static constexpr uint32_t ElementAlign = alignof(SetElementType);
        static constexpr uint32_t ElementSize = sizeof(SetElementType);

    private:
        using SetDataType = ContainerImpl::SetElement<SetElementType>;
        using HashType = ContainerImpl::TInlineAllocator<1>::ForElementType<int32_t>;

    private:
        TSparseArray<SetDataType> Elements;
        HashType Hash;
        int32_t HashSize;

    public:
        TSet()
            : HashSize(0)
        {
        }

        TSet(TSet&& Other)
        {
            MoveFrom(std::move(Other));
        }

        /* Todo */
        TSet(const TSet& Other)
            : HashSize(0)
        {
            CopyFrom(Other);
        }

    public:
        TSet& operator=(TSet&& Other) noexcept
        {
            MoveFrom(std::move(Other));

            return *this;
        }

        TSet& operator=(const TSet& Other)
        {
            CopyFrom(Other);

            return *this;
        }

    private:
        inline void MoveFrom(TSet&& Other)
        {
            if (this == &Other)
                return;

            Elements = std::move(Other.Elements);
            Hash = std::move(Other.Hash);
            HashSize = Other.HashSize;

            Other.HashSize = 0x0;
        }

        inline void CopyFrom(const TSet& Other)
        {
            if (this == &Other)
                return;

            Elements = Other.Elements;
            Hash.CopyFrom(Other.Hash, HashSize, Other.HashSize);
            HashSize = Other.HashSize;
        }

    private:
        inline void VerifyIndex(int32_t Index) const
        {
            if (!IsValidIndex(Index))
                throw std::out_of_range("Index was out of range!");
        }

    public:
        inline int32_t NumAllocated() const { return Elements.NumAllocated(); }

        inline int32_t Num() const { return Elements.Num(); }
        inline int32_t Max() const { return Elements.Max(); }

        inline bool IsValidIndex(int32_t Index) const { return Elements.IsValidIndex(Index); }

        inline bool IsValid() const { return Elements.IsValid(); }

    public:
        const ContainerImpl::FBitArray& GetAllocationFlags() const { return Elements.GetAllocationFlags(); }

    public:
        inline SetElementType& operator[](int32_t Index) { return Elements[Index].Value; }
        inline const SetElementType& operator[](int32_t Index) const { return Elements[Index].Value; }

        inline bool operator==(const TSet<SetElementType>& Other) const { return Elements == Other.Elements; }
        inline bool operator!=(const TSet<SetElementType>& Other) const { return Elements != Other.Elements; }

    public:
        template <typename T>
        friend Iterators::TSetIterator<T> begin(const TSet& Set);
        template <typename T>
        friend Iterators::TSetIterator<T> end(const TSet& Set);
    };

    template <typename KeyElementType, typename ValueElementType>
    class TMap
    {
    public:
        using ElementType = TPair<KeyElementType, ValueElementType>;

    private:
        TSet<ElementType> Elements;

    private:
        inline void VerifyIndex(int32_t Index) const
        {
            if (!IsValidIndex(Index))
                throw std::out_of_range("Index was out of range!");
        }

    public:
        inline int32_t NumAllocated() const { return Elements.NumAllocated(); }

        inline int32_t Num() const { return Elements.Num(); }
        inline int32_t Max() const { return Elements.Max(); }

        inline bool IsValidIndex(int32_t Index) const { return Elements.IsValidIndex(Index); }

        inline bool IsValid() const { return Elements.IsValid(); }

    public:
        const ContainerImpl::FBitArray& GetAllocationFlags() const { return Elements.GetAllocationFlags(); }

    public:
        inline decltype(auto) Find(const KeyElementType& Key, bool (*Equals)(const KeyElementType& LeftKey, const KeyElementType& RightKey))
        {
            for (auto It = begin(*this); It != end(*this); ++It) {
                if (Equals(It->Key(), Key))
                    return It;
            }

            return end(*this);
        }

    public:
        inline ElementType& operator[](int32_t Index) { return Elements[Index]; }
        inline const ElementType& operator[](int32_t Index) const { return Elements[Index]; }

        inline bool operator==(const TMap<KeyElementType, ValueElementType>& Other) const { return Elements == Other.Elements; }
        inline bool operator!=(const TMap<KeyElementType, ValueElementType>& Other) const { return Elements != Other.Elements; }

    public:
        template <typename KeyType, typename ValueType>
        friend Iterators::TMapIterator<KeyType, ValueType> begin(const TMap& Map);
        template <typename KeyType, typename ValueType>
        friend Iterators::TMapIterator<KeyType, ValueType> end(const TMap& Map);
    };

    namespace Iterators
    {
        class FRelativeBitReference
        {
        protected:
            static constexpr int32_t NumBitsPerDWORD = 32;
            static constexpr int32_t NumBitsPerDWORDLogTwo = 5;

        public:
            inline explicit FRelativeBitReference(int32_t BitIndex)
                : WordIndex(BitIndex >> NumBitsPerDWORDLogTwo)
                , Mask(1 << (BitIndex & (NumBitsPerDWORD - 1)))
            {
            }

            int32_t WordIndex;
            uint32_t Mask;
        };

        class FSetBitIterator : public FRelativeBitReference
        {
        private:
            const ContainerImpl::FBitArray& Array;

            uint32_t UnvisitedBitMask;
            int32_t CurrentBitIndex;
            int32_t BaseBitIndex;

        public:
            explicit FSetBitIterator(const ContainerImpl::FBitArray& InArray, int32_t StartIndex = 0)
                : FRelativeBitReference(StartIndex)
                , Array(InArray)
                , UnvisitedBitMask((~0U) << (StartIndex & (NumBitsPerDWORD - 1)))
                , CurrentBitIndex(StartIndex)
                , BaseBitIndex(StartIndex & ~(NumBitsPerDWORD - 1))
            {
                if (StartIndex != Array.Num())
                    FindFirstSetBit();
            }

        public:
            inline FSetBitIterator& operator++()
            {
                UnvisitedBitMask &= ~this->Mask;

                FindFirstSetBit();

                return *this;
            }

            inline explicit operator bool() const { return CurrentBitIndex < Array.Num(); }

            inline bool operator==(const FSetBitIterator& Rhs) const { return CurrentBitIndex == Rhs.CurrentBitIndex && &Array == &Rhs.Array; }
            inline bool operator!=(const FSetBitIterator& Rhs) const { return CurrentBitIndex != Rhs.CurrentBitIndex || &Array != &Rhs.Array; }

        public:
            inline int32_t GetIndex() { return CurrentBitIndex; }

            void FindFirstSetBit()
            {
                const uint32_t* ArrayData = Array.GetData();
                const int32_t ArrayNum = Array.Num();
                const int32_t LastWordIndex = (ArrayNum - 1) / NumBitsPerDWORD;

                uint32_t RemainingBitMask = ArrayData[this->WordIndex] & UnvisitedBitMask;
                while (!RemainingBitMask) {
                    ++this->WordIndex;
                    BaseBitIndex += NumBitsPerDWORD;
                    if (this->WordIndex > LastWordIndex) {
                        CurrentBitIndex = ArrayNum;
                        return;
                    }

                    RemainingBitMask = ArrayData[this->WordIndex];
                    UnvisitedBitMask = ~0;
                }

                const uint32_t NewRemainingBitMask = RemainingBitMask & (RemainingBitMask - 1);

                this->Mask = NewRemainingBitMask ^ RemainingBitMask;

                CurrentBitIndex = BaseBitIndex + NumBitsPerDWORD - 1 - ContainerImpl::HelperFunctions::CountLeadingZeros(this->Mask);

                if (CurrentBitIndex > ArrayNum)
                    CurrentBitIndex = ArrayNum;
            }
        };

        template <typename ArrayType>
        class TArrayIterator
        {
        private:
            TArray<ArrayType>& IteratedArray;
            int32_t Index;

        public:
            TArrayIterator(const TArray<ArrayType>& Array, int32_t StartIndex = 0x0)
                : IteratedArray(const_cast<TArray<ArrayType>&>(Array))
                , Index(StartIndex)
            {
            }

        public:
            inline int32_t GetIndex() { return Index; }

            inline int32_t IsValid() { return IteratedArray.IsValidIndex(GetIndex()); }

        public:
            inline TArrayIterator& operator++()
            {
                ++Index;
                return *this;
            }
            inline TArrayIterator& operator--()
            {
                --Index;
                return *this;
            }

            inline ArrayType& operator*() { return IteratedArray[GetIndex()]; }
            inline const ArrayType& operator*() const { return IteratedArray[GetIndex()]; }

            inline ArrayType* operator->() { return &IteratedArray[GetIndex()]; }
            inline const ArrayType* operator->() const { return &IteratedArray[GetIndex()]; }

            inline bool operator==(const TArrayIterator& Other) const { return &IteratedArray == &Other.IteratedArray && Index == Other.Index; }
            inline bool operator!=(const TArrayIterator& Other) const { return &IteratedArray != &Other.IteratedArray || Index != Other.Index; }
        };

        template <class ContainerType>
        class TContainerIterator
        {
        private:
            ContainerType& IteratedContainer;
            FSetBitIterator BitIterator;

        public:
            TContainerIterator(const ContainerType& Container, const ContainerImpl::FBitArray& BitArray, int32_t StartIndex = 0x0)
                : IteratedContainer(const_cast<ContainerType&>(Container))
                , BitIterator(BitArray, StartIndex)
            {
            }

        public:
            inline int32_t GetIndex() { return BitIterator.GetIndex(); }

            inline int32_t IsValid() { return IteratedContainer.IsValidIndex(GetIndex()); }

        public:
            inline TContainerIterator& operator++()
            {
                ++BitIterator;
                return *this;
            }
            inline TContainerIterator& operator--()
            {
                --BitIterator;
                return *this;
            }

            inline auto& operator*() { return IteratedContainer[GetIndex()]; }
            inline const auto& operator*() const { return IteratedContainer[GetIndex()]; }

            inline auto* operator->() { return &IteratedContainer[GetIndex()]; }
            inline const auto* operator->() const { return &IteratedContainer[GetIndex()]; }

            inline bool operator==(const TContainerIterator& Other) const { return &IteratedContainer == &Other.IteratedContainer && BitIterator == Other.BitIterator; }
            inline bool operator!=(const TContainerIterator& Other) const { return &IteratedContainer != &Other.IteratedContainer || BitIterator != Other.BitIterator; }
        };
    }

    inline Iterators::FSetBitIterator begin(const ContainerImpl::FBitArray& Array) { return Iterators::FSetBitIterator(Array, 0); }
    inline Iterators::FSetBitIterator end(const ContainerImpl::FBitArray& Array) { return Iterators::FSetBitIterator(Array, Array.Num()); }

    template <typename T>
    inline Iterators::TArrayIterator<T> begin(const TArray<T>& Array) { return Iterators::TArrayIterator<T>(Array, 0); }
    template <typename T>
    inline Iterators::TArrayIterator<T> end(const TArray<T>& Array) { return Iterators::TArrayIterator<T>(Array, Array.Num()); }

    template <typename T>
    inline Iterators::TSparseArrayIterator<T> begin(const TSparseArray<T>& Array) { return Iterators::TSparseArrayIterator<T>(Array, Array.GetAllocationFlags(), 0); }
    template <typename T>
    inline Iterators::TSparseArrayIterator<T> end(const TSparseArray<T>& Array) { return Iterators::TSparseArrayIterator<T>(Array, Array.GetAllocationFlags(), Array.NumAllocated()); }

    template <typename T>
    inline Iterators::TSetIterator<T> begin(const TSet<T>& Set) { return Iterators::TSetIterator<T>(Set, Set.GetAllocationFlags(), 0); }
    template <typename T>
    inline Iterators::TSetIterator<T> end(const TSet<T>& Set) { return Iterators::TSetIterator<T>(Set, Set.GetAllocationFlags(), Set.NumAllocated()); }

    template <typename T0, typename T1>
    inline Iterators::TMapIterator<T0, T1> begin(const TMap<T0, T1>& Map) { return Iterators::TMapIterator<T0, T1>(Map, Map.GetAllocationFlags(), 0); }
    template <typename T0, typename T1>
    inline Iterators::TMapIterator<T0, T1> end(const TMap<T0, T1>& Map) { return Iterators::TMapIterator<T0, T1>(Map, Map.GetAllocationFlags(), Map.NumAllocated()); }
}
