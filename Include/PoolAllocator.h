#pragma once
#include <type_traits>

template<typename T>
class PoolAllocator
{
private:
    struct FreeBlock
    {
        FreeBlock* next;
    };

    void* _memory = nullptr;
    FreeBlock* _freeList = nullptr;

    size_t _capacity = 0;

public:
    void Initialize(size_t capacity)
    {
        _capacity = capacity;

        _memory = ::operator new(sizeof(T) * capacity);

        _freeList = nullptr;

        for (size_t i = 0; i < capacity; ++i)
        {
            auto* block =
                reinterpret_cast<FreeBlock*>(
                    static_cast<char*>(_memory)
                    + i * sizeof(T)
                    );

            block->next = _freeList;
            _freeList = block;
        }
    }

    T* Allocate()
    {
        if (_freeList == nullptr)
            return nullptr;

        FreeBlock* block = _freeList;

        _freeList = _freeList->next;

        return new (block) T();
    }

    void Deallocate(T* object)
    {
        if (object == nullptr)
            return;

        object->~T();

        auto* block =
            reinterpret_cast<FreeBlock*>(object);

        block->next = _freeList;

        _freeList = block;
    }
};