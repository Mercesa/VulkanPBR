//
// Copyright (c) 2017 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#ifndef AMD_VULKAN_MEMORY_ALLOCATOR_H
#define AMD_VULKAN_MEMORY_ALLOCATOR_H

/** \mainpage Vulkan Memory Allocator

\tableofcontents

<b>Version 2.0.0-alpha.3</b> (2017-09-12)

Members grouped: see <a href="modules.html"><b>Modules</b></a>.

All members: see vk_mem_alloc.h.

\section problem Problem statement

Memory allocation and resource (buffer and image) creation in Vulkan is
difficult (comparing to older graphics API-s, like D3D11 or OpenGL) for several
reasons:

- It requires a lot of boilerplate code, just like everything else in Vulkan,
  because it is a low-level and high-performance API.
- There is additional level of indirection: `VkDeviceMemory` is allocated
  separately from creating `VkBuffer`/`VkImage` and they must be bound together. The
  binding cannot be changed later - resource must be recreated.
- Driver must be queried for supported memory heaps and memory types. Different
  IHV-s provide different types of it.
- It is recommended practice to allocate bigger chunks of memory and assign
  parts of them to particular resources.

\section features Features

This library is helps game developers to manage memory allocations and resource
creation by offering some higher-level functions. Features of the library could
be divided into several layers, low level to high level:

-# Functions that help to choose correct and optimal memory type based on
   intended usage of the memory.
   - Required or preferred traits of the memory are expressed using higher-level
     description comparing to Vulkan flags.
-# Functions that allocate memory blocks, reserve and return parts of them
   (`VkDeviceMemory` + offset + size) to the user.
   - Library keeps track of allocated memory blocks, used and unused ranges
     inside them, finds best matching unused ranges for new allocations, takes
     all the rules of alignment into consideration.
-# Functions that can create an image/buffer, allocate memory for it and bind
   them together - all in one call.

\section prequisites Prequisites

- Self-contained C++ library in single header file. No external dependencies
  other than standard C and C++ library and of course Vulkan.
- Public interface in C, in same convention as Vulkan API. Implementation in
  C++.
- Interface documented using Doxygen-style comments.
- Platform-independent, but developed and tested on Windows using Visual Studio.
- Error handling implemented by returning `VkResult` error codes - same way as in
  Vulkan.

\section user_guide User guide

\subsection quick_start Quick start

In your project code:

-# Include "vk_mem_alloc.h" file wherever you want to use the library.
-# In exacly one C++ file define following macro before include to build library
   implementation.


    #define VMA_IMPLEMENTATION
    #include "vk_mem_alloc.h"

At program startup:

-# Initialize Vulkan to have `VkPhysicalDevice` and `VkDevice` object.
-# Fill VmaAllocatorCreateInfo structure and create `VmaAllocator` object by
   calling vmaCreateAllocator().


    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;

    VmaAllocator allocator;
    vmaCreateAllocator(&allocatorInfo, &allocator);

When you want to create a buffer or image:

-# Fill `VkBufferCreateInfo` / `VkImageCreateInfo` structure.
-# Fill VmaAllocationCreateInfo structure.
-# Call vmaCreateBuffer() / vmaCreateImage() to get `VkBuffer`/`VkImage` with memory
   already allocated and bound to it.


    VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = myBufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VmaAllocationCreateInfo memReq = {};
    memReq.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VkBuffer buffer;
    VmaAllocation allocation;
    vmaCreateBuffer(allocator, &bufferInfo, &memReq, &buffer, &allocation, nullptr);

Don't forget to destroy your objects when no longer needed:


    vmaDestroyBuffer(allocator, buffer, allocation);
    vmaDestroyAllocator(allocator);

\subsection persistently_mapped_memory Persistently mapped memory

If you need to map memory on host, it may happen that two allocations are
assigned to the same `VkDeviceMemory` block, so if you map them both at the same
time, it will cause error because mapping single memory block multiple times is
illegal in Vulkan.

It is safer, more convenient and more efficient to use special feature designed
for that: persistently mapped memory. Allocations made with
`VMA_ALLOCATION_CREATE_PERSISTENT_MAP_BIT` flag set in
VmaAllocationCreateInfo::flags are returned from device memory
blocks that stay mapped all the time, so you can just access CPU pointer to it.
VmaAllocationInfo::pMappedData pointer is already offseted to the beginning of
particular allocation. Example:


    VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufCreateInfo.size = 1024;
    bufCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    allocCreateInfo.flags = VMA_ALLOCATION_CREATE_PERSISTENT_MAP_BIT;

    VkBuffer buf;
    VmaAllocation alloc;
    VmaAllocationInfo allocInfo;
    vmaCreateBuffer(allocator, &bufCreateInfo, &allocCreateInfo, &buf, &alloc, &allocInfo);

    .// Buffer is immediately mapped. You can access its memory.
    memcpy(allocInfo.pMappedData, myData, 1024);

Memory in Vulkan doesn't need to be unmapped before using it e.g. for transfers,
but if you are not sure whether it's `HOST_COHERENT` (here is surely is because
it's created with `VMA_MEMORY_USAGE_CPU_ONLY`), you should check it. If it's
not, you should call `vkInvalidateMappedMemoryRanges()` before reading and
`vkFlushMappedMemoryRanges()` after writing to mapped memory on CPU. Example:


    VkMemoryPropertyFlags memFlags;
    vmaGetMemoryTypeProperties(allocator, allocInfo.memoryType, &memFlags);
    if((memFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
    {
        VkMappedMemoryRange memRange = { VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE };
        memRange.memory = allocInfo.deviceMemory;
        memRange.offset = allocInfo.offset;
        memRange.size   = allocInfo.size;
        vkFlushMappedMemoryRanges(device, 1, &memRange);
    }

For performance reasons it is also recommended to unmap Vulkan memory for the
time of call to `vkQueueSubmit()` or `vkQueuePresent()`. You can do it for all
persistently mapped memory using just one function call. For details, see
function vmaUnmapPersistentlyMappedMemory(), vmaMapPersistentlyMappedMemory().

\subsection custom_memory_pools Custom memory pools

The library automatically creates and manages default memory pool for each
memory type available on the device. A pool contains a number of
`VkDeviceMemory` blocks. You can create custom pool and allocate memory out of
it. It can be useful if you want to:

- Keep certain kind of allocations separate from others.
- Enforce particular size of Vulkan memory blocks.
- Limit maximum amount of Vulkan memory allocated for that pool.

To use custom memory pools:

-# Fill VmaPoolCreateInfo structure.
-# Call vmaCreatePool() to obtain `VmaPool` handle.
-# When making an allocation, set VmaAllocationCreateInfo::pool to this handle.
   You don't need to specify any other parameters of this structure, like usage.

Example:


    .// Create a pool that could have at most 2 blocks, 128 MB each.
    VmaPoolCreateInfo poolCreateInfo = {};
    poolCreateInfo.memoryTypeIndex = ...
    poolCreateInfo.blockSize = 128ull * 1024 * 1024;
    poolCreateInfo.maxBlockCount = 2;

    VmaPool pool;
    vmaCreatePool(allocator, &poolCreateInfo, &pool);

    .// Allocate a buffer out of it.
    VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufCreateInfo.size = 1024;
    bufCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.pool = pool;

    VkBuffer buf;
    VmaAllocation alloc;
    VmaAllocationInfo allocInfo;
    vmaCreateBuffer(allocator, &bufCreateInfo, &allocCreateInfo, &buf, &alloc, &allocInfo);

You have to free all allocations made from this pool before destroying it.


    vmaDestroyBuffer(allocator, buf, alloc);
    vmaDestroyPool(allocator, pool);

\subsection defragmentation Defragmentation

Interleaved allocations and deallocations of many objects of varying size can
cause fragmentation, which can lead to a situation where the library is unable
to find a continuous range of free memory for a new allocation despite there is
enough free space, just scattered across many small free ranges between existing
allocations.

To mitigate this problem, you can use vmaDefragment(). Given set of allocations,
this function can move them to compact used memory, ensure more continuous free
space and possibly also free some `VkDeviceMemory`. It can work only on
allocations made from memory type that is `HOST_VISIBLE`. Allocations are
modified to point to the new `VkDeviceMemory` and offset. Data in this memory is
also `memmove`-ed to the new place. However, if you have images or buffers bound
to these allocations (and you certainly do), you need to destroy, recreate, and
bind them to the new place in memory.

For further details and example code, see documentation of function
vmaDefragment().

\subsection lost_allocations Lost allocations

If your game oversubscribes video memory, if may work OK in previous-generation
graphics APIs (DirectX 9, 10, 11, OpenGL) because resources are automatically
paged to system RAM. In Vulkan you can't do it because when you run out of
memory, an allocation just fails. If you have more data (e.g. textures) that can
fit into VRAM and you don't need it all at once, you may want to upload them to
GPU on demand and "push out" ones that are not used for a long time to make room
for the new ones, effectively using VRAM (or a cartain memory pool) as a form of
cache. Vulkan Memory Allocator can help you with that by supporting a concept of
"lost allocations".

To create an allocation that can become lost, include `VMA_ALLOCATION_CREATE_CAN_BECOME_LOST_BIT`
flag in VmaAllocationCreateInfo::flags. Before using a buffer or image bound to
such allocation in every new frame, you need to query it if it's not lost. To
check it: call vmaGetAllocationInfo() and see if VmaAllocationInfo::deviceMemory
is not `VK_NULL_HANDLE`. If the allocation is lost, you should not use it or
buffer/image bound to it. You mustn't forget to destroy this allocation and this
buffer/image.

To create an allocation that can make some other allocations lost to make room
for it, use `VMA_ALLOCATION_CREATE_CAN_MAKE_OTHER_LOST_BIT` flag. You will
usually use both flags `VMA_ALLOCATION_CREATE_CAN_MAKE_OTHER_LOST_BIT` and
`VMA_ALLOCATION_CREATE_CAN_BECOME_LOST_BIT` at the same time.

Warning! Current implementation uses quite naive, brute force algorithm,
which can make allocation calls that use `VMA_ALLOCATION_CREATE_CAN_MAKE_OTHER_LOST_BIT`
flag quite slow. A new, more optimal algorithm and data structure to speed this
up is planned for the future.

<b>When interleaving creation of new allocations with usage of existing ones,
how do you make sure that an allocation won't become lost while it's used in the
current frame?</b>

It is ensured because vmaGetAllocationInfo() not only returns allocation
parameters and checks whether it's not lost, but when it's not, it also
atomically marks it as used in the current frame, which makes it impossible to
become lost in that frame. It uses lockless algorithm, so it works fast and
doesn't involve locking any internal mutex.

<b>What if my allocation may still be in use by the GPU when it's rendering a
previous frame while I already submit new frame on the CPU?</b>

You can make sure that allocations "touched" by vmaGetAllocationInfo() will not
become lost for a number of additional frames back from the current one by
specifying this number as VmaAllocatorCreateInfo::frameInUseCount (for default
memory pool) and VmaPoolCreateInfo::frameInUseCount (for custom pool).

<b>How do you inform the library when new frame starts?</b>

You need to call function vmaSetCurrentFrameIndex().

Example code:


    struct MyBuffer
    {
        VkBuffer m_Buf = nullptr;
        VmaAllocation m_Alloc = nullptr;

        .// Called when the buffer is really needed in the current frame.
        void EnsureBuffer();
    };

    void MyBuffer::EnsureBuffer()
    {
        .// Buffer has been created.
        if(m_Buf != VK_NULL_HANDLE)
        {
            .// Check if its allocation is not lost + mark it as used in current frame.
            VmaAllocationInfo allocInfo;
            vmaGetAllocationInfo(allocator, m_Alloc, &allocInfo);
            if(allocInfo.deviceMemory != VK_NULL_HANDLE)
            {
                .// It's all OK - safe to use m_Buf.
                return;
            }
        }

        .// Buffer not yet exists or lost - destroy and recreate it.

        vmaDestroyBuffer(allocator, m_Buf, m_Alloc);

        VkBufferCreateInfo bufCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufCreateInfo.size = 1024;
        bufCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_CAN_BECOME_LOST_BIT |
            VMA_ALLOCATION_CREATE_CAN_MAKE_OTHER_LOST_BIT;

        vmaCreateBuffer(allocator, &bufCreateInfo, &allocCreateInfo, &m_Buf, &m_Alloc, nullptr);
    }

When using lost allocations, you may see some Vulkan validation layer warnings
about overlapping regions of memory bound to different kinds of buffers and
images. This is still valid as long as you implement proper handling of lost
allocations (like in the example above) and don't use them.

The library uses following algorithm for allocation, in order:

-# Try to find free range of memory in existing blocks.
-# If failed, try to create a new block of `VkDeviceMemory`, with preferred block size.
-# If failed, try to create such block with size/2 and size/4.
-# If failed and `VMA_ALLOCATION_CREATE_CAN_MAKE_OTHER_LOST_BIT` flag was
   specified, try to find space in existing blocks, possilby making some other
   allocations lost.
-# If failed, try to allocate separate `VkDeviceMemory` for this allocation,
   just like when you use `VMA_ALLOCATION_CREATE_OWN_MEMORY_BIT`.
-# If failed, choose other memory type that meets the requirements specified in
   VmaAllocationCreateInfo and go to point 1.
-# If failed, return `VK_ERROR_OUT_OF_DEVICE_MEMORY`.


\section configuration Configuration

Please check "CONFIGURATION SECTION" in the code to find macros that you can define
before each `#include` of this file or change directly in this file to provide
your own implementation of basic facilities like assert, min and max functions,
mutex etc. C++ STL is used by default, but changing these allows you to get rid
of any STL usage if you want, as many game developers tend to do.

\subsection custom_memory_allocator Custom memory allocator

You can use custom memory allocator by filling optional member
VmaAllocatorCreateInfo::pAllocationCallbacks. These functions will be passed to
Vulkan, as well as used by the library itself to make any CPU-side allocations.

\section thread_safety Thread safety

- The library has no global state, so separate `VmaAllocator` objects can be used
  independently. 
- By default, all calls to functions that take `VmaAllocator` as first parameter
  are safe to call from multiple threads simultaneously because they are
  synchronized internally when needed.
- When the allocator is created with `VMA_ALLOCATOR_EXTERNALLY_SYNCHRONIZED_BIT`
  flag, calls to functions that take such `VmaAllocator` object must be
  synchronized externally.
- Access to a `VmaAllocation` object must be externally synchronized. For example,
  you must not call vmaGetAllocationInfo() and vmaDefragment() from different
  threads at the same time if you pass the same `VmaAllocation` object to these
  functions.

*/

#include <vulkan/vulkan.h>

////////////////////////////////////////////////////////////////////////////////
/** \defgroup general General
@{
*/

VK_DEFINE_HANDLE(VmaAllocator)

/// Callback function called after successful vkAllocateMemory.
typedef void (VKAPI_PTR *PFN_vmaAllocateDeviceMemoryFunction)(
    VmaAllocator      allocator,
    uint32_t          memoryType,
    VkDeviceMemory    memory,
    VkDeviceSize      size);
/// Callback function called before vkFreeMemory.
typedef void (VKAPI_PTR *PFN_vmaFreeDeviceMemoryFunction)(
    VmaAllocator      allocator,
    uint32_t          memoryType,
    VkDeviceMemory    memory,
    VkDeviceSize      size);

/** \brief Set of callbacks that the library will call for vkAllocateMemory and vkFreeMemory.

Provided for informative purpose, e.g. to gather statistics about number of
allocations or total amount of memory allocated in Vulkan.
*/
typedef struct VmaDeviceMemoryCallbacks {
    /// Optional, can be null.
    PFN_vmaAllocateDeviceMemoryFunction pfnAllocate;
    /// Optional, can be null.
    PFN_vmaFreeDeviceMemoryFunction pfnFree;
} VmaDeviceMemoryCallbacks;

/// Flags for created VmaAllocator.
typedef enum VmaAllocatorFlagBits {
    /** \brief Allocator and all objects created from it will not be synchronized internally, so you must guarantee they are used from only one thread at a time or synchronized externally by you.

    Using this flag may increase performance because internal mutexes are not used.
    */
    VMA_ALLOCATOR_EXTERNALLY_SYNCHRONIZED_BIT = 0x00000001,

    VMA_ALLOCATOR_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
} VmaAllocatorFlagBits;
typedef VkFlags VmaAllocatorFlags;

/// Description of a Allocator to be created.
typedef struct VmaAllocatorCreateInfo
{
    /// Flags for created allocator. Use VmaAllocatorFlagBits enum.
    VmaAllocatorFlags flags;
    /// Vulkan physical device.
    /** It must be valid throughout whole lifetime of created allocator. */
    VkPhysicalDevice physicalDevice;
    /// Vulkan device.
    /** It must be valid throughout whole lifetime of created allocator. */
    VkDevice device;
    /// Size of a single memory block to allocate for resources.
    /** Set to 0 to use default, which is currently 256 MB. */
    VkDeviceSize preferredLargeHeapBlockSize;
    /// Size of a single memory block to allocate for resources from a small heap <= 512 MB.
    /** Set to 0 to use default, which is currently 64 MB. */
    VkDeviceSize preferredSmallHeapBlockSize;
    /// Custom CPU memory allocation callbacks.
    /** Optional, can be null. When specified, will also be used for all CPU-side memory allocations. */
    const VkAllocationCallbacks* pAllocationCallbacks;
    /// Informative callbacks for vkAllocateMemory, vkFreeMemory.
    /** Optional, can be null. */
    const VmaDeviceMemoryCallbacks* pDeviceMemoryCallbacks;
    /** \brief Maximum number of additional frames that are in use at the same time as current frame.

    This value is used only when you make allocations with
    VMA_ALLOCATION_CREATE_CAN_BECOME_LOST_BIT flag. Such allocation cannot become
    lost if allocation.lastUseFrameIndex >= allocator.currentFrameIndex - frameInUseCount.

    For example, if you double-buffer your command buffers, so resources used for
    rendering in previous frame may still be in use by the GPU at the moment you
    allocate resources needed for the current frame, set this value to 1.

    If you want to allow any allocations other than used in the current frame to
    become lost, set this value to 0.
    */
    uint32_t frameInUseCount;
} VmaAllocatorCreateInfo;

/// Creates Allocator object.
VkResult vmaCreateAllocator(
    const VmaAllocatorCreateInfo* pCreateInfo,
    VmaAllocator* pAllocator);

/// Destroys allocator object.
void vmaDestroyAllocator(
    VmaAllocator allocator);

/**
PhysicalDeviceProperties are fetched from physicalDevice by the allocator.
You can access it here, without fetching it again on your own.
*/
void vmaGetPhysicalDeviceProperties(
    VmaAllocator allocator,
    const VkPhysicalDeviceProperties** ppPhysicalDeviceProperties);

/**
PhysicalDeviceMemoryProperties are fetched from physicalDevice by the allocator.
You can access it here, without fetching it again on your own.
*/
void vmaGetMemoryProperties(
    VmaAllocator allocator,
    const VkPhysicalDeviceMemoryProperties** ppPhysicalDeviceMemoryProperties);

/**
\brief Given Memory Type Index, returns Property Flags of this memory type.

This is just a convenience function. Same information can be obtained using
vmaGetMemoryProperties().
*/
void vmaGetMemoryTypeProperties(
    VmaAllocator allocator,
    uint32_t memoryTypeIndex,
    VkMemoryPropertyFlags* pFlags);

/** \brief Sets index of the current frame.

This function must be used if you make allocations with
`VMA_ALLOCATION_CREATE_CAN_BECOME_LOST_BIT` and
`VMA_ALLOCATION_CREATE_CAN_MAKE_OTHER_LOST_BIT` flags to inform the allocator
when a new frame begins. Allocations queried using vmaGetAllocationInfo() cannot
become lost in the current frame.
*/
void vmaSetCurrentFrameIndex(
    VmaAllocator allocator,
    uint32_t frameIndex);

typedef struct VmaStatInfo
{
    /// Number of `VkDeviceMemory` Vulkan memory blocks allocated.
    uint32_t BlockCount;
    /// Number of `VmaAllocation` allocation objects allocated.
    uint32_t AllocationCount;
    /// Number of free ranges of memory between allocations.
    uint32_t UnusedRangeCount;
    /// Total number of bytes occupied by all allocations.
    VkDeviceSize UsedBytes;
    /// Total number of bytes occupied by unused ranges.
    VkDeviceSize UnusedBytes;
    VkDeviceSize AllocationSizeMin, AllocationSizeAvg, AllocationSizeMax;
    VkDeviceSize UnusedRangeSizeMin, UnusedRangeSizeAvg, UnusedRangeSizeMax;
} VmaStatInfo;

/// General statistics from current state of Allocator.
struct VmaStats
{
    VmaStatInfo memoryType[VK_MAX_MEMORY_TYPES];
    VmaStatInfo memoryHeap[VK_MAX_MEMORY_HEAPS];
    VmaStatInfo total;
};

/// Retrieves statistics from current state of the Allocator.
void vmaCalculateStats(
    VmaAllocator allocator,
    VmaStats* pStats);

#define VMA_STATS_STRING_ENABLED 1

#if VMA_STATS_STRING_ENABLED

/// Builds and returns statistics as string in JSON format.
/** @param[out] ppStatsString Must be freed using vmaFreeStatsString() function.
*/
void vmaBuildStatsString(
    VmaAllocator allocator,
    char** ppStatsString,
    VkBool32 detailedMap);

void vmaFreeStatsString(
    VmaAllocator allocator,
    char* pStatsString);

#endif // #if VMA_STATS_STRING_ENABLED

/** @} */

////////////////////////////////////////////////////////////////////////////////
/** \defgroup layer1 Layer 1 Choosing Memory Type
@{
*/

VK_DEFINE_HANDLE(VmaPool)

typedef enum VmaMemoryUsage
{
    /// No intended memory usage specified.
    VMA_MEMORY_USAGE_UNKNOWN = 0,
    /// Memory will be used on device only, so faster access from the device is preferred. No need to be mappable on host.
    VMA_MEMORY_USAGE_GPU_ONLY = 1,
    /// Memory will be mapped on host. Could be used for transfer to/from device.
    /** Guarantees to be `HOST_VISIBLE` and `HOST_COHERENT`. */
    VMA_MEMORY_USAGE_CPU_ONLY = 2,
    /// Memory will be used for frequent (dynamic) updates from host and reads on device (upload).
    /** Guarantees to be `HOST_VISIBLE`. */
    VMA_MEMORY_USAGE_CPU_TO_GPU = 3,
    /// Memory will be used for frequent writing on device and readback on host (download).
    /** Guarantees to be `HOST_VISIBLE`. */
    VMA_MEMORY_USAGE_GPU_TO_CPU = 4,
    VMA_MEMORY_USAGE_MAX_ENUM = 0x7FFFFFFF
} VmaMemoryUsage;

/// Flags to be passed as VmaAllocationCreateInfo::flags.
typedef enum VmaAllocationCreateFlagBits {
    /** \brief Set this flag if the allocation should have its own memory block.
    
    Use it for special, big resources, like fullscreen images used as attachments.
   
    This flag must also be used for host visible resources that you want to map
    simultaneously because otherwise they might end up as regions of the same
    `VkDeviceMemory`, while mapping same `VkDeviceMemory` multiple times
    simultaneously is illegal.

    You should not use this flag if VmaAllocationCreateInfo::pool is not null.
    */
    VMA_ALLOCATION_CREATE_OWN_MEMORY_BIT = 0x00000001,

    /** \brief Set this flag to only try to allocate from existing `VkDeviceMemory` blocks and never create new such block.
    
    If new allocation cannot be placed in any of the existing blocks, allocation
    fails with `VK_ERROR_OUT_OF_DEVICE_MEMORY` error.
    
    You should not use `VMA_ALLOCATION_CREATE_OWN_MEMORY_BIT` and
    `VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT` at the same time. It makes no sense.
    
    If VmaAllocationCreateInfo::pool is not null, this flag is implied and ignored. */
    VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT = 0x00000002,
    /** \brief Set this flag to use a memory that will be persistently mapped and retrieve pointer to it.
    
    Pointer to mapped memory will be returned through VmaAllocationInfo::pMappedData. You cannot
    map the memory on your own as multiple mappings of a single `VkDeviceMemory` are
    illegal.

    If VmaAllocationCreateInfo::pool is not null, usage of this flag must match
    usage of flag `VMA_POOL_CREATE_PERSISTENT_MAP_BIT` used during pool creation.
    */
    VMA_ALLOCATION_CREATE_PERSISTENT_MAP_BIT = 0x00000004,
    /** Allocation created with this flag can become lost as a result of another
    allocation with `VMA_ALLOCATION_CREATE_CAN_MAKE_OTHER_LOST_BIT` flag, so you
    must check it before use.

    To check if allocation is not lost, call vmaGetAllocationInfo() and check if
    VmaAllocationInfo::deviceMemory is not `VK_NULL_HANDLE`.

    For details about supporting lost allocations, see Lost Allocations
    chapter of User Guide on Main Page.
    */
    VMA_ALLOCATION_CREATE_CAN_BECOME_LOST_BIT = 0x00000008,
    /** While creating allocation using this flag, other allocations that were
    created with flag `VMA_ALLOCATION_CREATE_CAN_BECOME_LOST_BIT` can become lost.

    For details about supporting lost allocations, see Lost Allocations
    chapter of User Guide on Main Page.
    */
    VMA_ALLOCATION_CREATE_CAN_MAKE_OTHER_LOST_BIT = 0x00000010,

    VMA_ALLOCATION_CREATE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
} VmaAllocationCreateFlagBits;
typedef VkFlags VmaAllocationCreateFlags;

typedef struct VmaAllocationCreateInfo
{
    /// Use VmaAllocationCreateFlagBits enum.
    VmaAllocationCreateFlags flags;
    /** \brief Intended usage of memory.
    
    Leave `VMA_MEMORY_USAGE_UNKNOWN` if you specify `requiredFlags`. You can also use both. \n
    If `pool` is not null, this member is ignored.
    */
    VmaMemoryUsage usage;
    /** \brief Flags that must be set in a Memory Type chosen for an allocation.
    
    Leave 0 if you specify requirement via usage. \n
    If `pool` is not null, this member is ignored.*/
    VkMemoryPropertyFlags requiredFlags;
    /** \brief Flags that preferably should be set in a Memory Type chosen for an allocation.
    
    Set to 0 if no additional flags are prefered and only `requiredFlags` should be used. \n
    If not 0, it must be a superset or equal to `requiredFlags`. \n
    If `pool` is not null, this member is ignored. */
    VkMemoryPropertyFlags preferredFlags;
    /** \brief Custom general-purpose pointer that will be stored in VmaAllocation, can be read as VmaAllocationInfo::pUserData and changed using vmaSetAllocationUserData(). */
    void* pUserData;
    /** \brief Pool that this allocation should be created in.

    Leave `VK_NULL_HANDLE` to allocate from general memory.
    */
    VmaPool pool;
} VmaAllocationCreateInfo;

/**
This algorithm tries to find a memory type that:

- Is allowed by memoryTypeBits.
- Contains all the flags from pAllocationCreateInfo->requiredFlags.
- Matches intended usage.
- Has as many flags from pAllocationCreateInfo->preferredFlags as possible.

\return Returns VK_ERROR_FEATURE_NOT_PRESENT if not found. Receiving such result
from this function or any other allocating function probably means that your
device doesn't support any memory type with requested features for the specific
type of resource you want to use it for. Please check parameters of your
resource, like image layout (OPTIMAL versus LINEAR) or mip level count.
*/
VkResult vmaFindMemoryTypeIndex(
    VmaAllocator allocator,
    uint32_t memoryTypeBits,
    const VmaAllocationCreateInfo* pAllocationCreateInfo,
    uint32_t* pMemoryTypeIndex);

/** @} */

////////////////////////////////////////////////////////////////////////////////
/** \defgroup layer2 Layer 2 Allocating Memory
@{
*/

/// Flags to be passed as VmaPoolCreateInfo::flags.
typedef enum VmaPoolCreateFlagBits {
    /** Set this flag to use a memory that will be persistently mapped.

    Each allocation made from this pool will have VmaAllocationInfo::pMappedData
    available.
    
    Usage of this flag must match usage of VMA_ALLOCATION_CREATE_PERSISTENT_MAP_BIT
    flag for every allocation made from this pool.
    */
    VMA_POOL_CREATE_PERSISTENT_MAP_BIT = 0x00000001,
    /** \brief Use this flag if you always allocate only buffers and linear images or only optimal images out of this pool and so Buffer-Image Granularity can be ignored.

    This is na optional optimization flag.

    If you always allocate using vmaCreateBuffer(), vmaCreateImage(),
    vmaAllocateMemoryForBuffer(), then you don't need to use it because allocator
    knows exact type of your allocations so it can handle Buffer-Image Granularity
    in the optimal way.

    If you also allocate using vmaAllocateMemoryForImage() or vmaAllocateMemory(),
    exact type of such allocations is not known, so allocator must be conservative
    in handling Buffer-Image Granularity, which can lead to suboptimal allocation
    (wasted memory). In that case, if you can make sure you always allocate only
    buffers and linear images or only optimal images out of this pool, use this flag
    to make allocator disregard Buffer-Image Granularity and so make allocations
    more optimal.
    */
    VMA_POOL_CREATE_IGNORE_BUFFER_IMAGE_GRANULARITY_BIT = 0x00000002,

    VMA_POOL_CREATE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
} VmaPoolCreateFlagBits;
typedef VkFlags VmaPoolCreateFlags;

/** \brief Describes parameter of created `VmaPool`.
*/
typedef struct VmaPoolCreateInfo {
    /** \brief Vulkan memory type index to allocate this pool from.
    */
    uint32_t memoryTypeIndex;
    /** \brief Use combination of `VmaPoolCreateFlagBits`.
    */
    VmaPoolCreateFlags flags;
    /** \brief Size of a single `VkDeviceMemory` block to be allocated as part of this pool, in bytes.

    Optional. Leave 0 to use default.
    */
    VkDeviceSize blockSize;
    /** \brief Minimum number of blocks to be always allocated in this pool, even if they stay empty.

    Set to 0 to have no preallocated blocks and let the pool be completely empty.
    */
    size_t minBlockCount;
    /** \brief Maximum number of blocks that can be allocated in this pool.

    Optional. Set to 0 to use `SIZE_MAX`, which means no limit.
    
    Set to same value as minBlockCount to have fixed amount of memory allocated
    throuout whole lifetime of this pool.
    */
    size_t maxBlockCount;
    /** \brief Maximum number of additional frames that are in use at the same time as current frame.

    This value is used only when you make allocations with
    `VMA_ALLOCATION_CREATE_CAN_BECOME_LOST_BIT` flag. Such allocation cannot become
    lost if allocation.lastUseFrameIndex >= allocator.currentFrameIndex - frameInUseCount.

    For example, if you double-buffer your command buffers, so resources used for
    rendering in previous frame may still be in use by the GPU at the moment you
    allocate resources needed for the current frame, set this value to 1.

    If you want to allow any allocations other than used in the current frame to
    become lost, set this value to 0.
    */
    uint32_t frameInUseCount;
} VmaPoolCreateInfo;

/** \brief Describes parameter of existing VmaPool.
*/
typedef struct VmaPoolStats {
    /** \brief Total amount of `VkDeviceMemory` allocated from Vulkan for this pool, in bytes.
    */
    VkDeviceSize size;
    /** \brief Total number of bytes in the pool not used by any `VmaAllocation`.
    */
    VkDeviceSize unusedSize;
    /** \brief Number of VmaAllocation objects created from this pool that were not destroyed or lost.
    */
    size_t allocationCount;
    /** \brief Number of continuous memory ranges in the pool not used by any `VmaAllocation`.
    */
    size_t unusedRangeCount;
} VmaPoolStats;

/** \brief Allocates Vulkan device memory and creates `VmaPool` object.

@param allocator Allocator object.
@param pCreateInfo Parameters of pool to create.
@param[out] pPool Handle to created pool.
*/
VkResult vmaCreatePool(
	VmaAllocator allocator,
	const VmaPoolCreateInfo* pCreateInfo,
	VmaPool* pPool);

/** \brief Destroys VmaPool object and frees Vulkan device memory.
*/
void vmaDestroyPool(
    VmaAllocator allocator,
    VmaPool pool);

/** \brief Retrieves statistics of existing VmaPool object.

@param allocator Allocator object.
@param pool Pool object.
@param[out] pPoolStats Statistics of specified pool.
*/
void vmaGetPoolStats(
    VmaAllocator allocator,
    VmaPool pool,
    VmaPoolStats* pPoolStats);

/** \brief Marks all allocations in given pool as lost if they are not used in current frame or VmaPoolCreateInfo::frameInUseCount back from now.

@param allocator Allocator object.
@param pool Pool.
@param[out] pLostAllocationCount Number of allocations marked as lost. Optional - pass null if you don't need this information.
*/
void vmaMakePoolAllocationsLost(
    VmaAllocator allocator,
    VmaPool pool,
    size_t* pLostAllocationCount);

VK_DEFINE_HANDLE(VmaAllocation)

/** \brief Parameters of `VmaAllocation` objects, that can be retrieved using function vmaGetAllocationInfo().
*/
typedef struct VmaAllocationInfo {
    /** \brief Memory type index that this allocation was allocated from.
    
    It never changes.
    */
    uint32_t memoryType;
    /** \brief Handle to Vulkan memory object.

    Same memory object can be shared by multiple allocations.
    
    It can change after call to vmaDefragment() if this allocation is passed to the function, or if allocation is lost.

    If the allocation is lost, it is equal to `VK_NULL_HANDLE`.
    */
    VkDeviceMemory deviceMemory;
    /** \brief Offset into deviceMemory object to the beginning of this allocation, in bytes. (deviceMemory, offset) pair is unique to this allocation.

    It can change after call to vmaDefragment() if this allocation is passed to the function, or if allocation is lost.
    */
    VkDeviceSize offset;
    /** \brief Size of this allocation, in bytes.

    It never changes, unless allocation is lost.
    */
    VkDeviceSize size;
    /** \brief Pointer to the beginning of this allocation as mapped data. Null if this alloaction is not persistently mapped.

    It can change after call to vmaUnmapPersistentlyMappedMemory(), vmaMapPersistentlyMappedMemory().
    It can also change after call to vmaDefragment() if this allocation is passed to the function.
    */
    void* pMappedData;
    /** \brief Custom general-purpose pointer that was passed as VmaAllocationCreateInfo::pUserData or set using vmaSetAllocationUserData().

    It can change after call to vmaSetAllocationUserData() for this allocation.
    */
    void* pUserData;
} VmaAllocationInfo;

/** \brief General purpose memory allocation.

@param[out] pAllocation Handle to allocated memory.
@param[out] pAllocationInfo Optional. Information about allocated memory. It can be later fetched using function vmaGetAllocationInfo().

You should free the memory using vmaFreeMemory().

It is recommended to use vmaAllocateMemoryForBuffer(), vmaAllocateMemoryForImage(),
vmaCreateBuffer(), vmaCreateImage() instead whenever possible.
*/
VkResult vmaAllocateMemory(
    VmaAllocator allocator,
    const VkMemoryRequirements* pVkMemoryRequirements,
    const VmaAllocationCreateInfo* pCreateInfo,
    VmaAllocation* pAllocation,
    VmaAllocationInfo* pAllocationInfo);

/**
@param[out] pAllocation Handle to allocated memory.
@param[out] pAllocationInfo Optional. Information about allocated memory. It can be later fetched using function vmaGetAllocationInfo().

You should free the memory using vmaFreeMemory().
*/
VkResult vmaAllocateMemoryForBuffer(
    VmaAllocator allocator,
    VkBuffer buffer,
    const VmaAllocationCreateInfo* pCreateInfo,
    VmaAllocation* pAllocation,
    VmaAllocationInfo* pAllocationInfo);

/// Function similar to vmaAllocateMemoryForBuffer().
VkResult vmaAllocateMemoryForImage(
    VmaAllocator allocator,
    VkImage image,
    const VmaAllocationCreateInfo* pCreateInfo,
    VmaAllocation* pAllocation,
    VmaAllocationInfo* pAllocationInfo);

/// Frees memory previously allocated using vmaAllocateMemory(), vmaAllocateMemoryForBuffer(), or vmaAllocateMemoryForImage().
void vmaFreeMemory(
    VmaAllocator allocator,
    VmaAllocation allocation);

/// Returns current information about specified allocation.
void vmaGetAllocationInfo(
    VmaAllocator allocator,
    VmaAllocation allocation,
    VmaAllocationInfo* pAllocationInfo);

/// Sets pUserData in given allocation to new value.
void vmaSetAllocationUserData(
    VmaAllocator allocator,
    VmaAllocation allocation,
    void* pUserData);

/** \brief Creates new allocation that is in lost state from the beginning.

It can be useful if you need a dummy, non-null allocation.

You still need to destroy created object using vmaFreeMemory().

Returned allocation is not tied to any specific memory pool or memory type and
not bound to any image or buffer. It has size = 0. It cannot be turned into
a real, non-empty allocation.
*/
void vmaCreateLostAllocation(
    VmaAllocator allocator,
    VmaAllocation* pAllocation);

/**
Feel free to use vkMapMemory on these memory blocks on you own if you want, but
just for convenience and to make sure correct offset and size is always
specified, usage of vmaMapMemory() / vmaUnmapMemory() is recommended.

Do not use it on memory allocated with `VMA_ALLOCATION_CREATE_PERSISTENT_MAP_BIT`
as multiple maps to same `VkDeviceMemory` is illegal.
*/
VkResult vmaMapMemory(
    VmaAllocator allocator,
    VmaAllocation allocation,
    void** ppData);

void vmaUnmapMemory(
    VmaAllocator allocator,
    VmaAllocation allocation);

/** \brief Unmaps persistently mapped memory of types that are `HOST_COHERENT` and `DEVICE_LOCAL`.

This is optional performance optimization. On Windows you should call it before
every call to `vkQueueSubmit` and `vkQueuePresent`. After which you can remap the
allocations again using vmaMapPersistentlyMappedMemory(). This is because of the
internal behavior of WDDM. Example:


    vmaUnmapPersistentlyMappedMemory(allocator);
    vkQueueSubmit(...)
    vmaMapPersistentlyMappedMemory(allocator);

After this call VmaAllocationInfo::pMappedData of some allocations may become null.

This call is reference-counted. Memory is mapped again after you call
vmaMapPersistentlyMappedMemory() same number of times that you called
vmaUnmapPersistentlyMappedMemory().
*/
void vmaUnmapPersistentlyMappedMemory(VmaAllocator allocator);

/** \brief Maps back persistently mapped memory of types that are `HOST_COHERENT` and `DEVICE_LOCAL`.

See vmaUnmapPersistentlyMappedMemory().

After this call VmaAllocationInfo::pMappedData of some allocation may have value
different than before calling vmaUnmapPersistentlyMappedMemory().
*/
VkResult vmaMapPersistentlyMappedMemory(VmaAllocator allocator);

/** \brief Optional configuration parameters to be passed to function vmaDefragment(). */
typedef struct VmaDefragmentationInfo {
    /** \brief Maximum total numbers of bytes that can be copied while moving allocations to different places.
    
    Default is `VK_WHOLE_SIZE`, which means no limit.
    */
    VkDeviceSize maxBytesToMove;
    /** \brief Maximum number of allocations that can be moved to different place.

    Default is `UINT32_MAX`, which means no limit.
    */
    uint32_t maxAllocationsToMove;
} VmaDefragmentationInfo;

/** \brief Statistics returned by function vmaDefragment(). */
typedef struct VmaDefragmentationStats {
    /// Total number of bytes that have been copied while moving allocations to different places.
    VkDeviceSize bytesMoved;
    /// Total number of bytes that have been released to the system by freeing empty `VkDeviceMemory` objects.
    VkDeviceSize bytesFreed;
    /// Number of allocations that have been moved to different places.
    uint32_t allocationsMoved;
    /// Number of empty `VkDeviceMemory` objects that have been released to the system.
    uint32_t deviceMemoryBlocksFreed;
} VmaDefragmentationStats;

/** \brief Compacts memory by moving allocations.

@param pAllocations Array of allocations that can be moved during this compation.
@param allocationCount Number of elements in pAllocations and pAllocationsChanged arrays.
@param[out] pAllocationsChanged Array of boolean values that will indicate whether matching allocation in pAllocations array has been moved. This parameter is optional. Pass null if you don't need this information.
@param pDefragmentationInfo Configuration parameters. Optional - pass null to use default values.
@param[out] pDefragmentationStats Statistics returned by the function. Optional - pass null if you don't need this information.
@return VK_SUCCESS if completed, VK_INCOMPLETE if succeeded but didn't make all possible optimizations because limits specified in pDefragmentationInfo have been reached, negative error code in case of error.

This function works by moving allocations to different places (different
`VkDeviceMemory` objects and/or different offsets) in order to optimize memory
usage. Only allocations that are in pAllocations array can be moved. All other
allocations are considered nonmovable in this call. Basic rules:

- Only allocations made in memory types that have
  `VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT` flag can be compacted. You may pass other
  allocations but it makes no sense - these will never be moved.
- You may pass allocations made with `VMA_ALLOCATION_CREATE_OWN_MEMORY_BIT` but
  it makes no sense - they will never be moved.
- Both allocations made with or without `VMA_ALLOCATION_CREATE_PERSISTENT_MAP_BIT`
  flag can be compacted. If not persistently mapped, memory will be mapped
  temporarily inside this function if needed, so it shouldn't be mapped by you for
  the time of this call.
- You must not pass same `VmaAllocation` object multiple times in pAllocations array.

The function also frees empty `VkDeviceMemory` blocks.

After allocation has been moved, its VmaAllocationInfo::deviceMemory and/or
VmaAllocationInfo::offset changes. You must query them again using
vmaGetAllocationInfo() if you need them.

If an allocation has been moved, data in memory is copied to new place
automatically, but if it was bound to a buffer or an image, you must destroy
that object yourself, create new one and bind it to the new memory pointed by
the allocation. You must use `vkDestroyBuffer()`, `vkDestroyImage()`,
`vkCreateBuffer()`, `vkCreateImage()` for that purpose and NOT vmaDestroyBuffer(),
vmaDestroyImage(), vmaCreateBuffer(), vmaCreateImage()! Example:


    VkDevice device = ...;
    VmaAllocator allocator = ...;
    std::vector<VkBuffer> buffers = ...;
    std::vector<VmaAllocation> allocations = ...;

    std::vector<VkBool32> allocationsChanged(allocations.size());
    vmaDefragment(allocator, allocations.data(), allocations.size(), allocationsChanged.data(), nullptr, nullptr);

    for(size_t i = 0; i < allocations.size(); ++i)
    {
        if(allocationsChanged[i])
        {
            VmaAllocationInfo allocInfo;
            vmaGetAllocationInfo(allocator, allocations[i], &allocInfo);

            vkDestroyBuffer(device, buffers[i], nullptr);

            VkBufferCreateInfo bufferInfo = ...;
            vkCreateBuffer(device, &bufferInfo, nullptr, &buffers[i]);
            
            .// You can make dummy call to vkGetBufferMemoryRequirements here to silence validation layer warning.
            
            vkBindBufferMemory(device, buffers[i], allocInfo.deviceMemory, allocInfo.offset);
        }
    }

This function may be time-consuming, so you shouldn't call it too often (like
every frame or after every resource creation/destruction), but rater you can
call it on special occasions (like when reloading a game level, when you just
destroyed a lot of objects).
*/
VkResult vmaDefragment(
    VmaAllocator allocator,
    VmaAllocation* pAllocations,
    size_t allocationCount,
    VkBool32* pAllocationsChanged,
    const VmaDefragmentationInfo *pDefragmentationInfo,
    VmaDefragmentationStats* pDefragmentationStats);

/** @} */

////////////////////////////////////////////////////////////////////////////////
/** \defgroup layer3 Layer 3 Creating Buffers and Images
@{
*/

/**
@param[out] pBuffer Buffer that was created.
@param[out] pAllocation Allocation that was created.
@param[out] pAllocationInfo Optional. Information about allocated memory. It can be later fetched using function vmaGetAllocationInfo().

This function automatically:

-# Creates buffer.
-# Allocates appropriate memory for it.
-# Binds the buffer with the memory.

If any of these operations fail, buffer and allocation are not created,
returned value is negative error code, *pBuffer and *pAllocation are null.

If the function succeeded, you must destroy both buffer and allocation when you
no longer need them using either convenience function vmaDestroyBuffer() or
separately, using vkDestroyBuffer() and vmaFreeMemory().
*/
VkResult vmaCreateBuffer(
    VmaAllocator allocator,
    const VkBufferCreateInfo* pBufferCreateInfo,
    const VmaAllocationCreateInfo* pAllocationCreateInfo,
    VkBuffer* pBuffer,
    VmaAllocation* pAllocation,
    VmaAllocationInfo* pAllocationInfo);

void vmaDestroyBuffer(
    VmaAllocator allocator,
    VkBuffer buffer,
    VmaAllocation allocation);

/// Function similar to vmaCreateBuffer().
VkResult vmaCreateImage(
    VmaAllocator allocator,
    const VkImageCreateInfo* pImageCreateInfo,
    const VmaAllocationCreateInfo* pAllocationCreateInfo,
    VkImage* pImage,
    VmaAllocation* pAllocation,
    VmaAllocationInfo* pAllocationInfo);

void vmaDestroyImage(
    VmaAllocator allocator,
    VkImage image,
    VmaAllocation allocation);

/** @} */

#endif // AMD_VULKAN_MEMORY_ALLOCATOR_H

// For Visual Studio IntelliSense.
#ifdef __INTELLISENSE__
#define VMA_IMPLEMENTATION
#endif

#ifdef VMA_IMPLEMENTATION
#undef VMA_IMPLEMENTATION

#include <cstdint>
#include <cstdlib>
#include <cstring>

/*******************************************************************************
CONFIGURATION SECTION

Define some of these macros before each #include of this header or change them
here if you need other then default behavior depending on your environment.
*/

// Define this macro to 1 to make the library use STL containers instead of its own implementation.
//#define VMA_USE_STL_CONTAINERS 1

/* Set this macro to 1 to make the library including and using STL containers:
std::pair, std::vector, std::list, std::unordered_map.

Set it to 0 or undefined to make the library using its own implementation of
the containers.
*/
#if VMA_USE_STL_CONTAINERS
   #define VMA_USE_STL_VECTOR 1
   #define VMA_USE_STL_UNORDERED_MAP 1
   #define VMA_USE_STL_LIST 1
#endif

#if VMA_USE_STL_VECTOR
   #include <vector>
#endif

#if VMA_USE_STL_UNORDERED_MAP
   #include <unordered_map>
#endif

#if VMA_USE_STL_LIST
   #include <list>
#endif

/*
Following headers are used in this CONFIGURATION section only, so feel free to
remove them if not needed.
*/
#include <cassert> // for assert
#include <algorithm> // for min, max
#include <mutex> // for std::mutex
#include <atomic> // for std::atomic

#if !defined(_WIN32)
    #include <malloc.h> // for aligned_alloc()
#endif

// Normal assert to check for programmer's errors, especially in Debug configuration.
#ifndef VMA_ASSERT
   #ifdef _DEBUG
       #define VMA_ASSERT(expr)         assert(expr)
   #else
       #define VMA_ASSERT(expr)
   #endif
#endif

// Assert that will be called very often, like inside data structures e.g. operator[].
// Making it non-empty can make program slow.
#ifndef VMA_HEAVY_ASSERT
   #ifdef _DEBUG
       #define VMA_HEAVY_ASSERT(expr)   //VMA_ASSERT(expr)
   #else
       #define VMA_HEAVY_ASSERT(expr)
   #endif
#endif

#ifndef VMA_NULL
   // Value used as null pointer. Define it to e.g.: nullptr, NULL, 0, (void*)0.
   #define VMA_NULL   nullptr
#endif

#ifndef VMA_ALIGN_OF
   #define VMA_ALIGN_OF(type)       (__alignof(type))
#endif

#ifndef VMA_SYSTEM_ALIGNED_MALLOC
   #if defined(_WIN32)
       #define VMA_SYSTEM_ALIGNED_MALLOC(size, alignment)   (_aligned_malloc((size), (alignment)))
   #else
       #define VMA_SYSTEM_ALIGNED_MALLOC(size, alignment)   (aligned_alloc((alignment), (size) ))
   #endif
#endif

#ifndef VMA_SYSTEM_FREE
   #if defined(_WIN32)
       #define VMA_SYSTEM_FREE(ptr)   _aligned_free(ptr)
   #else
       #define VMA_SYSTEM_FREE(ptr)   free(ptr)
   #endif
#endif

#ifndef VMA_MIN
   #define VMA_MIN(v1, v2)    (std::min((v1), (v2)))
#endif

#ifndef VMA_MAX
   #define VMA_MAX(v1, v2)    (std::max((v1), (v2)))
#endif

#ifndef VMA_SWAP
   #define VMA_SWAP(v1, v2)   std::swap((v1), (v2))
#endif

#ifndef VMA_SORT
   #define VMA_SORT(beg, end, cmp)  std::sort(beg, end, cmp)
#endif

#ifndef VMA_DEBUG_LOG
   #define VMA_DEBUG_LOG(format, ...)
   /*
   #define VMA_DEBUG_LOG(format, ...) do { \
       printf(format, __VA_ARGS__); \
       printf("\n"); \
   } while(false)
   */
#endif

// Define this macro to 1 to enable functions: vmaBuildStatsString, vmaFreeStatsString.
#if VMA_STATS_STRING_ENABLED
   static inline void VmaUint32ToStr(char* outStr, size_t strLen, uint32_t num)
   {
       snprintf(outStr, strLen, "%u", static_cast<unsigned int>(num));
   }
   static inline void VmaUint64ToStr(char* outStr, size_t strLen, uint64_t num)
   {
       snprintf(outStr, strLen, "%llu", static_cast<unsigned long long>(num));
   }
   static inline void VmaPtrToStr(char* outStr, size_t strLen, const void* ptr)
   {
       snprintf(outStr, strLen, "%p", ptr);
   }
#endif

#ifndef VMA_MUTEX
   class VmaMutex
   {
   public:
       VmaMutex() { }
       ~VmaMutex() { }
       void Lock() { m_Mutex.lock(); }
       void Unlock() { m_Mutex.unlock(); }
   private:
       std::mutex m_Mutex;
   };
   #define VMA_MUTEX VmaMutex
#endif

/*
If providing your own implementation, you need to implement a subset of std::atomic:

- Constructor(uint32_t desired)
- uint32_t load() const
- void store(uint32_t desired)
- bool compare_exchange_weak(uint32_t& expected, uint32_t desired)
*/
#ifndef VMA_ATOMIC_UINT32
   #define VMA_ATOMIC_UINT32 std::atomic<uint32_t>
#endif

#ifndef VMA_BEST_FIT
   /**
   Main parameter for function assessing how good is a free suballocation for a new
   allocation request.

   - Set to 1 to use Best-Fit algorithm - prefer smaller blocks, as close to the
     size of requested allocations as possible.
   - Set to 0 to use Worst-Fit algorithm - prefer larger blocks, as large as
     possible.

   Experiments in special testing environment showed that Best-Fit algorithm is
   better.
   */
   #define VMA_BEST_FIT (1)
#endif

#ifndef VMA_DEBUG_ALWAYS_OWN_MEMORY
   /**
   Every object will have its own allocation.
   Define to 1 for debugging purposes only.
   */
   #define VMA_DEBUG_ALWAYS_OWN_MEMORY (0)
#endif

#ifndef VMA_DEBUG_ALIGNMENT
   /**
   Minimum alignment of all suballocations, in bytes.
   Set to more than 1 for debugging purposes only. Must be power of two.
   */
   #define VMA_DEBUG_ALIGNMENT (1)
#endif

#ifndef VMA_DEBUG_MARGIN
   /**
   Minimum margin between suballocations, in bytes.
   Set nonzero for debugging purposes only.
   */
   #define VMA_DEBUG_MARGIN (0)
#endif

#ifndef VMA_DEBUG_GLOBAL_MUTEX
   /**
   Set this to 1 for debugging purposes only, to enable single mutex protecting all
   entry calls to the library. Can be useful for debugging multithreading issues.
   */
   #define VMA_DEBUG_GLOBAL_MUTEX (0)
#endif

#ifndef VMA_DEBUG_MIN_BUFFER_IMAGE_GRANULARITY
   /**
   Minimum value for VkPhysicalDeviceLimits::bufferImageGranularity.
   Set to more than 1 for debugging purposes only. Must be power of two.
   */
   #define VMA_DEBUG_MIN_BUFFER_IMAGE_GRANULARITY (1)
#endif

#ifndef VMA_SMALL_HEAP_MAX_SIZE
   /// Maximum size of a memory heap in Vulkan to consider it "small".
   #define VMA_SMALL_HEAP_MAX_SIZE (512 * 1024 * 1024)
#endif

#ifndef VMA_DEFAULT_LARGE_HEAP_BLOCK_SIZE
   /// Default size of a block allocated as single VkDeviceMemory from a "large" heap.
   #define VMA_DEFAULT_LARGE_HEAP_BLOCK_SIZE (256 * 1024 * 1024)
#endif

#ifndef VMA_DEFAULT_SMALL_HEAP_BLOCK_SIZE
   /// Default size of a block allocated as single VkDeviceMemory from a "small" heap.
   #define VMA_DEFAULT_SMALL_HEAP_BLOCK_SIZE (64 * 1024 * 1024)
#endif

static const uint32_t VMA_FRAME_INDEX_LOST = UINT32_MAX;

/*******************************************************************************
END OF CONFIGURATION
*/

static VkAllocationCallbacks VmaEmptyAllocationCallbacks = {
    VMA_NULL, VMA_NULL, VMA_NULL, VMA_NULL, VMA_NULL, VMA_NULL };

// Returns number of bits set to 1 in (v).
static inline uint32_t CountBitsSet(uint32_t v)
{
	uint32_t c = v - ((v >> 1) & 0x55555555);
	c = ((c >>  2) & 0x33333333) + (c & 0x33333333);
	c = ((c >>  4) + c) & 0x0F0F0F0F;
	c = ((c >>  8) + c) & 0x00FF00FF;
	c = ((c >> 16) + c) & 0x0000FFFF;
	return c;
}

// Aligns given value up to nearest multiply of align value. For example: VmaAlignUp(11, 8) = 16.
// Use types like uint32_t, uint64_t as T.
template <typename T>
static inline T VmaAlignUp(T val, T align)
{
	return (val + align - 1) / align * align;
}

// Division with mathematical rounding to nearest number.
template <typename T>
inline T VmaRoundDiv(T x, T y)
{
	return (x + (y / (T)2)) / y;
}

#ifndef VMA_SORT

template<typename Iterator, typename Compare>
Iterator VmaQuickSortPartition(Iterator beg, Iterator end, Compare cmp)
{
    Iterator centerValue = end; --centerValue;
    Iterator insertIndex = beg;
    for(Iterator memTypeIndex = beg; memTypeIndex < centerValue; ++memTypeIndex)
    {
        if(cmp(*memTypeIndex, *centerValue))
        {
            if(insertIndex != memTypeIndex)
            {
                VMA_SWAP(*memTypeIndex, *insertIndex);
            }
            ++insertIndex;
        }
    }
    if(insertIndex != centerValue)
    {
        VMA_SWAP(*insertIndex, *centerValue);
    }
    return insertIndex;
}

template<typename Iterator, typename Compare>
void VmaQuickSort(Iterator beg, Iterator end, Compare cmp)
{
    if(beg < end)
    {
        Iterator it = VmaQuickSortPartition<Iterator, Compare>(beg, end, cmp);
        VmaQuickSort<Iterator, Compare>(beg, it, cmp);
        VmaQuickSort<Iterator, Compare>(it + 1, end, cmp);
    }
}

#define VMA_SORT(beg, end, cmp) VmaQuickSort(beg, end, cmp)

#endif // #ifndef VMA_SORT

/*
Returns true if two memory blocks occupy overlapping pages.
ResourceA must be in less memory offset than ResourceB.

Algorithm is based on "Vulkan 1.0.39 - A Specification (with all registered Vulkan extensions)"
chapter 11.6 "Resource Memory Association", paragraph "Buffer-Image Granularity".
*/
static inline bool VmaBlocksOnSamePage(
    VkDeviceSize resourceAOffset,
    VkDeviceSize resourceASize,
    VkDeviceSize resourceBOffset,
    VkDeviceSize pageSize)
{
    VMA_ASSERT(resourceAOffset + resourceASize <= resourceBOffset && resourceASize > 0 && pageSize > 0);
    VkDeviceSize resourceAEnd = resourceAOffset + resourceASize - 1;
    VkDeviceSize resourceAEndPage = resourceAEnd & ~(pageSize - 1);
    VkDeviceSize resourceBStart = resourceBOffset;
    VkDeviceSize resourceBStartPage = resourceBStart & ~(pageSize - 1);
    return resourceAEndPage == resourceBStartPage;
}

enum VmaSuballocationType
{
    VMA_SUBALLOCATION_TYPE_FREE = 0,
    VMA_SUBALLOCATION_TYPE_UNKNOWN = 1,
    VMA_SUBALLOCATION_TYPE_BUFFER = 2,
    VMA_SUBALLOCATION_TYPE_IMAGE_UNKNOWN = 3,
    VMA_SUBALLOCATION_TYPE_IMAGE_LINEAR = 4,
    VMA_SUBALLOCATION_TYPE_IMAGE_OPTIMAL = 5,
    VMA_SUBALLOCATION_TYPE_MAX_ENUM = 0x7FFFFFFF
};

/*
Returns true if given suballocation types could conflict and must respect
VkPhysicalDeviceLimits::bufferImageGranularity. They conflict if one is buffer
or linear image and another one is optimal image. If type is unknown, behave
conservatively.
*/
static inline bool VmaIsBufferImageGranularityConflict(
    VmaSuballocationType suballocType1,
    VmaSuballocationType suballocType2)
{
    if(suballocType1 > suballocType2)
    {
        VMA_SWAP(suballocType1, suballocType2);
    }
    
    switch(suballocType1)
    {
    case VMA_SUBALLOCATION_TYPE_FREE:
        return false;
    case VMA_SUBALLOCATION_TYPE_UNKNOWN:
        return true;
    case VMA_SUBALLOCATION_TYPE_BUFFER:
        return
            suballocType2 == VMA_SUBALLOCATION_TYPE_IMAGE_UNKNOWN ||
            suballocType2 == VMA_SUBALLOCATION_TYPE_IMAGE_OPTIMAL;
    case VMA_SUBALLOCATION_TYPE_IMAGE_UNKNOWN:
        return
            suballocType2 == VMA_SUBALLOCATION_TYPE_IMAGE_UNKNOWN ||
            suballocType2 == VMA_SUBALLOCATION_TYPE_IMAGE_LINEAR ||
            suballocType2 == VMA_SUBALLOCATION_TYPE_IMAGE_OPTIMAL;
    case VMA_SUBALLOCATION_TYPE_IMAGE_LINEAR:
        return
            suballocType2 == VMA_SUBALLOCATION_TYPE_IMAGE_OPTIMAL;
    case VMA_SUBALLOCATION_TYPE_IMAGE_OPTIMAL:
        return false;
    default:
        VMA_ASSERT(0);
        return true;
    }
}

// Helper RAII class to lock a mutex in constructor and unlock it in destructor (at the end of scope).
struct VmaMutexLock
{
public:
    VmaMutexLock(VMA_MUTEX& mutex, bool useMutex) :
        m_pMutex(useMutex ? &mutex : VMA_NULL)
    {
        if(m_pMutex)
        {
            m_pMutex->Lock();
        }
    }
    
    ~VmaMutexLock()
    {
        if(m_pMutex)
        {
            m_pMutex->Unlock();
        }
    }

private:
    VMA_MUTEX* m_pMutex;
};

#if VMA_DEBUG_GLOBAL_MUTEX
    static VMA_MUTEX gDebugGlobalMutex;
    #define VMA_DEBUG_GLOBAL_MUTEX_LOCK VmaMutexLock debugGlobalMutexLock(gDebugGlobalMutex, true);
#else
    #define VMA_DEBUG_GLOBAL_MUTEX_LOCK
#endif

// Minimum size of a free suballocation to register it in the free suballocation collection.
static const VkDeviceSize VMA_MIN_FREE_SUBALLOCATION_SIZE_TO_REGISTER = 16;

/*
Performs binary search and returns iterator to first element that is greater or
equal to (key), according to comparison (cmp).

Cmp should return true if first argument is less than second argument.

Returned value is the found element, if present in the collection or place where
new element with value (key) should be inserted.
*/
template <typename IterT, typename KeyT, typename CmpT>
static IterT VmaBinaryFindFirstNotLess(IterT beg, IterT end, const KeyT &key, CmpT cmp)
{
   size_t down = 0, up = (end - beg);
   while(down < up)
   {
      const size_t mid = (down + up) / 2;
      if(cmp(*(beg+mid), key))
      {
         down = mid + 1;
      }
      else
      {
         up = mid;
      }
   }
   return beg + down;
}

////////////////////////////////////////////////////////////////////////////////
// Memory allocation

static void* VmaMalloc(const VkAllocationCallbacks* pAllocationCallbacks, size_t size, size_t alignment)
{
    if((pAllocationCallbacks != VMA_NULL) &&
        (pAllocationCallbacks->pfnAllocation != VMA_NULL))
    {
        return (*pAllocationCallbacks->pfnAllocation)(
            pAllocationCallbacks->pUserData,
            size,
            alignment,
            VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
    }
    else
    {
        return VMA_SYSTEM_ALIGNED_MALLOC(size, alignment);
    }
}

static void VmaFree(const VkAllocationCallbacks* pAllocationCallbacks, void* ptr)
{
    if((pAllocationCallbacks != VMA_NULL) &&
        (pAllocationCallbacks->pfnFree != VMA_NULL))
    {
        (*pAllocationCallbacks->pfnFree)(pAllocationCallbacks->pUserData, ptr);
    }
    else
    {
        VMA_SYSTEM_FREE(ptr);
    }
}

template<typename T>
static T* VmaAllocate(const VkAllocationCallbacks* pAllocationCallbacks)
{
    return (T*)VmaMalloc(pAllocationCallbacks, sizeof(T), VMA_ALIGN_OF(T));
}

template<typename T>
static T* VmaAllocateArray(const VkAllocationCallbacks* pAllocationCallbacks, size_t count)
{
    return (T*)VmaMalloc(pAllocationCallbacks, sizeof(T) * count, VMA_ALIGN_OF(T));
}

#define vma_new(allocator, type)   new(VmaAllocate<type>(allocator))(type)

#define vma_new_array(allocator, type, count)   new(VmaAllocateArray<type>((allocator), (count)))(type)

template<typename T>
static void vma_delete(const VkAllocationCallbacks* pAllocationCallbacks, T* ptr)
{
    ptr->~T();
    VmaFree(pAllocationCallbacks, ptr);
}

template<typename T>
static void vma_delete_array(const VkAllocationCallbacks* pAllocationCallbacks, T* ptr, size_t count)
{
    if(ptr != VMA_NULL)
    {
        for(size_t i = count; i--; )
        {
            ptr[i].~T();
        }
        VmaFree(pAllocationCallbacks, ptr);
    }
}

// STL-compatible allocator.
template<typename T>
class VmaStlAllocator
{
public:
    const VkAllocationCallbacks* const m_pCallbacks;
    typedef T value_type;
    
    VmaStlAllocator(const VkAllocationCallbacks* pCallbacks) : m_pCallbacks(pCallbacks) { }
    template<typename U> VmaStlAllocator(const VmaStlAllocator<U>& src) : m_pCallbacks(src.m_pCallbacks) { }

    T* allocate(size_t n) { return VmaAllocateArray<T>(m_pCallbacks, n); }
    void deallocate(T* p, size_t n) { VmaFree(m_pCallbacks, p); }

    template<typename U>
    bool operator==(const VmaStlAllocator<U>& rhs) const
    {
        return m_pCallbacks == rhs.m_pCallbacks;
    }
    template<typename U>
    bool operator!=(const VmaStlAllocator<U>& rhs) const
    {
        return m_pCallbacks != rhs.m_pCallbacks;
    }

    VmaStlAllocator& operator=(const VmaStlAllocator& x) = delete;
};

#if VMA_USE_STL_VECTOR

#define VmaVector std::vector

template<typename T, typename allocatorT>
static void VmaVectorInsert(std::vector<T, allocatorT>& vec, size_t index, const T& item)
{
    vec.insert(vec.begin() + index, item);
}

template<typename T, typename allocatorT>
static void VmaVectorRemove(std::vector<T, allocatorT>& vec, size_t index)
{
    vec.erase(vec.begin() + index);
}

#else // #if VMA_USE_STL_VECTOR

/* Class with interface compatible with subset of std::vector.
T must be POD because constructors and destructors are not called and memcpy is
used for these objects. */
template<typename T, typename AllocatorT>
class VmaVector
{
public:
    typedef T value_type;

    VmaVector(const AllocatorT& allocator) :
        m_Allocator(allocator),
        m_pArray(VMA_NULL),
        m_Count(0),
        m_Capacity(0)
    {
    }

    VmaVector(size_t count, const AllocatorT& allocator) :
        m_Allocator(allocator),
        m_pArray(count ? (T*)VmaAllocateArray<T>(allocator.m_pCallbacks, count) : VMA_NULL),
        m_Count(count),
        m_Capacity(count)
    {
    }
    
    VmaVector(const VmaVector<T, AllocatorT>& src) :
        m_Allocator(src.m_Allocator),
        m_pArray(src.m_Count ? (T*)VmaAllocateArray<T>(src.m_Allocator.m_pCallbacks, src.m_Count) : VMA_NULL),
        m_Count(src.m_Count),
        m_Capacity(src.m_Count)
    {
        if(m_Count != 0)
        {
            memcpy(m_pArray, src.m_pArray, m_Count * sizeof(T));
        }
    }
    
    ~VmaVector()
    {
        VmaFree(m_Allocator.m_pCallbacks, m_pArray);
    }

    VmaVector& operator=(const VmaVector<T, AllocatorT>& rhs)
    {
        if(&rhs != this)
        {
            resize(rhs.m_Count);
            if(m_Count != 0)
            {
                memcpy(m_pArray, rhs.m_pArray, m_Count * sizeof(T));
            }
        }
        return *this;
    }
    
    bool empty() const { return m_Count == 0; }
    size_t size() const { return m_Count; }
    T* data() { return m_pArray; }
    const T* data() const { return m_pArray; }
    
    T& operator[](size_t index)
    {
        VMA_HEAVY_ASSERT(index < m_Count);
        return m_pArray[index];
    }
    const T& operator[](size_t index) const
    {
        VMA_HEAVY_ASSERT(index < m_Count);
        return m_pArray[index];
    }

    T& front()
    {
        VMA_HEAVY_ASSERT(m_Count > 0);
        return m_pArray[0];
    }
    const T& front() const
    {
        VMA_HEAVY_ASSERT(m_Count > 0);
        return m_pArray[0];
    }
    T& back()
    {
        VMA_HEAVY_ASSERT(m_Count > 0);
        return m_pArray[m_Count - 1];
    }
    const T& back() const
    {
        VMA_HEAVY_ASSERT(m_Count > 0);
        return m_pArray[m_Count - 1];
    }

    void reserve(size_t newCapacity, bool freeMemory = false)
    {
        newCapacity = VMA_MAX(newCapacity, m_Count);
        
        if((newCapacity < m_Capacity) && !freeMemory)
        {
            newCapacity = m_Capacity;
        }
        
        if(newCapacity != m_Capacity)
        {
            T* const newArray = newCapacity ? VmaAllocateArray<T>(m_Allocator, newCapacity) : VMA_NULL;
            if(m_Count != 0)
            {
                memcpy(newArray, m_pArray, m_Count * sizeof(T));
            }
            VmaFree(m_Allocator.m_pCallbacks, m_pArray);
            m_Capacity = newCapacity;
            m_pArray = newArray;
        }
    }

    void resize(size_t newCount, bool freeMemory = false)
    {
        size_t newCapacity = m_Capacity;
        if(newCount > m_Capacity)
        {
            newCapacity = VMA_MAX(newCount, VMA_MAX(m_Capacity * 3 / 2, (size_t)8));
        }
        else if(freeMemory)
        {
            newCapacity = newCount;
        }

        if(newCapacity != m_Capacity)
        {
            T* const newArray = newCapacity ? VmaAllocateArray<T>(m_Allocator.m_pCallbacks, newCapacity) : VMA_NULL;
            const size_t elementsToCopy = VMA_MIN(m_Count, newCount);
            if(elementsToCopy != 0)
            {
                memcpy(newArray, m_pArray, elementsToCopy * sizeof(T));
            }
            VmaFree(m_Allocator.m_pCallbacks, m_pArray);
            m_Capacity = newCapacity;
            m_pArray = newArray;
        }

        m_Count = newCount;
    }

    void clear(bool freeMemory = false)
    {
        resize(0, freeMemory);
    }

    void insert(size_t index, const T& src)
    {
        VMA_HEAVY_ASSERT(index <= m_Count);
        const size_t oldCount = size();
        resize(oldCount + 1);
        if(index < oldCount)
        {
            memmove(m_pArray + (index + 1), m_pArray + index, (oldCount - index) * sizeof(T));
        }
        m_pArray[index] = src;
    }

    void remove(size_t index)
    {
        VMA_HEAVY_ASSERT(index < m_Count);
        const size_t oldCount = size();
        if(index < oldCount - 1)
        {
            memmove(m_pArray + index, m_pArray + (index + 1), (oldCount - index - 1) * sizeof(T));
        }
        resize(oldCount - 1);
    }

    void push_back(const T& src)
    {
        const size_t newIndex = size();
        resize(newIndex + 1);
        m_pArray[newIndex] = src;
    }

    void pop_back()
    {
        VMA_HEAVY_ASSERT(m_Count > 0);
        resize(size() - 1);
    }

    void push_front(const T& src)
    {
        insert(0, src);
    }

    void pop_front()
    {
        VMA_HEAVY_ASSERT(m_Count > 0);
        remove(0);
    }

    typedef T* iterator;

    iterator begin() { return m_pArray; }
    iterator end() { return m_pArray + m_Count; }

private:
    AllocatorT m_Allocator;
    T* m_pArray;
    size_t m_Count;
    size_t m_Capacity;
};

template<typename T, typename allocatorT>
static void VmaVectorInsert(VmaVector<T, allocatorT>& vec, size_t index, const T& item)
{
    vec.insert(index, item);
}

template<typename T, typename allocatorT>
static void VmaVectorRemove(VmaVector<T, allocatorT>& vec, size_t index)
{
    vec.remove(index);
}

#endif // #if VMA_USE_STL_VECTOR

template<typename CmpLess, typename VectorT>
size_t VmaVectorInsertSorted(VectorT& vector, const typename VectorT::value_type& value)
{
    const size_t indexToInsert = VmaBinaryFindFirstNotLess(
        vector.data(),
        vector.data() + vector.size(),
        value,
        CmpLess()) - vector.data();
    VmaVectorInsert(vector, indexToInsert, value);
    return indexToInsert;
}

template<typename CmpLess, typename VectorT>
bool VmaVectorRemoveSorted(VectorT& vector, const typename VectorT::value_type& value)
{
    CmpLess comparator;
    typename VectorT::iterator it = VmaBinaryFindFirstNotLess(
        vector.data(),
        vector.data() + vector.size(),
        value,
        comparator);
    if((it != vector.end()) && !comparator(*it, value) && !comparator(value, *it))
    {
        size_t indexToRemove = it - vector.begin();
        VmaVectorRemove(vector, indexToRemove);
        return true;
    }
    return false;
}

template<typename CmpLess, typename VectorT>
size_t VmaVectorFindSorted(const VectorT& vector, const typename VectorT::value_type& value)
{
    CmpLess comparator;
    typename VectorT::iterator it = VmaBinaryFindFirstNotLess(
        vector.data(),
        vector.data() + vector.size(),
        value,
        comparator);
    if(it != vector.size() && !comparator(*it, value) && !comparator(value, *it))
    {
        return it - vector.begin();
    }
    else
    {
        return vector.size();
    }
}

////////////////////////////////////////////////////////////////////////////////
// class VmaPoolAllocator

/*
Allocator for objects of type T using a list of arrays (pools) to speed up
allocation. Number of elements that can be allocated is not bounded because
allocator can create multiple blocks.
*/
template<typename T>
class VmaPoolAllocator
{
public:
    VmaPoolAllocator(const VkAllocationCallbacks* pAllocationCallbacks, size_t itemsPerBlock);
    ~VmaPoolAllocator();
    void Clear();
    T* Alloc();
    void Free(T* ptr);

private:
    union Item
    {
        uint32_t NextFreeIndex;
        T Value;
    };

    struct ItemBlock
    {
        Item* pItems;
        uint32_t FirstFreeIndex;
    };
    
    const VkAllocationCallbacks* m_pAllocationCallbacks;
    size_t m_ItemsPerBlock;
    VmaVector< ItemBlock, VmaStlAllocator<ItemBlock> > m_ItemBlocks;

    ItemBlock& CreateNewBlock();
};

template<typename T>
VmaPoolAllocator<T>::VmaPoolAllocator(const VkAllocationCallbacks* pAllocationCallbacks, size_t itemsPerBlock) :
    m_pAllocationCallbacks(pAllocationCallbacks),
    m_ItemsPerBlock(itemsPerBlock),
    m_ItemBlocks(VmaStlAllocator<ItemBlock>(pAllocationCallbacks))
{
    VMA_ASSERT(itemsPerBlock > 0);
}

template<typename T>
VmaPoolAllocator<T>::~VmaPoolAllocator()
{
    Clear();
}

template<typename T>
void VmaPoolAllocator<T>::Clear()
{
    for(size_t i = m_ItemBlocks.size(); i--; )
        vma_delete_array(m_pAllocationCallbacks, m_ItemBlocks[i].pItems, m_ItemsPerBlock);
    m_ItemBlocks.clear();
}

template<typename T>
T* VmaPoolAllocator<T>::Alloc()
{
    for(size_t i = m_ItemBlocks.size(); i--; )
    {
        ItemBlock& block = m_ItemBlocks[i];
        // This block has some free items: Use first one.
        if(block.FirstFreeIndex != UINT32_MAX)
        {
            Item* const pItem = &block.pItems[block.FirstFreeIndex];
            block.FirstFreeIndex = pItem->NextFreeIndex;
            return &pItem->Value;
        }
    }

    // No block has free item: Create new one and use it.
    ItemBlock& newBlock = CreateNewBlock();
    Item* const pItem = &newBlock.pItems[0];
    newBlock.FirstFreeIndex = pItem->NextFreeIndex;
    return &pItem->Value;
}

template<typename T>
void VmaPoolAllocator<T>::Free(T* ptr)
{
    // Search all memory blocks to find ptr.
    for(size_t i = 0; i < m_ItemBlocks.size(); ++i)
    {
        ItemBlock& block = m_ItemBlocks[i];
        
        // Casting to union.
        Item* pItemPtr;
        memcpy(&pItemPtr, &ptr, sizeof(pItemPtr));
        
        // Check if pItemPtr is in address range of this block.
        if((pItemPtr >= block.pItems) && (pItemPtr < block.pItems + m_ItemsPerBlock))
        {
            const uint32_t index = static_cast<uint32_t>(pItemPtr - block.pItems);
            pItemPtr->NextFreeIndex = block.FirstFreeIndex;
            block.FirstFreeIndex = index;
            return;
        }
    }
    VMA_ASSERT(0 && "Pointer doesn't belong to this memory pool.");
}

template<typename T>
typename VmaPoolAllocator<T>::ItemBlock& VmaPoolAllocator<T>::CreateNewBlock()
{
    ItemBlock newBlock = {
        vma_new_array(m_pAllocationCallbacks, Item, m_ItemsPerBlock), 0 };

    m_ItemBlocks.push_back(newBlock);

    // Setup singly-linked list of all free items in this block.
    for(uint32_t i = 0; i < m_ItemsPerBlock - 1; ++i)
        newBlock.pItems[i].NextFreeIndex = i + 1;
    newBlock.pItems[m_ItemsPerBlock - 1].NextFreeIndex = UINT32_MAX;
    return m_ItemBlocks.back();
}

////////////////////////////////////////////////////////////////////////////////
// class VmaRawList, VmaList

#if VMA_USE_STL_LIST

#define VmaList std::list

#else // #if VMA_USE_STL_LIST

template<typename T>
struct VmaListItem
{
    VmaListItem* pPrev;
    VmaListItem* pNext;
    T Value;
};

// Doubly linked list.
template<typename T>
class VmaRawList
{
public:
    typedef VmaListItem<T> ItemType;

    VmaRawList(const VkAllocationCallbacks* pAllocationCallbacks);
    ~VmaRawList();
    void Clear();

    size_t GetCount() const { return m_Count; }
    bool IsEmpty() const { return m_Count == 0; }

    ItemType* Front() { return m_pFront; }
    const ItemType* Front() const { return m_pFront; }
    ItemType* Back() { return m_pBack; }
    const ItemType* Back() const { return m_pBack; }

    ItemType* PushBack();
    ItemType* PushFront();
    ItemType* PushBack(const T& value);
    ItemType* PushFront(const T& value);
    void PopBack();
    void PopFront();
    
    // Item can be null - it means PushBack.
    ItemType* InsertBefore(ItemType* pItem);
    // Item can be null - it means PushFront.
    ItemType* InsertAfter(ItemType* pItem);

    ItemType* InsertBefore(ItemType* pItem, const T& value);
    ItemType* InsertAfter(ItemType* pItem, const T& value);

    void Remove(ItemType* pItem);

private:
    const VkAllocationCallbacks* const m_pAllocationCallbacks;
    VmaPoolAllocator<ItemType> m_ItemAllocator;
    ItemType* m_pFront;
    ItemType* m_pBack;
    size_t m_Count;

    // Declared not defined, to block copy constructor and assignment operator.
    VmaRawList(const VmaRawList<T>& src);
    VmaRawList<T>& operator=(const VmaRawList<T>& rhs);
};

template<typename T>
VmaRawList<T>::VmaRawList(const VkAllocationCallbacks* pAllocationCallbacks) :
    m_pAllocationCallbacks(pAllocationCallbacks),
    m_ItemAllocator(pAllocationCallbacks, 128),
    m_pFront(VMA_NULL),
    m_pBack(VMA_NULL),
    m_Count(0)
{
}

template<typename T>
VmaRawList<T>::~VmaRawList()
{
    // Intentionally not calling Clear, because that would be unnecessary
    // computations to return all items to m_ItemAllocator as free.
}

template<typename T>
void VmaRawList<T>::Clear()
{
    if(IsEmpty() == false)
    {
        ItemType* pItem = m_pBack;
        while(pItem != VMA_NULL)
        {
            ItemType* const pPrevItem = pItem->pPrev;
            m_ItemAllocator.Free(pItem);
            pItem = pPrevItem;
        }
        m_pFront = VMA_NULL;
        m_pBack = VMA_NULL;
        m_Count = 0;
    }
}

template<typename T>
VmaListItem<T>* VmaRawList<T>::PushBack()
{
    ItemType* const pNewItem = m_ItemAllocator.Alloc();
    pNewItem->pNext = VMA_NULL;
    if(IsEmpty())
    {
        pNewItem->pPrev = VMA_NULL;
        m_pFront = pNewItem;
        m_pBack = pNewItem;
        m_Count = 1;
    }
    else
    {
        pNewItem->pPrev = m_pBack;
        m_pBack->pNext = pNewItem;
        m_pBack = pNewItem;
        ++m_Count;
    }
    return pNewItem;
}

template<typename T>
VmaListItem<T>* VmaRawList<T>::PushFront()
{
    ItemType* const pNewItem = m_ItemAllocator.Alloc();
    pNewItem->pPrev = VMA_NULL;
    if(IsEmpty())
    {
        pNewItem->pNext = VMA_NULL;
        m_pFront = pNewItem;
        m_pBack = pNewItem;
        m_Count = 1;
    }
    else
    {
        pNewItem->pNext = m_pFront;
        m_pFront->pPrev = pNewItem;
        m_pFront = pNewItem;
        ++m_Count;
    }
    return pNewItem;
}

template<typename T>
VmaListItem<T>* VmaRawList<T>::PushBack(const T& value)
{
    ItemType* const pNewItem = PushBack();
    pNewItem->Value = value;
    return pNewItem;
}

template<typename T>
VmaListItem<T>* VmaRawList<T>::PushFront(const T& value)
{
    ItemType* const pNewItem = PushFront();
    pNewItem->Value = value;
    return pNewItem;
}

template<typename T>
void VmaRawList<T>::PopBack()
{
    VMA_HEAVY_ASSERT(m_Count > 0);
    ItemType* const pBackItem = m_pBack;
    ItemType* const pPrevItem = pBackItem->pPrev;
    if(pPrevItem != VMA_NULL)
    {
        pPrevItem->pNext = VMA_NULL;
    }
    m_pBack = pPrevItem;
    m_ItemAllocator.Free(pBackItem);
    --m_Count;
}

template<typename T>
void VmaRawList<T>::PopFront()
{
    VMA_HEAVY_ASSERT(m_Count > 0);
    ItemType* const pFrontItem = m_pFront;
    ItemType* const pNextItem = pFrontItem->pNext;
    if(pNextItem != VMA_NULL)
    {
        pNextItem->pPrev = VMA_NULL;
    }
    m_pFront = pNextItem;
    m_ItemAllocator.Free(pFrontItem);
    --m_Count;
}

template<typename T>
void VmaRawList<T>::Remove(ItemType* pItem)
{
    VMA_HEAVY_ASSERT(pItem != VMA_NULL);
    VMA_HEAVY_ASSERT(m_Count > 0);

    if(pItem->pPrev != VMA_NULL)
    {
        pItem->pPrev->pNext = pItem->pNext;
    }
    else
    {
        VMA_HEAVY_ASSERT(m_pFront == pItem);
        m_pFront = pItem->pNext;
    }

    if(pItem->pNext != VMA_NULL)
    {
        pItem->pNext->pPrev = pItem->pPrev;
    }
    else
    {
        VMA_HEAVY_ASSERT(m_pBack == pItem);
        m_pBack = pItem->pPrev;
    }

    m_ItemAllocator.Free(pItem);
    --m_Count;
}

template<typename T>
VmaListItem<T>* VmaRawList<T>::InsertBefore(ItemType* pItem)
{
    if(pItem != VMA_NULL)
    {
        ItemType* const prevItem = pItem->pPrev;
        ItemType* const newItem = m_ItemAllocator.Alloc();
        newItem->pPrev = prevItem;
        newItem->pNext = pItem;
        pItem->pPrev = newItem;
        if(prevItem != VMA_NULL)
        {
            prevItem->pNext = newItem;
        }
        else
        {
            VMA_HEAVY_ASSERT(m_pFront == pItem);
            m_pFront = newItem;
        }
        ++m_Count;
        return newItem;
    }
    else
        return PushBack();
}

template<typename T>
VmaListItem<T>* VmaRawList<T>::InsertAfter(ItemType* pItem)
{
    if(pItem != VMA_NULL)
    {
        ItemType* const nextItem = pItem->pNext;
        ItemType* const newItem = m_ItemAllocator.Alloc();
        newItem->pNext = nextItem;
        newItem->pPrev = pItem;
        pItem->pNext = newItem;
        if(nextItem != VMA_NULL)
        {
            nextItem->pPrev = newItem;
        }
        else
        {
            VMA_HEAVY_ASSERT(m_pBack == pItem);
            m_pBack = newItem;
        }
        ++m_Count;
        return newItem;
    }
    else
        return PushFront();
}

template<typename T>
VmaListItem<T>* VmaRawList<T>::InsertBefore(ItemType* pItem, const T& value)
{
    ItemType* const newItem = InsertBefore(pItem);
    newItem->Value = value;
    return newItem;
}

template<typename T>
VmaListItem<T>* VmaRawList<T>::InsertAfter(ItemType* pItem, const T& value)
{
    ItemType* const newItem = InsertAfter(pItem);
    newItem->Value = value;
    return newItem;
}

template<typename T, typename AllocatorT>
class VmaList
{
public:
    class iterator
    {
    public:
        iterator() :
            m_pList(VMA_NULL),
            m_pItem(VMA_NULL)
        {
        }

        T& operator*() const
        {
            VMA_HEAVY_ASSERT(m_pItem != VMA_NULL);
            return m_pItem->Value;
        }
        T* operator->() const
        {
            VMA_HEAVY_ASSERT(m_pItem != VMA_NULL);
            return &m_pItem->Value;
        }

        iterator& operator++()
        {
            VMA_HEAVY_ASSERT(m_pItem != VMA_NULL);
            m_pItem = m_pItem->pNext;
            return *this;
        }
        iterator& operator--()
        {
            if(m_pItem != VMA_NULL)
            {
                m_pItem = m_pItem->pPrev;
            }
            else
            {
                VMA_HEAVY_ASSERT(!m_pList.IsEmpty());
                m_pItem = m_pList->Back();
            }
            return *this;
        }

        iterator operator++(int)
        {
            iterator result = *this;
            ++*this;
            return result;
        }
        iterator operator--(int)
        {
            iterator result = *this;
            --*this;
            return result;
        }

        bool operator==(const iterator& rhs) const
        {
            VMA_HEAVY_ASSERT(m_pList == rhs.m_pList);
            return m_pItem == rhs.m_pItem;
        }
        bool operator!=(const iterator& rhs) const
        {
            VMA_HEAVY_ASSERT(m_pList == rhs.m_pList);
            return m_pItem != rhs.m_pItem;
        }
        
    private:
        VmaRawList<T>* m_pList;
        VmaListItem<T>* m_pItem;

        iterator(VmaRawList<T>* pList, VmaListItem<T>* pItem) :
            m_pList(pList),
            m_pItem(pItem)
        {
        }

        friend class VmaList<T, AllocatorT>;
    };

    class const_iterator
    {
    public:
        const_iterator() :
            m_pList(VMA_NULL),
            m_pItem(VMA_NULL)
        {
        }

        const_iterator(const iterator& src) :
            m_pList(src.m_pList),
            m_pItem(src.m_pItem)
        {
        }
        
        const T& operator*() const
        {
            VMA_HEAVY_ASSERT(m_pItem != VMA_NULL);
            return m_pItem->Value;
        }
        const T* operator->() const
        {
            VMA_HEAVY_ASSERT(m_pItem != VMA_NULL);
            return &m_pItem->Value;
        }

        const_iterator& operator++()
        {
            VMA_HEAVY_ASSERT(m_pItem != VMA_NULL);
            m_pItem = m_pItem->pNext;
            return *this;
        }
        const_iterator& operator--()
        {
            if(m_pItem != VMA_NULL)
            {
                m_pItem = m_pItem->pPrev;
            }
            else
            {
                VMA_HEAVY_ASSERT(!m_pList->IsEmpty());
                m_pItem = m_pList->Back();
            }
            return *this;
        }

        const_iterator operator++(int)
        {
            const_iterator result = *this;
            ++*this;
            return result;
        }
        const_iterator operator--(int)
        {
            const_iterator result = *this;
            --*this;
            return result;
        }

        bool operator==(const const_iterator& rhs) const
        {
            VMA_HEAVY_ASSERT(m_pList == rhs.m_pList);
            return m_pItem == rhs.m_pItem;
        }
        bool operator!=(const const_iterator& rhs) const
        {
            VMA_HEAVY_ASSERT(m_pList == rhs.m_pList);
            return m_pItem != rhs.m_pItem;
        }
        
    private:
        const_iterator(const VmaRawList<T>* pList, const VmaListItem<T>* pItem) :
            m_pList(pList),
            m_pItem(pItem)
        {
        }

        const VmaRawList<T>* m_pList;
        const VmaListItem<T>* m_pItem;

        friend class VmaList<T, AllocatorT>;
    };

    VmaList(const AllocatorT& allocator) : m_RawList(allocator.m_pCallbacks) { }

    bool empty() const { return m_RawList.IsEmpty(); }
    size_t size() const { return m_RawList.GetCount(); }

    iterator begin() { return iterator(&m_RawList, m_RawList.Front()); }
    iterator end() { return iterator(&m_RawList, VMA_NULL); }

    const_iterator cbegin() const { return const_iterator(&m_RawList, m_RawList.Front()); }
    const_iterator cend() const { return const_iterator(&m_RawList, VMA_NULL); }

    void clear() { m_RawList.Clear(); }
    void push_back(const T& value) { m_RawList.PushBack(value); }
    void erase(iterator it) { m_RawList.Remove(it.m_pItem); }
    iterator insert(iterator it, const T& value) { return iterator(&m_RawList, m_RawList.InsertBefore(it.m_pItem, value)); }

private:
    VmaRawList<T> m_RawList;
};

#endif // #if VMA_USE_STL_LIST

////////////////////////////////////////////////////////////////////////////////
// class VmaMap

// Unused in this version.
#if 0

#if VMA_USE_STL_UNORDERED_MAP

#define VmaPair std::pair

#define VMA_MAP_TYPE(KeyT, ValueT) \
    std::unordered_map< KeyT, ValueT, std::hash<KeyT>, std::equal_to<KeyT>, VmaStlAllocator< std::pair<KeyT, ValueT> > >

#else // #if VMA_USE_STL_UNORDERED_MAP

template<typename T1, typename T2>
struct VmaPair
{
    T1 first;
    T2 second;

    VmaPair() : first(), second() { }
    VmaPair(const T1& firstSrc, const T2& secondSrc) : first(firstSrc), second(secondSrc) { }
};

/* Class compatible with subset of interface of std::unordered_map.
KeyT, ValueT must be POD because they will be stored in VmaVector.
*/
template<typename KeyT, typename ValueT>
class VmaMap
{
public:
    typedef VmaPair<KeyT, ValueT> PairType;
    typedef PairType* iterator;

    VmaMap(const VmaStlAllocator<PairType>& allocator) : m_Vector(allocator) { }

    iterator begin() { return m_Vector.begin(); }
    iterator end() { return m_Vector.end(); }

    void insert(const PairType& pair);
    iterator find(const KeyT& key);
    void erase(iterator it);
    
private:
    VmaVector< PairType, VmaStlAllocator<PairType> > m_Vector;
};

#define VMA_MAP_TYPE(KeyT, ValueT) VmaMap<KeyT, ValueT>

template<typename FirstT, typename SecondT>
struct VmaPairFirstLess
{
    bool operator()(const VmaPair<FirstT, SecondT>& lhs, const VmaPair<FirstT, SecondT>& rhs) const
    {
        return lhs.first < rhs.first;
    }
    bool operator()(const VmaPair<FirstT, SecondT>& lhs, const FirstT& rhsFirst) const
    {
        return lhs.first < rhsFirst;
    }
};

template<typename KeyT, typename ValueT>
void VmaMap<KeyT, ValueT>::insert(const PairType& pair)
{
    const size_t indexToInsert = VmaBinaryFindFirstNotLess(
        m_Vector.data(),
        m_Vector.data() + m_Vector.size(),
        pair,
        VmaPairFirstLess<KeyT, ValueT>()) - m_Vector.data();
    VmaVectorInsert(m_Vector, indexToInsert, pair);
}

template<typename KeyT, typename ValueT>
VmaPair<KeyT, ValueT>* VmaMap<KeyT, ValueT>::find(const KeyT& key)
{
    PairType* it = VmaBinaryFindFirstNotLess(
        m_Vector.data(),
        m_Vector.data() + m_Vector.size(),
        key,
        VmaPairFirstLess<KeyT, ValueT>());
    if((it != m_Vector.end()) && (it->first == key))
    {
        return it;
    }
    else
    {
        return m_Vector.end();
    }
}

template<typename KeyT, typename ValueT>
void VmaMap<KeyT, ValueT>::erase(iterator it)
{
    VmaVectorRemove(m_Vector, it - m_Vector.begin());
}

#endif // #if VMA_USE_STL_UNORDERED_MAP

#endif // #if 0

////////////////////////////////////////////////////////////////////////////////

class VmaDeviceMemoryBlock;

enum VMA_BLOCK_VECTOR_TYPE
{
    VMA_BLOCK_VECTOR_TYPE_UNMAPPED,
    VMA_BLOCK_VECTOR_TYPE_MAPPED,
    VMA_BLOCK_VECTOR_TYPE_COUNT
};

static VMA_BLOCK_VECTOR_TYPE VmaAllocationCreateFlagsToBlockVectorType(VmaAllocationCreateFlags flags)
{
    return (flags & VMA_ALLOCATION_CREATE_PERSISTENT_MAP_BIT) != 0 ?
        VMA_BLOCK_VECTOR_TYPE_MAPPED :
        VMA_BLOCK_VECTOR_TYPE_UNMAPPED;
}

struct VmaAllocation_T
{
public:
    enum ALLOCATION_TYPE
    {
        ALLOCATION_TYPE_NONE,
        ALLOCATION_TYPE_BLOCK,
        ALLOCATION_TYPE_OWN,
    };

    VmaAllocation_T(uint32_t currentFrameIndex) :
        m_Alignment(1),
        m_Size(0),
        m_pUserData(VMA_NULL),
        m_Type(ALLOCATION_TYPE_NONE),
        m_SuballocationType(VMA_SUBALLOCATION_TYPE_UNKNOWN),
        m_LastUseFrameIndex(currentFrameIndex)
    {
    }

    void InitBlockAllocation(
        VmaPool hPool,
        VmaDeviceMemoryBlock* block,
        VkDeviceSize offset,
        VkDeviceSize alignment,
        VkDeviceSize size,
        VmaSuballocationType suballocationType,
        void* pUserData,
        bool canBecomeLost)
    {
        VMA_ASSERT(m_Type == ALLOCATION_TYPE_NONE);
        VMA_ASSERT(block != VMA_NULL);
        m_Type = ALLOCATION_TYPE_BLOCK;
        m_Alignment = alignment;
        m_Size = size;
        m_pUserData = pUserData;
        m_SuballocationType = suballocationType;
        m_BlockAllocation.m_hPool = hPool;
        m_BlockAllocation.m_Block = block;
        m_BlockAllocation.m_Offset = offset;
        m_BlockAllocation.m_CanBecomeLost = canBecomeLost;
    }

    void InitLost()
    {
        VMA_ASSERT(m_Type == ALLOCATION_TYPE_NONE);
        VMA_ASSERT(m_LastUseFrameIndex.load() == VMA_FRAME_INDEX_LOST);
        m_Type = ALLOCATION_TYPE_BLOCK;
        m_BlockAllocation.m_hPool = VK_NULL_HANDLE;
        m_BlockAllocation.m_Block = VMA_NULL;
        m_BlockAllocation.m_Offset = 0;
        m_BlockAllocation.m_CanBecomeLost = true;
    }

    void ChangeBlockAllocation(
        VmaDeviceMemoryBlock* block,
        VkDeviceSize offset)
    {
        VMA_ASSERT(block != VMA_NULL);
        VMA_ASSERT(m_Type == ALLOCATION_TYPE_BLOCK);
        m_BlockAllocation.m_Block = block;
        m_BlockAllocation.m_Offset = offset;
    }

    void InitOwnAllocation(
        uint32_t memoryTypeIndex,
        VkDeviceMemory hMemory,
        VmaSuballocationType suballocationType,
        bool persistentMap,
        void* pMappedData,
        VkDeviceSize size,
        void* pUserData)
    {
        VMA_ASSERT(m_Type == ALLOCATION_TYPE_NONE);
        VMA_ASSERT(hMemory != VK_NULL_HANDLE);
        m_Type = ALLOCATION_TYPE_OWN;
        m_Alignment = 0;
        m_Size = size;
        m_pUserData = pUserData;
        m_SuballocationType = suballocationType;
        m_OwnAllocation.m_MemoryTypeIndex = memoryTypeIndex;
        m_OwnAllocation.m_hMemory = hMemory;
        m_OwnAllocation.m_PersistentMap = persistentMap;
        m_OwnAllocation.m_pMappedData = pMappedData;
    }

    ALLOCATION_TYPE GetType() const { return m_Type; }
    VkDeviceSize GetAlignment() const { return m_Alignment; }
    VkDeviceSize GetSize() const { return m_Size; }
    void* GetUserData() const { return m_pUserData; }
    void SetUserData(void* pUserData) { m_pUserData = pUserData; }
    VmaSuballocationType GetSuballocationType() const { return m_SuballocationType; }

    VmaDeviceMemoryBlock* GetBlock() const
    {
        VMA_ASSERT(m_Type == ALLOCATION_TYPE_BLOCK);
        return m_BlockAllocation.m_Block;
    }
    VkDeviceSize GetOffset() const;
    VkDeviceMemory GetMemory() const;
    uint32_t GetMemoryTypeIndex() const;
    VMA_BLOCK_VECTOR_TYPE GetBlockVectorType() const;
    void* GetMappedData() const;
    bool CanBecomeLost() const;
    VmaPool GetPool() const;

    VkResult OwnAllocMapPersistentlyMappedMemory(VkDevice hDevice)
    {
        VMA_ASSERT(m_Type == ALLOCATION_TYPE_OWN);
        if(m_OwnAllocation.m_PersistentMap)
        {
            return vkMapMemory(hDevice, m_OwnAllocation.m_hMemory, 0, VK_WHOLE_SIZE, 0, &m_OwnAllocation.m_pMappedData);
        }
        return VK_SUCCESS;
    }
    void OwnAllocUnmapPersistentlyMappedMemory(VkDevice hDevice)
    {
        VMA_ASSERT(m_Type == ALLOCATION_TYPE_OWN);
        if(m_OwnAllocation.m_pMappedData)
        {
            VMA_ASSERT(m_OwnAllocation.m_PersistentMap);
            vkUnmapMemory(hDevice, m_OwnAllocation.m_hMemory);
            m_OwnAllocation.m_pMappedData = VMA_NULL;
        }
    }
    
    uint32_t GetLastUseFrameIndex() const
    {
        return m_LastUseFrameIndex.load();
    }
    bool CompareExchangeLastUseFrameIndex(uint32_t& expected, uint32_t desired)
    {
        return m_LastUseFrameIndex.compare_exchange_weak(expected, desired);
    }
    /*
    - If hAllocation.LastUseFrameIndex + frameInUseCount < allocator.CurrentFrameIndex,
      makes it lost by setting LastUseFrameIndex = VMA_FRAME_INDEX_LOST and returns true.
    - Else, returns false.
    
    If hAllocation is already lost, assert - you should not call it then.
    If hAllocation was not created with CAN_BECOME_LOST_BIT, assert.
    */
    bool MakeLost(uint32_t currentFrameIndex, uint32_t frameInUseCount);

    void OwnAllocCalcStatsInfo(VmaStatInfo& outInfo)
    {
        VMA_ASSERT(m_Type == ALLOCATION_TYPE_OWN);
        outInfo.BlockCount = 1;
        outInfo.AllocationCount = 1;
        outInfo.UnusedRangeCount = 0;
        outInfo.UsedBytes = m_Size;
        outInfo.UnusedBytes = 0;
        outInfo.AllocationSizeMin = outInfo.AllocationSizeMax = m_Size;
        outInfo.UnusedRangeSizeMin = UINT64_MAX;
        outInfo.UnusedRangeSizeMax = 0;
    }

private:
    VkDeviceSize m_Alignment;
    VkDeviceSize m_Size;
    void* m_pUserData;
    ALLOCATION_TYPE m_Type;
    VmaSuballocationType m_SuballocationType;
    VMA_ATOMIC_UINT32 m_LastUseFrameIndex;

    // Allocation out of VmaDeviceMemoryBlock.
    struct BlockAllocation
    {
        VmaPool m_hPool; // Null if belongs to general memory.
        VmaDeviceMemoryBlock* m_Block;
        VkDeviceSize m_Offset;
        bool m_CanBecomeLost;
    };

    // Allocation for an object that has its own private VkDeviceMemory.
    struct OwnAllocation
    {
        uint32_t m_MemoryTypeIndex;
        VkDeviceMemory m_hMemory;
        bool m_PersistentMap;
        void* m_pMappedData;
    };

    union
    {
        // Allocation out of VmaDeviceMemoryBlock.
        BlockAllocation m_BlockAllocation;
        // Allocation for an object that has its own private VkDeviceMemory.
        OwnAllocation m_OwnAllocation;
    };
};

/*
Represents a region of VmaDeviceMemoryBlock that is either assigned and returned as
allocated memory block or free.
*/
struct VmaSuballocation
{
    VkDeviceSize offset;
    VkDeviceSize size;
    VmaAllocation hAllocation;
    VmaSuballocationType type;
};

typedef VmaList< VmaSuballocation, VmaStlAllocator<VmaSuballocation> > VmaSuballocationList;

// Cost of one additional allocation lost, as equivalent in bytes.
static const VkDeviceSize VMA_LOST_ALLOCATION_COST = 1048576;

/*
Parameters of planned allocation inside a VmaDeviceMemoryBlock.

If canMakeOtherLost was false:
- item points to a FREE suballocation.
- itemsToMakeLostCount is 0.

If canMakeOtherLost was true:
- item points to first of sequence of suballocations, which are either FREE,
  or point to VmaAllocations that can become lost.
- itemsToMakeLostCount is the number of VmaAllocations that need to be made lost for
  the requested allocation to succeed.
*/
struct VmaAllocationRequest
{
    VkDeviceSize offset;
    VkDeviceSize sumFreeSize; // Sum size of free items that overlap with proposed allocation.
    VkDeviceSize sumItemSize; // Sum size of items to make lost that overlap with proposed allocation.
    VmaSuballocationList::iterator item;
    size_t itemsToMakeLostCount;

    VkDeviceSize CalcCost() const
    {
        return sumItemSize + itemsToMakeLostCount * VMA_LOST_ALLOCATION_COST;
    }
};

/*
Represents a single block of device memory (VkDeviceMemory ) with all the
data about its regions (aka suballocations, VmaAllocation), assigned and free.

Thread-safety: This class must be externally synchronized.
*/
class VmaDeviceMemoryBlock
{
public:
    uint32_t m_MemoryTypeIndex;
    VMA_BLOCK_VECTOR_TYPE m_BlockVectorType;
    VkDeviceMemory m_hMemory;
    VkDeviceSize m_Size;
    bool m_PersistentMap;
    void* m_pMappedData;
    uint32_t m_FreeCount;
    VkDeviceSize m_SumFreeSize;
    VmaSuballocationList m_Suballocations;
    // Suballocations that are free and have size greater than certain threshold.
    // Sorted by size, ascending.
    VmaVector< VmaSuballocationList::iterator, VmaStlAllocator< VmaSuballocationList::iterator > > m_FreeSuballocationsBySize;

    VmaDeviceMemoryBlock(VmaAllocator hAllocator);

    ~VmaDeviceMemoryBlock()
    {
        VMA_ASSERT(m_hMemory == VK_NULL_HANDLE);
    }

    // Always call after construction.
    void Init(
        uint32_t newMemoryTypeIndex,
        VMA_BLOCK_VECTOR_TYPE newBlockVectorType,
        VkDeviceMemory newMemory,
        VkDeviceSize newSize,
        bool persistentMap,
        void* pMappedData);
    // Always call before destruction.
    void Destroy(VmaAllocator allocator);
    
    // Validates all data structures inside this object. If not valid, returns false.
    bool Validate() const;
    
    // Tries to find a place for suballocation with given parameters inside this allocation.
    // If succeeded, fills pAllocationRequest and returns true.
    // If failed, returns false.
    bool CreateAllocationRequest(
        uint32_t currentFrameIndex,
        uint32_t frameInUseCount,
        VkDeviceSize bufferImageGranularity,
        VkDeviceSize allocSize,
        VkDeviceSize allocAlignment,
        VmaSuballocationType allocType,
        bool canMakeOtherLost,
        VmaAllocationRequest* pAllocationRequest);

    bool MakeRequestedAllocationsLost(uint32_t currentFrameIndex, uint32_t frameInUseCount, VmaAllocationRequest* pAllocationRequest);

    uint32_t MakeAllocationsLost(uint32_t currentFrameIndex, uint32_t frameInUseCount);

    // Returns true if this allocation is empty - contains only single free suballocation.
    bool IsEmpty() const;

    // Makes actual allocation based on request. Request must already be checked
    // and valid.
    void Alloc(
        const VmaAllocationRequest& request,
        VmaSuballocationType type,
        VkDeviceSize allocSize,
        VmaAllocation hAllocation);

    // Frees suballocation assigned to given memory region.
    void Free(const VmaAllocation allocation);

#if VMA_STATS_STRING_ENABLED
    void PrintDetailedMap(class VmaJsonWriter& json) const;
#endif

private:
    // Checks if requested suballocation with given parameters can be placed in given pFreeSuballocItem.
    // If yes, fills pOffset and returns true. If no, returns false.
    bool CheckAllocation(
        uint32_t currentFrameIndex,
        uint32_t frameInUseCount,
        VkDeviceSize bufferImageGranularity,
        VkDeviceSize allocSize,
        VkDeviceSize allocAlignment,
        VmaSuballocationType allocType,
        VmaSuballocationList::const_iterator suballocItem,
        bool canMakeOtherLost,
        VkDeviceSize* pOffset,
        size_t* itemsToMakeLostCount,
        VkDeviceSize* pSumFreeSize,
        VkDeviceSize* pSumItemSize) const;
    
    // Given free suballocation, it merges it with following one, which must also be free.
    void MergeFreeWithNext(VmaSuballocationList::iterator item);
    // Releases given suballocation, making it free.
    // Merges it with adjacent free suballocations if applicable.
    // Returns iterator to new free suballocation at this place.
    VmaSuballocationList::iterator FreeSuballocation(VmaSuballocationList::iterator suballocItem);
    // Given free suballocation, it inserts it into sorted list of
    // m_FreeSuballocationsBySize if it's suitable.
    void RegisterFreeSuballocation(VmaSuballocationList::iterator item);
    // Given free suballocation, it removes it from sorted list of
    // m_FreeSuballocationsBySize if it's suitable.
    void UnregisterFreeSuballocation(VmaSuballocationList::iterator item);

    bool ValidateFreeSuballocationList() const;
};

struct VmaPointerLess
{
    bool operator()(const void* lhs, const void* rhs) const
    {
        return lhs < rhs;
    }
};

class VmaDefragmentator;

/*
Sequence of VmaDeviceMemoryBlock. Represents memory blocks allocated for a specific
Vulkan memory type.

Synchronized internally with a mutex.
*/
struct VmaBlockVector
{
    VmaBlockVector(
        VmaAllocator hAllocator,
        uint32_t memoryTypeIndex,
        VMA_BLOCK_VECTOR_TYPE blockVectorType,
        VkDeviceSize preferredBlockSize,
        size_t minBlockCount,
        size_t maxBlockCount,
        VkDeviceSize bufferImageGranularity,
        uint32_t frameInUseCount,
        bool isCustomPool);
    ~VmaBlockVector();

    VkResult CreateMinBlocks();

    uint32_t GetMemoryTypeIndex() const { return m_MemoryTypeIndex; }
    VkDeviceSize GetPreferredBlockSize() const { return m_PreferredBlockSize; }
    VkDeviceSize GetBufferImageGranularity() const { return m_BufferImageGranularity; }
    uint32_t GetFrameInUseCount() const { return m_FrameInUseCount; }
    VMA_BLOCK_VECTOR_TYPE GetBlockVectorType() const { return m_BlockVectorType; }

    void GetPoolStats(VmaPoolStats* pStats);

    bool IsEmpty() const { return m_Blocks.empty(); }

    VkResult Allocate(
        VmaPool hCurrentPool,
        uint32_t currentFrameIndex,
        const VkMemoryRequirements& vkMemReq,
        const VmaAllocationCreateInfo& createInfo,
        VmaSuballocationType suballocType,
        VmaAllocation* pAllocation);

    void Free(
        VmaAllocation hAllocation);

    // Adds statistics of this BlockVector to pStats.
    void AddStats(VmaStats* pStats);

#if VMA_STATS_STRING_ENABLED
    void PrintDetailedMap(class VmaJsonWriter& json);
#endif

    void UnmapPersistentlyMappedMemory();
    VkResult MapPersistentlyMappedMemory();

    void MakePoolAllocationsLost(
        uint32_t currentFrameIndex,
        size_t* pLostAllocationCount);

    VmaDefragmentator* EnsureDefragmentator(
        VkDevice hDevice,
        const VkAllocationCallbacks* pAllocationCallbacks,
        uint32_t currentFrameIndex);

    VkResult Defragment(
        VmaDefragmentationStats* pDefragmentationStats,
        VkDeviceSize& maxBytesToMove,
        uint32_t& maxAllocationsToMove);

    void DestroyDefragmentator();

private:
    friend class VmaDefragmentator;

    const VmaAllocator m_hAllocator;
    const uint32_t m_MemoryTypeIndex;
    const VMA_BLOCK_VECTOR_TYPE m_BlockVectorType;
    const VkDeviceSize m_PreferredBlockSize;
    const size_t m_MinBlockCount;
    const size_t m_MaxBlockCount;
    const VkDeviceSize m_BufferImageGranularity;
    const uint32_t m_FrameInUseCount;
    const bool m_IsCustomPool;
    VMA_MUTEX m_Mutex;
    // Incrementally sorted by sumFreeSize, ascending.
    VmaVector< VmaDeviceMemoryBlock*, VmaStlAllocator<VmaDeviceMemoryBlock*> > m_Blocks;
    /* There can be at most one allocation that is completely empty - a
    hysteresis to avoid pessimistic case of alternating creation and destruction
    of a VkDeviceMemory. */
    bool m_HasEmptyBlock;
    VmaDefragmentator* m_pDefragmentator;

    // Finds and removes given block from vector.
    void Remove(VmaDeviceMemoryBlock* pBlock);

    // Performs single step in sorting m_Blocks. They may not be fully sorted
    // after this call.
    void IncrementallySortBlocks();

    VkResult CreateBlock(VkDeviceSize blockSize, size_t* pNewBlockIndex);
};

struct VmaPool_T
{
public:
    VmaBlockVector m_BlockVector;

    // Takes ownership.
    VmaPool_T(
        VmaAllocator hAllocator,
        const VmaPoolCreateInfo& createInfo);
    ~VmaPool_T();

    VmaBlockVector& GetBlockVector() { return m_BlockVector; }

#if VMA_STATS_STRING_ENABLED
    //void PrintDetailedMap(class VmaStringBuilder& sb);
#endif
};

class VmaDefragmentator
{
    const VkDevice m_hDevice;
    const VkAllocationCallbacks* const m_pAllocationCallbacks;
    VmaBlockVector* const m_pBlockVector;
    uint32_t m_CurrentFrameIndex;
    VMA_BLOCK_VECTOR_TYPE m_BlockVectorType;
    VkDeviceSize m_BytesMoved;
    uint32_t m_AllocationsMoved;

    struct AllocationInfo
    {
        VmaAllocation m_hAllocation;
        VkBool32* m_pChanged;

        AllocationInfo() :
            m_hAllocation(VK_NULL_HANDLE),
            m_pChanged(VMA_NULL)
        {
        }
    };

    struct AllocationInfoSizeGreater
    {
        bool operator()(const AllocationInfo& lhs, const AllocationInfo& rhs) const
        {
            return lhs.m_hAllocation->GetSize() > rhs.m_hAllocation->GetSize();
        }
    };

    // Used between AddAllocation and Defragment.
    VmaVector< AllocationInfo, VmaStlAllocator<AllocationInfo> > m_Allocations;

    struct BlockInfo
    {
        VmaDeviceMemoryBlock* m_pBlock;
        bool m_HasNonMovableAllocations;
        VmaVector< AllocationInfo, VmaStlAllocator<AllocationInfo> > m_Allocations;

        BlockInfo(const VkAllocationCallbacks* pAllocationCallbacks) :
            m_pBlock(VMA_NULL),
            m_HasNonMovableAllocations(true),
            m_Allocations(pAllocationCallbacks),
            m_pMappedDataForDefragmentation(VMA_NULL)
        {
        }

        void CalcHasNonMovableAllocations()
        {
            const size_t blockAllocCount =
                m_pBlock->m_Suballocations.size() - m_pBlock->m_FreeCount;
            const size_t defragmentAllocCount = m_Allocations.size();
            m_HasNonMovableAllocations = blockAllocCount != defragmentAllocCount;
        }

        void SortAllocationsBySizeDescecnding()
        {
            VMA_SORT(m_Allocations.begin(), m_Allocations.end(), AllocationInfoSizeGreater());
        }

        VkResult EnsureMapping(VkDevice hDevice, void** ppMappedData)
        {
            // It has already been mapped for defragmentation.
            if(m_pMappedDataForDefragmentation)
            {
                *ppMappedData = m_pMappedDataForDefragmentation;
                return VK_SUCCESS;
            }
            
            // It is persistently mapped.
            if(m_pBlock->m_PersistentMap)
            {
                VMA_ASSERT(m_pBlock->m_pMappedData != VMA_NULL);
                *ppMappedData = m_pBlock->m_pMappedData;
                return VK_SUCCESS;
            }
            
            // Map on first usage.
            VkResult res = vkMapMemory(hDevice, m_pBlock->m_hMemory, 0, VK_WHOLE_SIZE, 0, &m_pMappedDataForDefragmentation);
            *ppMappedData = m_pMappedDataForDefragmentation;
            return res;
        }

        void Unmap(VkDevice hDevice)
        {
            if(m_pMappedDataForDefragmentation != VMA_NULL)
            {
                vkUnmapMemory(hDevice, m_pBlock->m_hMemory);
            }
        }

    private:
        // Not null if mapped for defragmentation only, not persistently mapped.
        void* m_pMappedDataForDefragmentation;
    };

    struct BlockPointerLess
    {
        bool operator()(const BlockInfo* pLhsBlockInfo, const VmaDeviceMemoryBlock* pRhsBlock) const
        {
            return pLhsBlockInfo->m_pBlock < pRhsBlock;
        }
        bool operator()(const BlockInfo* pLhsBlockInfo, const BlockInfo* pRhsBlockInfo) const
        {
            return pLhsBlockInfo->m_pBlock < pRhsBlockInfo->m_pBlock;
        }
    };

    // 1. Blocks with some non-movable allocations go first.
    // 2. Blocks with smaller sumFreeSize go first.
    struct BlockInfoCompareMoveDestination
    {
        bool operator()(const BlockInfo* pLhsBlockInfo, const BlockInfo* pRhsBlockInfo) const
        {
            if(pLhsBlockInfo->m_HasNonMovableAllocations && !pRhsBlockInfo->m_HasNonMovableAllocations)
            {
                return true;
            }
            if(!pLhsBlockInfo->m_HasNonMovableAllocations && pRhsBlockInfo->m_HasNonMovableAllocations)
            {
                return false;
            }
            if(pLhsBlockInfo->m_pBlock->m_SumFreeSize < pRhsBlockInfo->m_pBlock->m_SumFreeSize)
            {
                return true;
            }
            return false;
        }
    };

    typedef VmaVector< BlockInfo*, VmaStlAllocator<BlockInfo*> > BlockInfoVector;
    BlockInfoVector m_Blocks;

    VkResult DefragmentRound(
        VkDeviceSize maxBytesToMove,
        uint32_t maxAllocationsToMove);

    static bool MoveMakesSense(
        size_t dstBlockIndex, VkDeviceSize dstOffset,
        size_t srcBlockIndex, VkDeviceSize srcOffset);

public:
    VmaDefragmentator(
        VkDevice hDevice,
        const VkAllocationCallbacks* pAllocationCallbacks,
        VmaBlockVector* pBlockVector,
        uint32_t currentFrameIndex);

    ~VmaDefragmentator();

    VkDeviceSize GetBytesMoved() const { return m_BytesMoved; }
    uint32_t GetAllocationsMoved() const { return m_AllocationsMoved; }

    void AddAllocation(VmaAllocation hAlloc, VkBool32* pChanged);

    VkResult Defragment(
        VkDeviceSize maxBytesToMove,
        uint32_t maxAllocationsToMove);
};

// Main allocator object.
struct VmaAllocator_T
{
    bool m_UseMutex;
    VkDevice m_hDevice;
    bool m_AllocationCallbacksSpecified;
    VkAllocationCallbacks m_AllocationCallbacks;
    VmaDeviceMemoryCallbacks m_DeviceMemoryCallbacks;
    // Non-zero when we are inside UnmapPersistentlyMappedMemory...MapPersistentlyMappedMemory.
    // Counter to allow nested calls to these functions.
    uint32_t m_UnmapPersistentlyMappedMemoryCounter;

    VkPhysicalDeviceProperties m_PhysicalDeviceProperties;
    VkPhysicalDeviceMemoryProperties m_MemProps;

    // Default pools.
    VmaBlockVector* m_pBlockVectors[VK_MAX_MEMORY_TYPES][VMA_BLOCK_VECTOR_TYPE_COUNT];

    // Each vector is sorted by memory (handle value).
    typedef VmaVector< VmaAllocation, VmaStlAllocator<VmaAllocation> > AllocationVectorType;
    AllocationVectorType* m_pOwnAllocations[VK_MAX_MEMORY_TYPES][VMA_BLOCK_VECTOR_TYPE_COUNT];
    VMA_MUTEX m_OwnAllocationsMutex[VK_MAX_MEMORY_TYPES];

    VmaAllocator_T(const VmaAllocatorCreateInfo* pCreateInfo);
    ~VmaAllocator_T();

    const VkAllocationCallbacks* GetAllocationCallbacks() const
    {
        return m_AllocationCallbacksSpecified ? &m_AllocationCallbacks : 0;
    }

    VkDeviceSize GetBufferImageGranularity() const
    {
        return VMA_MAX(
            static_cast<VkDeviceSize>(VMA_DEBUG_MIN_BUFFER_IMAGE_GRANULARITY),
            m_PhysicalDeviceProperties.limits.bufferImageGranularity);
    }

    uint32_t GetMemoryHeapCount() const { return m_MemProps.memoryHeapCount; }
    uint32_t GetMemoryTypeCount() const { return m_MemProps.memoryTypeCount; }

    uint32_t MemoryTypeIndexToHeapIndex(uint32_t memTypeIndex) const
    {
        VMA_ASSERT(memTypeIndex < m_MemProps.memoryTypeCount);
        return m_MemProps.memoryTypes[memTypeIndex].heapIndex;
    }

    // Main allocation function.
    VkResult AllocateMemory(
        const VkMemoryRequirements& vkMemReq,
        const VmaAllocationCreateInfo& createInfo,
        VmaSuballocationType suballocType,
        VmaAllocation* pAllocation);

    // Main deallocation function.
    void FreeMemory(const VmaAllocation allocation);

    void CalculateStats(VmaStats* pStats);

#if VMA_STATS_STRING_ENABLED
    void PrintDetailedMap(class VmaJsonWriter& json);
#endif

    void UnmapPersistentlyMappedMemory();
    VkResult MapPersistentlyMappedMemory();

    VkResult Defragment(
        VmaAllocation* pAllocations,
        size_t allocationCount,
        VkBool32* pAllocationsChanged,
        const VmaDefragmentationInfo* pDefragmentationInfo,
        VmaDefragmentationStats* pDefragmentationStats);

    void GetAllocationInfo(VmaAllocation hAllocation, VmaAllocationInfo* pAllocationInfo);

    VkResult CreatePool(const VmaPoolCreateInfo* pCreateInfo, VmaPool* pPool);
    void DestroyPool(VmaPool pool);
    void GetPoolStats(VmaPool pool, VmaPoolStats* pPoolStats);

    void SetCurrentFrameIndex(uint32_t frameIndex);

    void MakePoolAllocationsLost(
        VmaPool hPool,
        size_t* pLostAllocationCount);

    void CreateLostAllocation(VmaAllocation* pAllocation);

private:
    VkDeviceSize m_PreferredLargeHeapBlockSize;
    VkDeviceSize m_PreferredSmallHeapBlockSize;

    VkPhysicalDevice m_PhysicalDevice;
    VMA_ATOMIC_UINT32 m_CurrentFrameIndex;
    
    VMA_MUTEX m_PoolsMutex;
    // Protected by m_PoolsMutex. Sorted by pointer value.
    VmaVector<VmaPool, VmaStlAllocator<VmaPool> > m_Pools;

    VkDeviceSize CalcPreferredBlockSize(uint32_t memTypeIndex);

    VkResult AllocateMemoryOfType(
        const VkMemoryRequirements& vkMemReq,
        const VmaAllocationCreateInfo& createInfo,
        uint32_t memTypeIndex,
        VmaSuballocationType suballocType,
        VmaAllocation* pAllocation);

    // Allocates and registers new VkDeviceMemory specifically for single allocation.
    VkResult AllocateOwnMemory(
        VkDeviceSize size,
        VmaSuballocationType suballocType,
        uint32_t memTypeIndex,
        bool map,
        void* pUserData,
        VmaAllocation* pAllocation);

    // Tries to free pMemory as Own Memory. Returns true if found and freed.
    void FreeOwnMemory(VmaAllocation allocation);
};

////////////////////////////////////////////////////////////////////////////////
// Memory allocation #2 after VmaAllocator_T definition

static void* VmaMalloc(VmaAllocator hAllocator, size_t size, size_t alignment)
{
    return VmaMalloc(&hAllocator->m_AllocationCallbacks, size, alignment);
}

static void VmaFree(VmaAllocator hAllocator, void* ptr)
{
    VmaFree(&hAllocator->m_AllocationCallbacks, ptr);
}

template<typename T>
static T* VmaAllocate(VmaAllocator hAllocator)
{
    return (T*)VmaMalloc(hAllocator, sizeof(T), VMA_ALIGN_OF(T));
}

template<typename T>
static T* VmaAllocateArray(VmaAllocator hAllocator, size_t count)
{
    return (T*)VmaMalloc(hAllocator, sizeof(T) * count, VMA_ALIGN_OF(T));
}

template<typename T>
static void vma_delete(VmaAllocator hAllocator, T* ptr)
{
    if(ptr != VMA_NULL)
    {
        ptr->~T();
        VmaFree(hAllocator, ptr);
    }
}

template<typename T>
static void vma_delete_array(VmaAllocator hAllocator, T* ptr, size_t count)
{
    if(ptr != VMA_NULL)
    {
        for(size_t i = count; i--; )
            ptr[i].~T();
        VmaFree(hAllocator, ptr);
    }
}

////////////////////////////////////////////////////////////////////////////////
// VmaStringBuilder

#if VMA_STATS_STRING_ENABLED

class VmaStringBuilder
{
public:
    VmaStringBuilder(VmaAllocator alloc) : m_Data(VmaStlAllocator<char>(alloc->GetAllocationCallbacks())) { }
    size_t GetLength() const { return m_Data.size(); }
    const char* GetData() const { return m_Data.data(); }

    void Add(char ch) { m_Data.push_back(ch); }
    void Add(const char* pStr);
    void AddNewLine() { Add('\n'); }
    void AddNumber(uint32_t num);
    void AddNumber(uint64_t num);
    void AddPointer(const void* ptr);

private:
    VmaVector< char, VmaStlAllocator<char> > m_Data;
};

void VmaStringBuilder::Add(const char* pStr)
{
    const size_t strLen = strlen(pStr);
    if(strLen > 0)
    {
        const size_t oldCount = m_Data.size();
        m_Data.resize(oldCount + strLen);
        memcpy(m_Data.data() + oldCount, pStr, strLen);
    }
}

void VmaStringBuilder::AddNumber(uint32_t num)
{
    char buf[11];
    VmaUint32ToStr(buf, sizeof(buf), num);
    Add(buf);
}

void VmaStringBuilder::AddNumber(uint64_t num)
{
    char buf[21];
    VmaUint64ToStr(buf, sizeof(buf), num);
    Add(buf);
}

void VmaStringBuilder::AddPointer(const void* ptr)
{
    char buf[21];
    VmaPtrToStr(buf, sizeof(buf), ptr);
    Add(buf);
}

#endif // #if VMA_STATS_STRING_ENABLED

////////////////////////////////////////////////////////////////////////////////
// VmaJsonWriter

#if VMA_STATS_STRING_ENABLED

class VmaJsonWriter
{
public:
    VmaJsonWriter(const VkAllocationCallbacks* pAllocationCallbacks, VmaStringBuilder& sb);
    ~VmaJsonWriter();

    void BeginObject(bool singleLine = false);
    void EndObject();
    
    void BeginArray(bool singleLine = false);
    void EndArray();
    
    void WriteString(const char* pStr);
    void BeginString(const char* pStr = VMA_NULL);
    void ContinueString(const char* pStr);
    void ContinueString(uint32_t n);
    void ContinueString(uint64_t n);
    void EndString(const char* pStr = VMA_NULL);
    
    void WriteNumber(uint32_t n);
    void WriteNumber(uint64_t n);
    void WriteBool(bool b);
    void WriteNull();

private:
    static const char* const INDENT;

    enum COLLECTION_TYPE
    {
        COLLECTION_TYPE_OBJECT,
        COLLECTION_TYPE_ARRAY,
    };
    struct StackItem
    {
        COLLECTION_TYPE type;
        uint32_t valueCount;
        bool singleLineMode;
    };

    VmaStringBuilder& m_SB;
    VmaVector< StackItem, VmaStlAllocator<StackItem> > m_Stack;
    bool m_InsideString;

    void BeginValue(bool isString);
    void WriteIndent(bool oneLess = false);
};

const char* const VmaJsonWriter::INDENT = "  ";

VmaJsonWriter::VmaJsonWriter(const VkAllocationCallbacks* pAllocationCallbacks, VmaStringBuilder& sb) :
    m_SB(sb),
    m_Stack(VmaStlAllocator<StackItem>(pAllocationCallbacks)),
    m_InsideString(false)
{
}

VmaJsonWriter::~VmaJsonWriter()
{
    VMA_ASSERT(!m_InsideString);
    VMA_ASSERT(m_Stack.empty());
}

void VmaJsonWriter::BeginObject(bool singleLine)
{
    VMA_ASSERT(!m_InsideString);

    BeginValue(false);
    m_SB.Add('{');

    StackItem item;
    item.type = COLLECTION_TYPE_OBJECT;
    item.valueCount = 0;
    item.singleLineMode = singleLine;
    m_Stack.push_back(item);
}

void VmaJsonWriter::EndObject()
{
    VMA_ASSERT(!m_InsideString);

    WriteIndent(true);
    m_SB.Add('}');

    VMA_ASSERT(!m_Stack.empty() && m_Stack.back().type == COLLECTION_TYPE_OBJECT);
    m_Stack.pop_back();
}

void VmaJsonWriter::BeginArray(bool singleLine)
{
    VMA_ASSERT(!m_InsideString);

    BeginValue(false);
    m_SB.Add('[');

    StackItem item;
    item.type = COLLECTION_TYPE_ARRAY;
    item.valueCount = 0;
    item.singleLineMode = singleLine;
    m_Stack.push_back(item);
}

void VmaJsonWriter::EndArray()
{
    VMA_ASSERT(!m_InsideString);

    WriteIndent(true);
    m_SB.Add(']');

    VMA_ASSERT(!m_Stack.empty() && m_Stack.back().type == COLLECTION_TYPE_ARRAY);
    m_Stack.pop_back();
}

void VmaJsonWriter::WriteString(const char* pStr)
{
    BeginString(pStr);
    EndString();
}

void VmaJsonWriter::BeginString(const char* pStr)
{
    VMA_ASSERT(!m_InsideString);

    BeginValue(true);
    m_SB.Add('"');
    m_InsideString = true;
    if(pStr != VMA_NULL && pStr[0] != '\0')
    {
        ContinueString(pStr);
    }
}

void VmaJsonWriter::ContinueString(const char* pStr)
{
    VMA_ASSERT(m_InsideString);

    const size_t strLen = strlen(pStr);
    for(size_t i = 0; i < strLen; ++i)
    {
        char ch = pStr[i];
        if(ch == '\'')
        {
            m_SB.Add("\\\\");
        }
        else if(ch == '"')
        {
            m_SB.Add("\\\"");
        }
        else if(ch >= 32)
        {
            m_SB.Add(ch);
        }
        else switch(ch)
        {
        case '\n':
            m_SB.Add("\\n");
            break;
        case '\r':
            m_SB.Add("\\r");
            break;
        case '\t':
            m_SB.Add("\\t");
            break;
        default:
            VMA_ASSERT(0 && "Character not currently supported.");
            break;
        }
    }
}

void VmaJsonWriter::ContinueString(uint32_t n)
{
    VMA_ASSERT(m_InsideString);
    m_SB.AddNumber(n);
}

void VmaJsonWriter::ContinueString(uint64_t n)
{
    VMA_ASSERT(m_InsideString);
    m_SB.AddNumber(n);
}

void VmaJsonWriter::EndString(const char* pStr)
{
    VMA_ASSERT(m_InsideString);
    if(pStr != VMA_NULL && pStr[0] != '\0')
    {
        ContinueString(pStr);
    }
    m_SB.Add('"');
    m_InsideString = false;
}

void VmaJsonWriter::WriteNumber(uint32_t n)
{
    VMA_ASSERT(!m_InsideString);
    BeginValue(false);
    m_SB.AddNumber(n);
}

void VmaJsonWriter::WriteNumber(uint64_t n)
{
    VMA_ASSERT(!m_InsideString);
    BeginValue(false);
    m_SB.AddNumber(n);
}

void VmaJsonWriter::WriteBool(bool b)
{
    VMA_ASSERT(!m_InsideString);
    BeginValue(false);
    m_SB.Add(b ? "true" : "false");
}

void VmaJsonWriter::WriteNull()
{
    VMA_ASSERT(!m_InsideString);
    BeginValue(false);
    m_SB.Add("null");
}

void VmaJsonWriter::BeginValue(bool isString)
{
    if(!m_Stack.empty())
    {
        StackItem& currItem = m_Stack.back();
        if(currItem.type == COLLECTION_TYPE_OBJECT &&
            currItem.valueCount % 2 == 0)
        {
            VMA_ASSERT(isString);
        }

        if(currItem.type == COLLECTION_TYPE_OBJECT &&
            currItem.valueCount % 2 != 0)
        {
            m_SB.Add(": ");
        }
        else if(currItem.valueCount > 0)
        {
            m_SB.Add(", ");
            WriteIndent();
        }
        else
        {
            WriteIndent();
        }
        ++currItem.valueCount;
    }
}

void VmaJsonWriter::WriteIndent(bool oneLess)
{
    if(!m_Stack.empty() && !m_Stack.back().singleLineMode)
    {
        m_SB.AddNewLine();
        
        size_t count = m_Stack.size();
        if(count > 0 && oneLess)
        {
            --count;
        }
        for(size_t i = 0; i < count; ++i)
        {
            m_SB.Add(INDENT);
        }
    }
}

#endif // #if VMA_STATS_STRING_ENABLED

////////////////////////////////////////////////////////////////////////////////

VkDeviceSize VmaAllocation_T::GetOffset() const
{
    switch(m_Type)
    {
    case ALLOCATION_TYPE_BLOCK:
        return m_BlockAllocation.m_Offset;
    case ALLOCATION_TYPE_OWN:
        return 0;
    default:
        VMA_ASSERT(0);
        return 0;
    }
}

VkDeviceMemory VmaAllocation_T::GetMemory() const
{
    switch(m_Type)
    {
    case ALLOCATION_TYPE_BLOCK:
        return m_BlockAllocation.m_Block->m_hMemory;
    case ALLOCATION_TYPE_OWN:
        return m_OwnAllocation.m_hMemory;
    default:
        VMA_ASSERT(0);
        return VK_NULL_HANDLE;
    }
}

uint32_t VmaAllocation_T::GetMemoryTypeIndex() const
{
    switch(m_Type)
    {
    case ALLOCATION_TYPE_BLOCK:
        return m_BlockAllocation.m_Block->m_MemoryTypeIndex;
    case ALLOCATION_TYPE_OWN:
        return m_OwnAllocation.m_MemoryTypeIndex;
    default:
        VMA_ASSERT(0);
        return UINT32_MAX;
    }
}

VMA_BLOCK_VECTOR_TYPE VmaAllocation_T::GetBlockVectorType() const
{
    switch(m_Type)
    {
    case ALLOCATION_TYPE_BLOCK:
        return m_BlockAllocation.m_Block->m_BlockVectorType;
    case ALLOCATION_TYPE_OWN:
        return (m_OwnAllocation.m_PersistentMap ? VMA_BLOCK_VECTOR_TYPE_MAPPED : VMA_BLOCK_VECTOR_TYPE_UNMAPPED);
    default:
        VMA_ASSERT(0);
        return VMA_BLOCK_VECTOR_TYPE_COUNT;
    }
}

void* VmaAllocation_T::GetMappedData() const
{
    switch(m_Type)
    {
    case ALLOCATION_TYPE_BLOCK:
        if(m_BlockAllocation.m_Block->m_pMappedData != VMA_NULL)
        {
            return (char*)m_BlockAllocation.m_Block->m_pMappedData + m_BlockAllocation.m_Offset;
        }
        else
        {
            return VMA_NULL;
        }
        break;
    case ALLOCATION_TYPE_OWN:
        return m_OwnAllocation.m_pMappedData;
    default:
        VMA_ASSERT(0);
        return VMA_NULL;
    }
}

bool VmaAllocation_T::CanBecomeLost() const
{
    switch(m_Type)
    {
    case ALLOCATION_TYPE_BLOCK:
        return m_BlockAllocation.m_CanBecomeLost;
    case ALLOCATION_TYPE_OWN:
        return false;
    default:
        VMA_ASSERT(0);
        return false;
    }
}

VmaPool VmaAllocation_T::GetPool() const
{
    VMA_ASSERT(m_Type == ALLOCATION_TYPE_BLOCK);
    return m_BlockAllocation.m_hPool;
}

bool VmaAllocation_T::MakeLost(uint32_t currentFrameIndex, uint32_t frameInUseCount)
{
    VMA_ASSERT(CanBecomeLost());

    /*
    Warning: This is a carefully designed algorithm.
    Do not modify unless you really know what you're doing :)
    */
    uint32_t localLastUseFrameIndex = GetLastUseFrameIndex();
    for(;;)
    {
        if(localLastUseFrameIndex == VMA_FRAME_INDEX_LOST)
        {
            VMA_ASSERT(0);
            return false;
        }
        else if(localLastUseFrameIndex + frameInUseCount >= currentFrameIndex)
        {
            return false;
        }
        else // Last use time earlier than current time.
        {
            if(CompareExchangeLastUseFrameIndex(localLastUseFrameIndex, VMA_FRAME_INDEX_LOST))
            {
                // Setting hAllocation.LastUseFrameIndex atomic to VMA_FRAME_INDEX_LOST is enough to mark it as LOST.
                // Calling code just needs to unregister this allocation in owning VmaDeviceMemoryBlock.
                return true;
            }
        }
    }
}

#if VMA_STATS_STRING_ENABLED

// Correspond to values of enum VmaSuballocationType.
static const char* VMA_SUBALLOCATION_TYPE_NAMES[] = {
    "FREE",
    "UNKNOWN",
    "BUFFER",
    "IMAGE_UNKNOWN",
    "IMAGE_LINEAR",
    "IMAGE_OPTIMAL",
};

static void VmaPrintStatInfo(VmaJsonWriter& json, const VmaStatInfo& stat)
{
    json.BeginObject();

    json.WriteString("Blocks");
    json.WriteNumber(stat.BlockCount);

    json.WriteString("Allocations");
    json.WriteNumber(stat.AllocationCount);

    json.WriteString("UnusedRanges");
    json.WriteNumber(stat.UnusedRangeCount);

    json.WriteString("UsedBytes");
    json.WriteNumber(stat.UsedBytes);

    json.WriteString("UnusedBytes");
    json.WriteNumber(stat.UnusedBytes);

    if(stat.AllocationCount > 1)
    {
        json.WriteString("AllocationSize");
        json.BeginObject(true);
        json.WriteString("Min");
        json.WriteNumber(stat.AllocationSizeMin);
        json.WriteString("Avg");
        json.WriteNumber(stat.AllocationSizeAvg);
        json.WriteString("Max");
        json.WriteNumber(stat.AllocationSizeMax);
        json.EndObject();
    }

    if(stat.UnusedRangeCount > 1)
    {
        json.WriteString("UnusedRangeSize");
        json.BeginObject(true);
        json.WriteString("Min");
        json.WriteNumber(stat.UnusedRangeSizeMin);
        json.WriteString("Avg");
        json.WriteNumber(stat.UnusedRangeSizeAvg);
        json.WriteString("Max");
        json.WriteNumber(stat.UnusedRangeSizeMax);
        json.EndObject();
    }

    json.EndObject();
}

#endif // #if VMA_STATS_STRING_ENABLED

struct VmaSuballocationItemSizeLess
{
    bool operator()(
        const VmaSuballocationList::iterator lhs,
        const VmaSuballocationList::iterator rhs) const
    {
        return lhs->size < rhs->size;
    }
    bool operator()(
        const VmaSuballocationList::iterator lhs,
        VkDeviceSize rhsSize) const
    {
        return lhs->size < rhsSize;
    }
};

VmaDeviceMemoryBlock::VmaDeviceMemoryBlock(VmaAllocator hAllocator) :
    m_MemoryTypeIndex(UINT32_MAX),
    m_BlockVectorType(VMA_BLOCK_VECTOR_TYPE_COUNT),
    m_hMemory(VK_NULL_HANDLE),
    m_Size(0),
    m_PersistentMap(false),
    m_pMappedData(VMA_NULL),
    m_FreeCount(0),
    m_SumFreeSize(0),
    m_Suballocations(VmaStlAllocator<VmaSuballocation>(hAllocator->GetAllocationCallbacks())),
    m_FreeSuballocationsBySize(VmaStlAllocator<VmaSuballocationList::iterator>(hAllocator->GetAllocationCallbacks()))
{
}

void VmaDeviceMemoryBlock::Init(
    uint32_t newMemoryTypeIndex,
    VMA_BLOCK_VECTOR_TYPE newBlockVectorType,
    VkDeviceMemory newMemory,
    VkDeviceSize newSize,
    bool persistentMap,
    void* pMappedData)
{
    VMA_ASSERT(m_hMemory == VK_NULL_HANDLE);

    m_MemoryTypeIndex = newMemoryTypeIndex;
    m_BlockVectorType = newBlockVectorType;
    m_hMemory = newMemory;
    m_Size = newSize;
    m_PersistentMap = persistentMap;
    m_pMappedData = pMappedData;
    m_FreeCount = 1;
    m_SumFreeSize = newSize;

    m_Suballocations.clear();
    m_FreeSuballocationsBySize.clear();

    VmaSuballocation suballoc = {};
    suballoc.offset = 0;
    suballoc.size = newSize;
    suballoc.type = VMA_SUBALLOCATION_TYPE_FREE;
    suballoc.hAllocation = VK_NULL_HANDLE;

    m_Suballocations.push_back(suballoc);
    VmaSuballocationList::iterator suballocItem = m_Suballocations.end();
    --suballocItem;
    m_FreeSuballocationsBySize.push_back(suballocItem);
}

void VmaDeviceMemoryBlock::Destroy(VmaAllocator allocator)
{
    // This is the most important assert in the entire library.
    // Hitting it means you have some memory leak - unreleased VmaAllocation objects.
    VMA_ASSERT(IsEmpty() && "Some allocations were not freed before destruction of this memory block!");
    
    VMA_ASSERT(m_hMemory != VK_NULL_HANDLE);
    if(m_pMappedData != VMA_NULL)
    {
        vkUnmapMemory(allocator->m_hDevice, m_hMemory);
        m_pMappedData = VMA_NULL;
    }

    // Callback.
    if(allocator->m_DeviceMemoryCallbacks.pfnFree != VMA_NULL)
    {
        (*allocator->m_DeviceMemoryCallbacks.pfnFree)(allocator, m_MemoryTypeIndex, m_hMemory, m_Size);
    }

    vkFreeMemory(allocator->m_hDevice, m_hMemory, allocator->GetAllocationCallbacks());
    m_hMemory = VK_NULL_HANDLE;
}

bool VmaDeviceMemoryBlock::Validate() const
{
    if((m_hMemory == VK_NULL_HANDLE) ||
        (m_Size == 0) ||
        m_Suballocations.empty())
    {
        return false;
    }
    
    // Expected offset of new suballocation as calculates from previous ones.
    VkDeviceSize calculatedOffset = 0;
    // Expected number of free suballocations as calculated from traversing their list.
    uint32_t calculatedFreeCount = 0;
    // Expected sum size of free suballocations as calculated from traversing their list.
    VkDeviceSize calculatedSumFreeSize = 0;
    // Expected number of free suballocations that should be registered in
    // m_FreeSuballocationsBySize calculated from traversing their list.
    size_t freeSuballocationsToRegister = 0;
    // True if previous visisted suballocation was free.
    bool prevFree = false;

    for(VmaSuballocationList::const_iterator suballocItem = m_Suballocations.cbegin();
        suballocItem != m_Suballocations.cend();
        ++suballocItem)
    {
        const VmaSuballocation& subAlloc = *suballocItem;
        
        // Actual offset of this suballocation doesn't match expected one.
        if(subAlloc.offset != calculatedOffset)
        {
            return false;
        }

        const bool currFree = (subAlloc.type == VMA_SUBALLOCATION_TYPE_FREE);
        // Two adjacent free suballocations are invalid. They should be merged.
        if(prevFree && currFree)
        {
            return false;
        }
        prevFree = currFree;

        if(currFree != (subAlloc.hAllocation == VK_NULL_HANDLE))
        {
            return false;
        }

        if(currFree)
        {
            calculatedSumFreeSize += subAlloc.size;
            ++calculatedFreeCount;
            if(subAlloc.size >= VMA_MIN_FREE_SUBALLOCATION_SIZE_TO_REGISTER)
            {
                ++freeSuballocationsToRegister;
            }
        }

        calculatedOffset += subAlloc.size;
    }

    // Number of free suballocations registered in m_FreeSuballocationsBySize doesn't
    // match expected one.
    if(m_FreeSuballocationsBySize.size() != freeSuballocationsToRegister)
    {
        return false;
    }

    VkDeviceSize lastSize = 0;
    for(size_t i = 0; i < m_FreeSuballocationsBySize.size(); ++i)
    {
        VmaSuballocationList::iterator suballocItem = m_FreeSuballocationsBySize[i];
        
        // Only free suballocations can be registered in m_FreeSuballocationsBySize.
        if(suballocItem->type != VMA_SUBALLOCATION_TYPE_FREE)
        {
            return false;
        }
        // They must be sorted by size ascending.
        if(suballocItem->size < lastSize)
        {
            return false;
        }

        lastSize = suballocItem->size;
    }

    // Check if totals match calculacted values.
    return
        (calculatedOffset == m_Size) &&
        (calculatedSumFreeSize == m_SumFreeSize) &&
        (calculatedFreeCount == m_FreeCount);
}

/*
How many suitable free suballocations to analyze before choosing best one.
- Set to 1 to use First-Fit algorithm - first suitable free suballocation will
  be chosen.
- Set to UINT32_MAX to use Best-Fit/Worst-Fit algorithm - all suitable free
  suballocations will be analized and best one will be chosen.
- Any other value is also acceptable.
*/
//static const uint32_t MAX_SUITABLE_SUBALLOCATIONS_TO_CHECK = 8;

bool VmaDeviceMemoryBlock::CreateAllocationRequest(
    uint32_t currentFrameIndex,
    uint32_t frameInUseCount,
    VkDeviceSize bufferImageGranularity,
    VkDeviceSize allocSize,
    VkDeviceSize allocAlignment,
    VmaSuballocationType allocType,
    bool canMakeOtherLost,
    VmaAllocationRequest* pAllocationRequest)
{
    VMA_ASSERT(allocSize > 0);
    VMA_ASSERT(allocType != VMA_SUBALLOCATION_TYPE_FREE);
    VMA_ASSERT(pAllocationRequest != VMA_NULL);
    VMA_HEAVY_ASSERT(Validate());

    // There is not enough total free space in this block to fullfill the request: Early return.
    if(canMakeOtherLost == false && m_SumFreeSize < allocSize)
    {
        return false;
    }

    // New algorithm, efficiently searching freeSuballocationsBySize.
    const size_t freeSuballocCount = m_FreeSuballocationsBySize.size();
    if(freeSuballocCount > 0)
    {
        if(VMA_BEST_FIT)
        {
            // Find first free suballocation with size not less than allocSize.
            VmaSuballocationList::iterator* const it = VmaBinaryFindFirstNotLess(
                m_FreeSuballocationsBySize.data(),
                m_FreeSuballocationsBySize.data() + freeSuballocCount,
                allocSize,
                VmaSuballocationItemSizeLess());
            size_t index = it - m_FreeSuballocationsBySize.data();
            for(; index < freeSuballocCount; ++index)
            {
                if(CheckAllocation(
                    currentFrameIndex,
                    frameInUseCount,
                    bufferImageGranularity,
                    allocSize,
                    allocAlignment,
                    allocType,
                    m_FreeSuballocationsBySize[index],
                    false, // canMakeOtherLost
                    &pAllocationRequest->offset,
                    &pAllocationRequest->itemsToMakeLostCount,
                    &pAllocationRequest->sumFreeSize,
                    &pAllocationRequest->sumItemSize))
                {
                    pAllocationRequest->item = m_FreeSuballocationsBySize[index];
                    return true;
                }
            }
        }
        else
        {
            // Search staring from biggest suballocations.
            for(size_t index = freeSuballocCount; index--; )
            {
                if(CheckAllocation(
                    currentFrameIndex,
                    frameInUseCount,
                    bufferImageGranularity,
                    allocSize,
                    allocAlignment,
                    allocType,
                    m_FreeSuballocationsBySize[index],
                    false, // canMakeOtherLost
                    &pAllocationRequest->offset,
                    &pAllocationRequest->itemsToMakeLostCount,
                    &pAllocationRequest->sumFreeSize,
                    &pAllocationRequest->sumItemSize))
                {
                    pAllocationRequest->item = m_FreeSuballocationsBySize[index];
                    return true;
                }
            }
        }
    }

    if(canMakeOtherLost)
    {
        // Brute-force algorithm. TODO: Come up with something better.

        pAllocationRequest->sumFreeSize = VK_WHOLE_SIZE;
        pAllocationRequest->sumItemSize = VK_WHOLE_SIZE;

        VmaAllocationRequest tmpAllocRequest = {};
        for(VmaSuballocationList::iterator suballocIt = m_Suballocations.begin();
            suballocIt != m_Suballocations.end();
            ++suballocIt)
        {
            if(suballocIt->type == VMA_SUBALLOCATION_TYPE_FREE ||
                suballocIt->hAllocation->CanBecomeLost())
            {
                if(CheckAllocation(
                    currentFrameIndex,
                    frameInUseCount,
                    bufferImageGranularity,
                    allocSize,
                    allocAlignment,
                    allocType,
                    suballocIt,
                    canMakeOtherLost,
                    &tmpAllocRequest.offset,
                    &tmpAllocRequest.itemsToMakeLostCount,
                    &tmpAllocRequest.sumFreeSize,
                    &tmpAllocRequest.sumItemSize))
                {
                    tmpAllocRequest.item = suballocIt;

                    if(tmpAllocRequest.CalcCost() < pAllocationRequest->CalcCost())
                    {
                        *pAllocationRequest = tmpAllocRequest;
                    }
                }
            }
        }

        if(pAllocationRequest->sumItemSize != VK_WHOLE_SIZE)
        {
            return true;
        }
    }

    return false;
}

bool VmaDeviceMemoryBlock::MakeRequestedAllocationsLost(uint32_t currentFrameIndex, uint32_t frameInUseCount, VmaAllocationRequest* pAllocationRequest)
{
    while(pAllocationRequest->itemsToMakeLostCount > 0)
    {
        if(pAllocationRequest->item->type == VMA_SUBALLOCATION_TYPE_FREE)
        {
            ++pAllocationRequest->item;
        }
        VMA_ASSERT(pAllocationRequest->item != m_Suballocations.end());
        VMA_ASSERT(pAllocationRequest->item->hAllocation != VK_NULL_HANDLE);
        VMA_ASSERT(pAllocationRequest->item->hAllocation->CanBecomeLost());
        if(pAllocationRequest->item->hAllocation->MakeLost(currentFrameIndex, frameInUseCount))
        {
            pAllocationRequest->item = FreeSuballocation(pAllocationRequest->item);
            --pAllocationRequest->itemsToMakeLostCount;
        }
        else
        {
            return false;
        }
    }

    VMA_HEAVY_ASSERT(Validate());
    VMA_ASSERT(pAllocationRequest->item != m_Suballocations.end());
    VMA_ASSERT(pAllocationRequest->item->type == VMA_SUBALLOCATION_TYPE_FREE);
    
    return true;
}

uint32_t VmaDeviceMemoryBlock::MakeAllocationsLost(uint32_t currentFrameIndex, uint32_t frameInUseCount)
{
    uint32_t lostAllocationCount = 0;
    for(VmaSuballocationList::iterator it = m_Suballocations.begin();
        it != m_Suballocations.end();
        ++it)
    {
        if(it->type != VMA_SUBALLOCATION_TYPE_FREE &&
            it->hAllocation->CanBecomeLost() &&
            it->hAllocation->MakeLost(currentFrameIndex, frameInUseCount))
        {
            it = FreeSuballocation(it);
            ++lostAllocationCount;
        }
    }
    return lostAllocationCount;
}

bool VmaDeviceMemoryBlock::CheckAllocation(
    uint32_t currentFrameIndex,
    uint32_t frameInUseCount,
    VkDeviceSize bufferImageGranularity,
    VkDeviceSize allocSize,
    VkDeviceSize allocAlignment,
    VmaSuballocationType allocType,
    VmaSuballocationList::const_iterator suballocItem,
    bool canMakeOtherLost,
    VkDeviceSize* pOffset,
    size_t* itemsToMakeLostCount,
    VkDeviceSize* pSumFreeSize,
    VkDeviceSize* pSumItemSize) const
{
    VMA_ASSERT(allocSize > 0);
    VMA_ASSERT(allocType != VMA_SUBALLOCATION_TYPE_FREE);
    VMA_ASSERT(suballocItem != m_Suballocations.cend());
    VMA_ASSERT(pOffset != VMA_NULL);
    
    *itemsToMakeLostCount = 0;
    *pSumFreeSize = 0;
    *pSumItemSize = 0;

    if(canMakeOtherLost)
    {
        if(suballocItem->type == VMA_SUBALLOCATION_TYPE_FREE)
        {
            *pSumFreeSize = suballocItem->size;
        }
        else
        {
            if(suballocItem->hAllocation->CanBecomeLost() &&
                suballocItem->hAllocation->GetLastUseFrameIndex() + frameInUseCount < currentFrameIndex)
            {
                ++*itemsToMakeLostCount;
                *pSumItemSize = suballocItem->size;
            }
            else
            {
                return false;
            }
        }

        // Remaining size is too small for this request: Early return.
        if(m_Size - suballocItem->offset < allocSize)
        {
            return false;
        }

        // Start from offset equal to beginning of this suballocation.
        *pOffset = suballocItem->offset;
    
        // Apply VMA_DEBUG_MARGIN at the beginning.
        if((VMA_DEBUG_MARGIN > 0) && suballocItem != m_Suballocations.cbegin())
        {
            *pOffset += VMA_DEBUG_MARGIN;
        }
    
        // Apply alignment.
        const VkDeviceSize alignment = VMA_MAX(allocAlignment, static_cast<VkDeviceSize>(VMA_DEBUG_ALIGNMENT));
        *pOffset = VmaAlignUp(*pOffset, alignment);

        // Check previous suballocations for BufferImageGranularity conflicts.
        // Make bigger alignment if necessary.
        if(bufferImageGranularity > 1)
        {
            bool bufferImageGranularityConflict = false;
            VmaSuballocationList::const_iterator prevSuballocItem = suballocItem;
            while(prevSuballocItem != m_Suballocations.cbegin())
            {
                --prevSuballocItem;
                const VmaSuballocation& prevSuballoc = *prevSuballocItem;
                if(VmaBlocksOnSamePage(prevSuballoc.offset, prevSuballoc.size, *pOffset, bufferImageGranularity))
                {
                    if(VmaIsBufferImageGranularityConflict(prevSuballoc.type, allocType))
                    {
                        bufferImageGranularityConflict = true;
                        break;
                    }
                }
                else
                    // Already on previous page.
                    break;
            }
            if(bufferImageGranularityConflict)
            {
                *pOffset = VmaAlignUp(*pOffset, bufferImageGranularity);
            }
        }
    
        // Now that we have final *pOffset, check if we are past suballocItem.
        // If yes, return false - this function should be called for another suballocItem as starting point.
        if(*pOffset >= suballocItem->offset + suballocItem->size)
        {
            return false;
        }
    
        // Calculate padding at the beginning based on current offset.
        const VkDeviceSize paddingBegin = *pOffset - suballocItem->offset;

        // Calculate required margin at the end if this is not last suballocation.
        VmaSuballocationList::const_iterator next = suballocItem;
        ++next;
        const VkDeviceSize requiredEndMargin =
            (next != m_Suballocations.cend()) ? VMA_DEBUG_MARGIN : 0;

        const VkDeviceSize totalSize = paddingBegin + allocSize + requiredEndMargin;
        // Another early return check.
        if(suballocItem->offset + totalSize > m_Size)
        {
            return false;
        }

        // Advance lastSuballocItem until desired size is reached.
        // Update itemsToMakeLostCount.
        VmaSuballocationList::const_iterator lastSuballocItem = suballocItem;
        if(totalSize > suballocItem->size)
        {
            VkDeviceSize remainingSize = totalSize - suballocItem->size;
            while(remainingSize > 0)
            {
                ++lastSuballocItem;
                if(lastSuballocItem == m_Suballocations.cend())
                {
                    return false;
                }
                if(lastSuballocItem->type == VMA_SUBALLOCATION_TYPE_FREE)
                {
                    *pSumFreeSize += lastSuballocItem->size;
                }
                else
                {
                    VMA_ASSERT(lastSuballocItem->hAllocation != VK_NULL_HANDLE);
                    if(lastSuballocItem->hAllocation->CanBecomeLost() &&
                        lastSuballocItem->hAllocation->GetLastUseFrameIndex() + frameInUseCount < currentFrameIndex)
                    {
                        ++*itemsToMakeLostCount;
                        *pSumItemSize += lastSuballocItem->size;
                    }
                    else
                    {
                        return false;
                    }
                }
                remainingSize = (lastSuballocItem->size < remainingSize) ?
                    remainingSize - lastSuballocItem->size : 0;
            }
        }

        // Check next suballocations for BufferImageGranularity conflicts.
        // If conflict exists, we must mark more allocations lost or fail.
        if(bufferImageGranularity > 1)
        {
            VmaSuballocationList::const_iterator nextSuballocItem = lastSuballocItem;
            ++nextSuballocItem;
            while(nextSuballocItem != m_Suballocations.cend())
            {
                const VmaSuballocation& nextSuballoc = *nextSuballocItem;
                if(VmaBlocksOnSamePage(*pOffset, allocSize, nextSuballoc.offset, bufferImageGranularity))
                {
                    if(VmaIsBufferImageGranularityConflict(allocType, nextSuballoc.type))
                    {
                        VMA_ASSERT(nextSuballoc.hAllocation != VK_NULL_HANDLE);
                        if(nextSuballoc.hAllocation->CanBecomeLost() &&
                            nextSuballoc.hAllocation->GetLastUseFrameIndex() + frameInUseCount < currentFrameIndex)
                        {
                            ++*itemsToMakeLostCount;
                        }
                        else
                        {
                            return false;
                        }
                    }
                }
                else
                {
                    // Already on next page.
                    break;
                }
                ++nextSuballocItem;
            }
        }
    }
    else
    {
        const VmaSuballocation& suballoc = *suballocItem;
        VMA_ASSERT(suballoc.type == VMA_SUBALLOCATION_TYPE_FREE);

        *pSumFreeSize = suballoc.size;

        // Size of this suballocation is too small for this request: Early return.
        if(suballoc.size < allocSize)
        {
            return false;
        }

        // Start from offset equal to beginning of this suballocation.
        *pOffset = suballoc.offset;
    
        // Apply VMA_DEBUG_MARGIN at the beginning.
        if((VMA_DEBUG_MARGIN > 0) && suballocItem != m_Suballocations.cbegin())
        {
            *pOffset += VMA_DEBUG_MARGIN;
        }
    
        // Apply alignment.
        const VkDeviceSize alignment = VMA_MAX(allocAlignment, static_cast<VkDeviceSize>(VMA_DEBUG_ALIGNMENT));
        *pOffset = VmaAlignUp(*pOffset, alignment);
    
        // Check previous suballocations for BufferImageGranularity conflicts.
        // Make bigger alignment if necessary.
        if(bufferImageGranularity > 1)
        {
            bool bufferImageGranularityConflict = false;
            VmaSuballocationList::const_iterator prevSuballocItem = suballocItem;
            while(prevSuballocItem != m_Suballocations.cbegin())
            {
                --prevSuballocItem;
                const VmaSuballocation& prevSuballoc = *prevSuballocItem;
                if(VmaBlocksOnSamePage(prevSuballoc.offset, prevSuballoc.size, *pOffset, bufferImageGranularity))
                {
                    if(VmaIsBufferImageGranularityConflict(prevSuballoc.type, allocType))
                    {
                        bufferImageGranularityConflict = true;
                        break;
                    }
                }
                else
                    // Already on previous page.
                    break;
            }
            if(bufferImageGranularityConflict)
            {
                *pOffset = VmaAlignUp(*pOffset, bufferImageGranularity);
            }
        }
    
        // Calculate padding at the beginning based on current offset.
        const VkDeviceSize paddingBegin = *pOffset - suballoc.offset;

        // Calculate required margin at the end if this is not last suballocation.
        VmaSuballocationList::const_iterator next = suballocItem;
        ++next;
        const VkDeviceSize requiredEndMargin =
            (next != m_Suballocations.cend()) ? VMA_DEBUG_MARGIN : 0;

        // Fail if requested size plus margin before and after is bigger than size of this suballocation.
        if(paddingBegin + allocSize + requiredEndMargin > suballoc.size)
        {
            return false;
        }

        // Check next suballocations for BufferImageGranularity conflicts.
        // If conflict exists, allocation cannot be made here.
        if(bufferImageGranularity > 1)
        {
            VmaSuballocationList::const_iterator nextSuballocItem = suballocItem;
            ++nextSuballocItem;
            while(nextSuballocItem != m_Suballocations.cend())
            {
                const VmaSuballocation& nextSuballoc = *nextSuballocItem;
                if(VmaBlocksOnSamePage(*pOffset, allocSize, nextSuballoc.offset, bufferImageGranularity))
                {
                    if(VmaIsBufferImageGranularityConflict(allocType, nextSuballoc.type))
                    {
                        return false;
                    }
                }
                else
                {
                    // Already on next page.
                    break;
                }
                ++nextSuballocItem;
            }
        }
    }

    // All tests passed: Success. pOffset is already filled.
    return true;
}

bool VmaDeviceMemoryBlock::IsEmpty() const
{
    return (m_Suballocations.size() == 1) && (m_FreeCount == 1);
}

void VmaDeviceMemoryBlock::Alloc(
    const VmaAllocationRequest& request,
    VmaSuballocationType type,
    VkDeviceSize allocSize,
    VmaAllocation hAllocation)
{
    VMA_ASSERT(request.item != m_Suballocations.end());
    VmaSuballocation& suballoc = *request.item;
    // Given suballocation is a free block.
    VMA_ASSERT(suballoc.type == VMA_SUBALLOCATION_TYPE_FREE);
    // Given offset is inside this suballocation.
    VMA_ASSERT(request.offset >= suballoc.offset);
    const VkDeviceSize paddingBegin = request.offset - suballoc.offset;
    VMA_ASSERT(suballoc.size >= paddingBegin + allocSize);
    const VkDeviceSize paddingEnd = suballoc.size - paddingBegin - allocSize;

    // Unregister this free suballocation from m_FreeSuballocationsBySize and update
    // it to become used.
    UnregisterFreeSuballocation(request.item);

    suballoc.offset = request.offset;
    suballoc.size = allocSize;
    suballoc.type = type;
    suballoc.hAllocation = hAllocation;

    // If there are any free bytes remaining at the end, insert new free suballocation after current one.
    if(paddingEnd)
    {
        VmaSuballocation paddingSuballoc = {};
        paddingSuballoc.offset = request.offset + allocSize;
        paddingSuballoc.size = paddingEnd;
        paddingSuballoc.type = VMA_SUBALLOCATION_TYPE_FREE;
        VmaSuballocationList::iterator next = request.item;
        ++next;
        const VmaSuballocationList::iterator paddingEndItem =
            m_Suballocations.insert(next, paddingSuballoc);
        RegisterFreeSuballocation(paddingEndItem);
    }

    // If there are any free bytes remaining at the beginning, insert new free suballocation before current one.
    if(paddingBegin)
    {
        VmaSuballocation paddingSuballoc = {};
        paddingSuballoc.offset = request.offset - paddingBegin;
        paddingSuballoc.size = paddingBegin;
        paddingSuballoc.type = VMA_SUBALLOCATION_TYPE_FREE;
        const VmaSuballocationList::iterator paddingBeginItem =
            m_Suballocations.insert(request.item, paddingSuballoc);
        RegisterFreeSuballocation(paddingBeginItem);
    }

    // Update totals.
    m_FreeCount = m_FreeCount - 1;
    if(paddingBegin > 0)
    {
        ++m_FreeCount;
    }
    if(paddingEnd > 0)
    {
        ++m_FreeCount;
    }
    m_SumFreeSize -= allocSize;
}

VmaSuballocationList::iterator VmaDeviceMemoryBlock::FreeSuballocation(VmaSuballocationList::iterator suballocItem)
{
    // Change this suballocation to be marked as free.
    VmaSuballocation& suballoc = *suballocItem;
    suballoc.type = VMA_SUBALLOCATION_TYPE_FREE;
    suballoc.hAllocation = VK_NULL_HANDLE;
    
    // Update totals.
    ++m_FreeCount;
    m_SumFreeSize += suballoc.size;

    // Merge with previous and/or next suballocation if it's also free.
    bool mergeWithNext = false;
    bool mergeWithPrev = false;
    
    VmaSuballocationList::iterator nextItem = suballocItem;
    ++nextItem;
    if((nextItem != m_Suballocations.end()) && (nextItem->type == VMA_SUBALLOCATION_TYPE_FREE))
    {
        mergeWithNext = true;
    }

    VmaSuballocationList::iterator prevItem = suballocItem;
    if(suballocItem != m_Suballocations.begin())
    {
        --prevItem;
        if(prevItem->type == VMA_SUBALLOCATION_TYPE_FREE)
        {
            mergeWithPrev = true;
        }
    }

    if(mergeWithNext)
    {
        UnregisterFreeSuballocation(nextItem);
        MergeFreeWithNext(suballocItem);
    }

    if(mergeWithPrev)
    {
        UnregisterFreeSuballocation(prevItem);
        MergeFreeWithNext(prevItem);
        RegisterFreeSuballocation(prevItem);
        return prevItem;
    }
    else
    {
        RegisterFreeSuballocation(suballocItem);
        return suballocItem;
    }
}

void VmaDeviceMemoryBlock::Free(const VmaAllocation allocation)
{
    for(VmaSuballocationList::iterator suballocItem = m_Suballocations.begin();
        suballocItem != m_Suballocations.end();
        ++suballocItem)
    {
        VmaSuballocation& suballoc = *suballocItem;
        if(suballoc.hAllocation == allocation)
        {
            FreeSuballocation(suballocItem);
            VMA_HEAVY_ASSERT(Validate());
            return;
        }
    }
    VMA_ASSERT(0 && "Not found!");
}

#if VMA_STATS_STRING_ENABLED

void VmaDeviceMemoryBlock::PrintDetailedMap(class VmaJsonWriter& json) const
{
    json.BeginObject();

    json.WriteString("TotalBytes");
    json.WriteNumber(m_Size);

    json.WriteString("UnusedBytes");
    json.WriteNumber(m_SumFreeSize);

    json.WriteString("Allocations");
    json.WriteNumber(m_Suballocations.size() - m_FreeCount);

    json.WriteString("UnusedRanges");
    json.WriteNumber(m_FreeCount);

    json.WriteString("Suballocations");
    json.BeginArray();
    size_t i = 0;
    for(VmaSuballocationList::const_iterator suballocItem = m_Suballocations.cbegin();
        suballocItem != m_Suballocations.cend();
        ++suballocItem, ++i)
    {
        json.BeginObject(true);
        
        json.WriteString("Type");
        json.WriteString(VMA_SUBALLOCATION_TYPE_NAMES[suballocItem->type]);

        json.WriteString("Size");
        json.WriteNumber(suballocItem->size);

        json.WriteString("Offset");
        json.WriteNumber(suballocItem->offset);

        json.EndObject();
    }
    json.EndArray();

    json.EndObject();
}

#endif // #if VMA_STATS_STRING_ENABLED

void VmaDeviceMemoryBlock::MergeFreeWithNext(VmaSuballocationList::iterator item)
{
    VMA_ASSERT(item != m_Suballocations.end());
    VMA_ASSERT(item->type == VMA_SUBALLOCATION_TYPE_FREE);
    
    VmaSuballocationList::iterator nextItem = item;
    ++nextItem;
    VMA_ASSERT(nextItem != m_Suballocations.end());
    VMA_ASSERT(nextItem->type == VMA_SUBALLOCATION_TYPE_FREE);

    item->size += nextItem->size;
    --m_FreeCount;
    m_Suballocations.erase(nextItem);
}

void VmaDeviceMemoryBlock::RegisterFreeSuballocation(VmaSuballocationList::iterator item)
{
    VMA_ASSERT(item->type == VMA_SUBALLOCATION_TYPE_FREE);
    VMA_ASSERT(item->size > 0);

    // You may want to enable this validation at the beginning or at the end of
    // this function, depending on what do you want to check.
    VMA_HEAVY_ASSERT(ValidateFreeSuballocationList());

    if(item->size >= VMA_MIN_FREE_SUBALLOCATION_SIZE_TO_REGISTER)
    {
        if(m_FreeSuballocationsBySize.empty())
        {
            m_FreeSuballocationsBySize.push_back(item);
        }
        else
        {
            VmaVectorInsertSorted<VmaSuballocationItemSizeLess>(m_FreeSuballocationsBySize, item);
        }
    }

    //VMA_HEAVY_ASSERT(ValidateFreeSuballocationList());
}


void VmaDeviceMemoryBlock::UnregisterFreeSuballocation(VmaSuballocationList::iterator item)
{
    VMA_ASSERT(item->type == VMA_SUBALLOCATION_TYPE_FREE);
    VMA_ASSERT(item->size > 0);

    // You may want to enable this validation at the beginning or at the end of
    // this function, depending on what do you want to check.
    VMA_HEAVY_ASSERT(ValidateFreeSuballocationList());

    if(item->size >= VMA_MIN_FREE_SUBALLOCATION_SIZE_TO_REGISTER)
    {
        VmaSuballocationList::iterator* const it = VmaBinaryFindFirstNotLess(
            m_FreeSuballocationsBySize.data(),
            m_FreeSuballocationsBySize.data() + m_FreeSuballocationsBySize.size(),
            item,
            VmaSuballocationItemSizeLess());
        for(size_t index = it - m_FreeSuballocationsBySize.data();
            index < m_FreeSuballocationsBySize.size();
            ++index)
        {
            if(m_FreeSuballocationsBySize[index] == item)
            {
                VmaVectorRemove(m_FreeSuballocationsBySize, index);
                return;
            }
            VMA_ASSERT((m_FreeSuballocationsBySize[index]->size == item->size) && "Not found.");
        }
        VMA_ASSERT(0 && "Not found.");
    }

    //VMA_HEAVY_ASSERT(ValidateFreeSuballocationList());
}

bool VmaDeviceMemoryBlock::ValidateFreeSuballocationList() const
{
    VkDeviceSize lastSize = 0;
    for(size_t i = 0, count = m_FreeSuballocationsBySize.size(); i < count; ++i)
    {
        const VmaSuballocationList::iterator it = m_FreeSuballocationsBySize[i];

        if(it->type != VMA_SUBALLOCATION_TYPE_FREE)
        {
            VMA_ASSERT(0);
            return false;
        }
        if(it->size < VMA_MIN_FREE_SUBALLOCATION_SIZE_TO_REGISTER)
        {
            VMA_ASSERT(0);
            return false;
        }
        if(it->size < lastSize)
        {
            VMA_ASSERT(0);
            return false;
        }

        lastSize = it->size;
    }
    return true;
}

static void InitStatInfo(VmaStatInfo& outInfo)
{
    memset(&outInfo, 0, sizeof(outInfo));
    outInfo.AllocationSizeMin = UINT64_MAX;
    outInfo.UnusedRangeSizeMin = UINT64_MAX;
}

static void CalcAllocationStatInfo(VmaStatInfo& outInfo, const VmaDeviceMemoryBlock& block)
{
    outInfo.BlockCount = 1;

    const uint32_t rangeCount = (uint32_t)block.m_Suballocations.size();
    outInfo.AllocationCount = rangeCount - block.m_FreeCount;
    outInfo.UnusedRangeCount = block.m_FreeCount;
    
    outInfo.UnusedBytes = block.m_SumFreeSize;
    outInfo.UsedBytes = block.m_Size - outInfo.UnusedBytes;

    outInfo.AllocationSizeMin = UINT64_MAX;
    outInfo.AllocationSizeMax = 0;
    outInfo.UnusedRangeSizeMin = UINT64_MAX;
    outInfo.UnusedRangeSizeMax = 0;

    for(VmaSuballocationList::const_iterator suballocItem = block.m_Suballocations.cbegin();
        suballocItem != block.m_Suballocations.cend();
        ++suballocItem)
    {
        const VmaSuballocation& suballoc = *suballocItem;
        if(suballoc.type != VMA_SUBALLOCATION_TYPE_FREE)
        {
            outInfo.AllocationSizeMin = VMA_MIN(outInfo.AllocationSizeMin, suballoc.size);
            outInfo.AllocationSizeMax = VMA_MAX(outInfo.AllocationSizeMax, suballoc.size);
        }
        else
        {
            outInfo.UnusedRangeSizeMin = VMA_MIN(outInfo.UnusedRangeSizeMin, suballoc.size);
            outInfo.UnusedRangeSizeMax = VMA_MAX(outInfo.UnusedRangeSizeMax, suballoc.size);
        }
    }
}

// Adds statistics srcInfo into inoutInfo, like: inoutInfo += srcInfo.
static void VmaAddStatInfo(VmaStatInfo& inoutInfo, const VmaStatInfo& srcInfo)
{
    inoutInfo.BlockCount += srcInfo.BlockCount;
    inoutInfo.AllocationCount += srcInfo.AllocationCount;
    inoutInfo.UnusedRangeCount += srcInfo.UnusedRangeCount;
    inoutInfo.UsedBytes += srcInfo.UsedBytes;
    inoutInfo.UnusedBytes += srcInfo.UnusedBytes;
    inoutInfo.AllocationSizeMin = VMA_MIN(inoutInfo.AllocationSizeMin, srcInfo.AllocationSizeMin);
    inoutInfo.AllocationSizeMax = VMA_MAX(inoutInfo.AllocationSizeMax, srcInfo.AllocationSizeMax);
    inoutInfo.UnusedRangeSizeMin = VMA_MIN(inoutInfo.UnusedRangeSizeMin, srcInfo.UnusedRangeSizeMin);
    inoutInfo.UnusedRangeSizeMax = VMA_MAX(inoutInfo.UnusedRangeSizeMax, srcInfo.UnusedRangeSizeMax);
}

static void VmaPostprocessCalcStatInfo(VmaStatInfo& inoutInfo)
{
    inoutInfo.AllocationSizeAvg = (inoutInfo.AllocationCount > 0) ?
        VmaRoundDiv<VkDeviceSize>(inoutInfo.UsedBytes, inoutInfo.AllocationCount) : 0;
    inoutInfo.UnusedRangeSizeAvg = (inoutInfo.UnusedRangeCount > 0) ?
        VmaRoundDiv<VkDeviceSize>(inoutInfo.UnusedBytes, inoutInfo.UnusedRangeCount) : 0;
}

VmaPool_T::VmaPool_T(
    VmaAllocator hAllocator,
    const VmaPoolCreateInfo& createInfo) :
    m_BlockVector(
        hAllocator,
        createInfo.memoryTypeIndex,
        (createInfo.flags & VMA_POOL_CREATE_PERSISTENT_MAP_BIT) != 0 ?
            VMA_BLOCK_VECTOR_TYPE_MAPPED : VMA_BLOCK_VECTOR_TYPE_UNMAPPED,
        createInfo.blockSize,
        createInfo.minBlockCount,
        createInfo.maxBlockCount,
        (createInfo.flags & VMA_POOL_CREATE_IGNORE_BUFFER_IMAGE_GRANULARITY_BIT) != 0 ? 1 : hAllocator->GetBufferImageGranularity(),
        createInfo.frameInUseCount,
        true) // isCustomPool
{
}

VmaPool_T::~VmaPool_T()
{
}

#if VMA_STATS_STRING_ENABLED

#endif // #if VMA_STATS_STRING_ENABLED

VmaBlockVector::VmaBlockVector(
    VmaAllocator hAllocator,
    uint32_t memoryTypeIndex,
    VMA_BLOCK_VECTOR_TYPE blockVectorType,
    VkDeviceSize preferredBlockSize,
    size_t minBlockCount,
    size_t maxBlockCount,
    VkDeviceSize bufferImageGranularity,
    uint32_t frameInUseCount,
    bool isCustomPool) :
    m_hAllocator(hAllocator),
    m_MemoryTypeIndex(memoryTypeIndex),
    m_BlockVectorType(blockVectorType),
    m_PreferredBlockSize(preferredBlockSize),
    m_MinBlockCount(minBlockCount),
    m_MaxBlockCount(maxBlockCount),
    m_BufferImageGranularity(bufferImageGranularity),
    m_FrameInUseCount(frameInUseCount),
    m_IsCustomPool(isCustomPool),
    m_Blocks(VmaStlAllocator<VmaDeviceMemoryBlock*>(hAllocator->GetAllocationCallbacks())),
    m_HasEmptyBlock(false),
    m_pDefragmentator(VMA_NULL)
{
}

VmaBlockVector::~VmaBlockVector()
{
    VMA_ASSERT(m_pDefragmentator == VMA_NULL);

    for(size_t i = m_Blocks.size(); i--; )
    {
        m_Blocks[i]->Destroy(m_hAllocator);
        vma_delete(m_hAllocator, m_Blocks[i]);
    }
}

VkResult VmaBlockVector::CreateMinBlocks()
{
    for(size_t i = 0; i < m_MinBlockCount; ++i)
    {
        VkResult res = CreateBlock(m_PreferredBlockSize, VMA_NULL);
        if(res != VK_SUCCESS)
        {
            return res;
        }
    }
    return VK_SUCCESS;
}

void VmaBlockVector::GetPoolStats(VmaPoolStats* pStats)
{
    pStats->size = 0;
    pStats->unusedSize = 0;
    pStats->allocationCount = 0;
    pStats->unusedRangeCount = 0;

    VmaMutexLock lock(m_Mutex, m_hAllocator->m_UseMutex);

    for(uint32_t blockIndex = 0; blockIndex < m_Blocks.size(); ++blockIndex)
    {
        const VmaDeviceMemoryBlock* const pBlock = m_Blocks[blockIndex];
        VMA_ASSERT(pBlock);
        VMA_HEAVY_ASSERT(pBlock->Validate());

        const uint32_t rangeCount = (uint32_t)pBlock->m_Suballocations.size();

        pStats->size += pBlock->m_Size;
        pStats->unusedSize += pBlock->m_SumFreeSize;
        pStats->allocationCount += rangeCount - pBlock->m_FreeCount;
        pStats->unusedRangeCount += pBlock->m_FreeCount;
    }
}

static const uint32_t VMA_ALLOCATION_TRY_COUNT = 32;

VkResult VmaBlockVector::Allocate(
    VmaPool hCurrentPool,
    uint32_t currentFrameIndex,
    const VkMemoryRequirements& vkMemReq,
    const VmaAllocationCreateInfo& createInfo,
    VmaSuballocationType suballocType,
    VmaAllocation* pAllocation)
{
    // Validate flags.
    if(((createInfo.flags & VMA_ALLOCATION_CREATE_PERSISTENT_MAP_BIT) != 0) !=
        (m_BlockVectorType == VMA_BLOCK_VECTOR_TYPE_MAPPED))
    {
        VMA_ASSERT(0 && "Usage of VMA_ALLOCATION_CREATE_PERSISTENT_MAP_BIT must match VMA_POOL_CREATE_PERSISTENT_MAP_BIT.");
        return VK_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    VmaMutexLock lock(m_Mutex, m_hAllocator->m_UseMutex);

    // 1. Search existing allocations. Try to allocate without making other allocations lost.
    // Forward order in m_Blocks - prefer blocks with smallest amount of free space.
    for(size_t blockIndex = 0; blockIndex < m_Blocks.size(); ++blockIndex )
    {
        VmaDeviceMemoryBlock* const pCurrBlock = m_Blocks[blockIndex];
        VMA_ASSERT(pCurrBlock);
        VmaAllocationRequest currRequest = {};
        if(pCurrBlock->CreateAllocationRequest(
            currentFrameIndex,
            m_FrameInUseCount,
            m_BufferImageGranularity,
            vkMemReq.size,
            vkMemReq.alignment,
            suballocType,
            false, // canMakeOtherLost
            &currRequest))
        {
            // Allocate from pCurrBlock.
            VMA_ASSERT(currRequest.itemsToMakeLostCount == 0);
            
            // We no longer have an empty Allocation.
            if(pCurrBlock->IsEmpty())
            {
                m_HasEmptyBlock = false;
            }
            
            *pAllocation = vma_new(m_hAllocator, VmaAllocation_T)(currentFrameIndex);
            pCurrBlock->Alloc(currRequest, suballocType, vkMemReq.size, *pAllocation);
            (*pAllocation)->InitBlockAllocation(
                hCurrentPool,
                pCurrBlock,
                currRequest.offset,
                vkMemReq.alignment,
                vkMemReq.size,
                suballocType,
                createInfo.pUserData,
                (createInfo.flags & VMA_ALLOCATION_CREATE_CAN_BECOME_LOST_BIT) != 0);
            VMA_HEAVY_ASSERT(pCurrBlock->Validate());
            VMA_DEBUG_LOG("    Returned from existing allocation #%u", (uint32_t)blockIndex);
            return VK_SUCCESS;
        }
    }

    const bool canCreateNewBlock =
        ((createInfo.flags & VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT) == 0) &&
        (m_Blocks.size() < m_MaxBlockCount);

    // 2. Try to create new block.
    if(canCreateNewBlock)
    {
        // 2.1. Start with full preferredBlockSize.
        VkDeviceSize blockSize = m_PreferredBlockSize;
        size_t newBlockIndex = 0;
        VkResult res = CreateBlock(blockSize, &newBlockIndex);
        // Allocating blocks of other sizes is allowed only in default pools.
        // In custom pools block size is fixed.
        if(res < 0 && m_IsCustomPool == false)
        {
            // 2.2. Try half the size.
            blockSize /= 2;
            if(blockSize >= vkMemReq.size)
            {
                res = CreateBlock(blockSize, &newBlockIndex);
                if(res < 0)
                {
                    // 2.3. Try quarter the size.
                    blockSize /= 2;
                    if(blockSize >= vkMemReq.size)
                    {
                        res = CreateBlock(blockSize, &newBlockIndex);
                    }
                }
            }
        }
        if(res == VK_SUCCESS)
        {
            VmaDeviceMemoryBlock* const pBlock = m_Blocks[newBlockIndex];
            VMA_ASSERT(pBlock->m_Size >= vkMemReq.size);

            // Allocate from pBlock. Because it is empty, dstAllocRequest can be trivially filled.
            VmaAllocationRequest allocRequest = {};
            allocRequest.item = pBlock->m_Suballocations.begin();
            allocRequest.offset = 0;
            *pAllocation = vma_new(m_hAllocator, VmaAllocation_T)(currentFrameIndex);
            pBlock->Alloc(allocRequest, suballocType, vkMemReq.size, *pAllocation);
            (*pAllocation)->InitBlockAllocation(
                hCurrentPool,
                pBlock,
                allocRequest.offset,
                vkMemReq.alignment,
                vkMemReq.size,
                suballocType,
                createInfo.pUserData,
                (createInfo.flags & VMA_ALLOCATION_CREATE_CAN_BECOME_LOST_BIT) != 0);
            VMA_HEAVY_ASSERT(pBlock->Validate());
            VMA_DEBUG_LOG("    Created new allocation Size=%llu", allocInfo.allocationSize);

            return VK_SUCCESS;
        }
    }

    const bool canMakeOtherLost = (createInfo.flags & VMA_ALLOCATION_CREATE_CAN_MAKE_OTHER_LOST_BIT) != 0;

    // 3. Try to allocate from existing blocks with making other allocations lost.
    if(canMakeOtherLost)
    {
        uint32_t tryIndex = 0;
        for(; tryIndex < VMA_ALLOCATION_TRY_COUNT; ++tryIndex)
        {
            VmaDeviceMemoryBlock* pBestRequestBlock = VMA_NULL;
            VmaAllocationRequest bestRequest = {};
            VkDeviceSize bestRequestCost = VK_WHOLE_SIZE;

            // 1. Search existing allocations.
            // Forward order in m_Blocks - prefer blocks with smallest amount of free space.
            for(size_t blockIndex = 0; blockIndex < m_Blocks.size(); ++blockIndex )
            {
                VmaDeviceMemoryBlock* const pCurrBlock = m_Blocks[blockIndex];
                VMA_ASSERT(pCurrBlock);
                VmaAllocationRequest currRequest = {};
                if(pCurrBlock->CreateAllocationRequest(
                    currentFrameIndex,
                    m_FrameInUseCount,
                    m_BufferImageGranularity,
                    vkMemReq.size,
                    vkMemReq.alignment,
                    suballocType,
                    canMakeOtherLost,
                    &currRequest))
                {
                    const VkDeviceSize currRequestCost = currRequest.CalcCost();
                    if(pBestRequestBlock == VMA_NULL ||
                        currRequestCost < bestRequestCost)
                    {
                        pBestRequestBlock = pCurrBlock;
                        bestRequest = currRequest;
                        bestRequestCost = currRequestCost;

                        if(bestRequestCost == 0)
                        {
                            break;
                        }
                    }
                }
            }

            if(pBestRequestBlock != VMA_NULL)
            {
                if(pBestRequestBlock->MakeRequestedAllocationsLost(
                    currentFrameIndex,
                    m_FrameInUseCount,
                    &bestRequest))
                {
                    // We no longer have an empty Allocation.
                    if(pBestRequestBlock->IsEmpty())
                    {
                        m_HasEmptyBlock = false;
                    }
                    // Allocate from this pBlock.
                    *pAllocation = vma_new(m_hAllocator, VmaAllocation_T)(currentFrameIndex);
                    pBestRequestBlock->Alloc(bestRequest, suballocType, vkMemReq.size, *pAllocation);
                    (*pAllocation)->InitBlockAllocation(
                        hCurrentPool,
                        pBestRequestBlock,
                        bestRequest.offset,
                        vkMemReq.alignment,
                        vkMemReq.size,
                        suballocType,
                        createInfo.pUserData,
                        (createInfo.flags & VMA_ALLOCATION_CREATE_CAN_BECOME_LOST_BIT) != 0);
                    VMA_HEAVY_ASSERT(pBlock->Validate());
                    VMA_DEBUG_LOG("    Returned from existing allocation #%u", (uint32_t)blockIndex);
                    return VK_SUCCESS;
                }
                // else: Some allocations must have been touched while we are here. Next try.
            }
            else
            {
                // Could not find place in any of the blocks - break outer loop.
                break;
            }
        }
        /* Maximum number of tries exceeded - a very unlike event when many other
        threads are simultaneously touching allocations making it impossible to make
        lost at the same time as we try to allocate. */
        if(tryIndex == VMA_ALLOCATION_TRY_COUNT)
        {
            return VK_ERROR_TOO_MANY_OBJECTS;
        }
    }

    return VK_ERROR_OUT_OF_DEVICE_MEMORY;
}

void VmaBlockVector::Free(
    VmaAllocation hAllocation)
{
    VmaDeviceMemoryBlock* pBlockToDelete = VMA_NULL;

    // Scope for lock.
    {
        VmaMutexLock lock(m_Mutex, m_hAllocator->m_UseMutex);

        VmaDeviceMemoryBlock* pBlock = hAllocation->GetBlock();

        pBlock->Free(hAllocation);
        VMA_HEAVY_ASSERT(pBlock->Validate());

        VMA_DEBUG_LOG("  Freed from MemoryTypeIndex=%u", memTypeIndex);

        // pBlock became empty after this deallocation.
        if(pBlock->IsEmpty())
        {
            // Already has empty Allocation. We don't want to have two, so delete this one.
            if(m_HasEmptyBlock && m_Blocks.size() > m_MinBlockCount)
            {
                pBlockToDelete = pBlock;
                Remove(pBlock);
            }
            // We now have first empty Allocation.
            else
            {
                m_HasEmptyBlock = true;
            }
        }
        // Must be called after srcBlockIndex is used, because later it may become invalid!
        IncrementallySortBlocks();
    }

    // Destruction of a free Allocation. Deferred until this point, outside of mutex
    // lock, for performance reason.
    if(pBlockToDelete != VMA_NULL)
    {
        VMA_DEBUG_LOG("    Deleted empty allocation");
        pBlockToDelete->Destroy(m_hAllocator);
        vma_delete(m_hAllocator, pBlockToDelete);
    }
}

void VmaBlockVector::Remove(VmaDeviceMemoryBlock* pBlock)
{
    for(uint32_t blockIndex = 0; blockIndex < m_Blocks.size(); ++blockIndex)
    {
        if(m_Blocks[blockIndex] == pBlock)
        {
            VmaVectorRemove(m_Blocks, blockIndex);
            return;
        }
    }
    VMA_ASSERT(0);
}

void VmaBlockVector::IncrementallySortBlocks()
{
    // Bubble sort only until first swap.
    for(size_t i = 1; i < m_Blocks.size(); ++i)
    {
        if(m_Blocks[i - 1]->m_SumFreeSize > m_Blocks[i]->m_SumFreeSize)
        {
            VMA_SWAP(m_Blocks[i - 1], m_Blocks[i]);
            return;
        }
    }
}

VkResult VmaBlockVector::CreateBlock(VkDeviceSize blockSize, size_t* pNewBlockIndex)
{
    VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.memoryTypeIndex = m_MemoryTypeIndex;
    allocInfo.allocationSize = blockSize;
    const VkDevice hDevice = m_hAllocator->m_hDevice;
    VkDeviceMemory mem = VK_NULL_HANDLE;
    VkResult res = vkAllocateMemory(hDevice, &allocInfo, m_hAllocator->GetAllocationCallbacks(), &mem);
    if(res < 0)
    {
        return res;
    }

    // New VkDeviceMemory successfully created.

    // Map memory if needed.
    void* pMappedData = VMA_NULL;
    const bool persistentMap = (m_BlockVectorType == VMA_BLOCK_VECTOR_TYPE_MAPPED);
    if(persistentMap && m_hAllocator->m_UnmapPersistentlyMappedMemoryCounter == 0)
    {
        res = vkMapMemory(hDevice, mem, 0, VK_WHOLE_SIZE, 0, &pMappedData);
        if(res < 0)
        {
            VMA_DEBUG_LOG("    vkMapMemory FAILED");
            vkFreeMemory(hDevice, mem, m_hAllocator->GetAllocationCallbacks());
            return res;
        }
    }

    // Callback.
    if(m_hAllocator->m_DeviceMemoryCallbacks.pfnAllocate != VMA_NULL)
    {
        (*m_hAllocator->m_DeviceMemoryCallbacks.pfnAllocate)(m_hAllocator, m_MemoryTypeIndex, mem, allocInfo.allocationSize);
    }

    // Create new Allocation for it.
    VmaDeviceMemoryBlock* const pBlock = vma_new(m_hAllocator, VmaDeviceMemoryBlock)(m_hAllocator);
    pBlock->Init(
        m_MemoryTypeIndex,
        (VMA_BLOCK_VECTOR_TYPE)m_BlockVectorType,
        mem,
        allocInfo.allocationSize,
        persistentMap,
        pMappedData);

    m_Blocks.push_back(pBlock);
    if(pNewBlockIndex != VMA_NULL)
    {
        *pNewBlockIndex = m_Blocks.size() - 1;
    }

    return VK_SUCCESS;
}

#if VMA_STATS_STRING_ENABLED

void VmaBlockVector::PrintDetailedMap(class VmaJsonWriter& json)
{
    VmaMutexLock lock(m_Mutex, m_hAllocator->m_UseMutex);

    json.BeginObject();

    if(m_IsCustomPool)
    {
        json.WriteString("MemoryTypeIndex");
        json.WriteNumber(m_MemoryTypeIndex);

        if(m_BlockVectorType == VMA_BLOCK_VECTOR_TYPE_MAPPED)
        {
            json.WriteString("Mapped");
            json.WriteBool(true);
        }

        json.WriteString("BlockSize");
        json.WriteNumber(m_PreferredBlockSize);

        json.WriteString("BlockCount");
        json.BeginObject(true);
        if(m_MinBlockCount > 0)
        {
            json.WriteString("Min");
            json.WriteNumber(m_MinBlockCount);
        }
        if(m_MaxBlockCount < SIZE_MAX)
        {
            json.WriteString("Max");
            json.WriteNumber(m_MaxBlockCount);
        }
        json.WriteString("Cur");
        json.WriteNumber(m_Blocks.size());
        json.EndObject();

        if(m_FrameInUseCount > 0)
        {
            json.WriteString("FrameInUseCount");
            json.WriteNumber(m_FrameInUseCount);
        }
    }
    else
    {
        json.WriteString("PreferredBlockSize");
        json.WriteNumber(m_PreferredBlockSize);
    }

    json.WriteString("Blocks");
    json.BeginArray();
    for(size_t i = 0; i < m_Blocks.size(); ++i)
    {
        m_Blocks[i]->PrintDetailedMap(json);
    }
    json.EndArray();

    json.EndObject();
}

#endif // #if VMA_STATS_STRING_ENABLED

void VmaBlockVector::UnmapPersistentlyMappedMemory()
{
    VmaMutexLock lock(m_Mutex, m_hAllocator->m_UseMutex);

    for(size_t i = m_Blocks.size(); i--; )
    {
        VmaDeviceMemoryBlock* pBlock = m_Blocks[i];
        if(pBlock->m_pMappedData != VMA_NULL)
        {
            VMA_ASSERT(pBlock->m_PersistentMap != false);
            vkUnmapMemory(m_hAllocator->m_hDevice, pBlock->m_hMemory);
            pBlock->m_pMappedData = VMA_NULL;
        }
    }
}

VkResult VmaBlockVector::MapPersistentlyMappedMemory()
{
    VmaMutexLock lock(m_Mutex, m_hAllocator->m_UseMutex);

    VkResult finalResult = VK_SUCCESS;
    for(size_t i = 0, count = m_Blocks.size(); i < count; ++i)
    {
        VmaDeviceMemoryBlock* pBlock = m_Blocks[i];
        if(pBlock->m_PersistentMap)
        {
            VMA_ASSERT(pBlock->m_pMappedData == nullptr);
            VkResult localResult = vkMapMemory(m_hAllocator->m_hDevice, pBlock->m_hMemory, 0, VK_WHOLE_SIZE, 0, &pBlock->m_pMappedData);
            if(localResult != VK_SUCCESS)
            {
                finalResult = localResult;
            }
        }
    }
    return finalResult;
}

VmaDefragmentator* VmaBlockVector::EnsureDefragmentator(
    VkDevice hDevice,
    const VkAllocationCallbacks* pAllocationCallbacks,
    uint32_t currentFrameIndex)
{
    if(m_pDefragmentator == VMA_NULL)
    {
        m_pDefragmentator = vma_new(m_hAllocator, VmaDefragmentator)(
            hDevice,
            pAllocationCallbacks,
            this,
            currentFrameIndex);
    }

    return m_pDefragmentator;
}

VkResult VmaBlockVector::Defragment(
    VmaDefragmentationStats* pDefragmentationStats,
    VkDeviceSize& maxBytesToMove,
    uint32_t& maxAllocationsToMove)
{
    if(m_pDefragmentator == VMA_NULL)
    {
        return VK_SUCCESS;
    }

    VmaMutexLock lock(m_Mutex, m_hAllocator->m_UseMutex);

    // Defragment.
    VkResult result = m_pDefragmentator->Defragment(maxBytesToMove, maxAllocationsToMove);

    // Accumulate statistics.
    if(pDefragmentationStats != VMA_NULL)
    {
        const VkDeviceSize bytesMoved = m_pDefragmentator->GetBytesMoved();
        const uint32_t allocationsMoved = m_pDefragmentator->GetAllocationsMoved();
        pDefragmentationStats->bytesMoved += bytesMoved;
        pDefragmentationStats->allocationsMoved += allocationsMoved;
        VMA_ASSERT(bytesMoved <= maxBytesToMove);
        VMA_ASSERT(allocationsMoved <= maxAllocationsToMove);
        maxBytesToMove -= bytesMoved;
        maxAllocationsToMove -= allocationsMoved;
    }
    
    // Free empty blocks.
    m_HasEmptyBlock = false;
    for(size_t blockIndex = m_Blocks.size(); blockIndex--; )
    {
        VmaDeviceMemoryBlock* pBlock = m_Blocks[blockIndex];
        if(pBlock->IsEmpty())
        {
            if(m_Blocks.size() > m_MinBlockCount)
            {
                if(pDefragmentationStats != VMA_NULL)
                {
                    ++pDefragmentationStats->deviceMemoryBlocksFreed;
                    pDefragmentationStats->bytesFreed += pBlock->m_Size;
                }

                VmaVectorRemove(m_Blocks, blockIndex);
                pBlock->Destroy(m_hAllocator);
                vma_delete(m_hAllocator, pBlock);
            }
            else
            {
                m_HasEmptyBlock = true;
            }
        }
    }

    return result;
}

void VmaBlockVector::DestroyDefragmentator()
{
    if(m_pDefragmentator != VMA_NULL)
    {
        vma_delete(m_hAllocator, m_pDefragmentator);
        m_pDefragmentator = VMA_NULL;
    }
}

void VmaBlockVector::MakePoolAllocationsLost(
    uint32_t currentFrameIndex,
    size_t* pLostAllocationCount)
{
    VmaMutexLock lock(m_Mutex, m_hAllocator->m_UseMutex);

    for(uint32_t blockIndex = 0; blockIndex < m_Blocks.size(); ++blockIndex)
    {
        VmaDeviceMemoryBlock* const pBlock = m_Blocks[blockIndex];
        VMA_ASSERT(pBlock);
        pBlock->MakeAllocationsLost(currentFrameIndex, m_FrameInUseCount);
    }
}

void VmaBlockVector::AddStats(VmaStats* pStats)
{
    const uint32_t memTypeIndex = m_MemoryTypeIndex;
    const uint32_t memHeapIndex = m_hAllocator->MemoryTypeIndexToHeapIndex(memTypeIndex);

    VmaMutexLock lock(m_Mutex, m_hAllocator->m_UseMutex);

    for(uint32_t blockIndex = 0; blockIndex < m_Blocks.size(); ++blockIndex)
    {
        const VmaDeviceMemoryBlock* const pBlock = m_Blocks[blockIndex];
        VMA_ASSERT(pBlock);
        VMA_HEAVY_ASSERT(pBlock->Validate());
        VmaStatInfo allocationStatInfo;
        CalcAllocationStatInfo(allocationStatInfo, *pBlock);
        VmaAddStatInfo(pStats->total, allocationStatInfo);
        VmaAddStatInfo(pStats->memoryType[memTypeIndex], allocationStatInfo);
        VmaAddStatInfo(pStats->memoryHeap[memHeapIndex], allocationStatInfo);
    }
}

////////////////////////////////////////////////////////////////////////////////
// VmaDefragmentator members definition

VmaDefragmentator::VmaDefragmentator(
    VkDevice hDevice,
    const VkAllocationCallbacks* pAllocationCallbacks,
    VmaBlockVector* pBlockVector,
    uint32_t currentFrameIndex) :
    m_hDevice(hDevice),
    m_pAllocationCallbacks(pAllocationCallbacks),
    m_pBlockVector(pBlockVector),
    m_CurrentFrameIndex(currentFrameIndex),
    m_BytesMoved(0),
    m_AllocationsMoved(0),
    m_Allocations(VmaStlAllocator<AllocationInfo>(pAllocationCallbacks)),
    m_Blocks(VmaStlAllocator<BlockInfo*>(pAllocationCallbacks))
{
}

VmaDefragmentator::~VmaDefragmentator()
{
    for(size_t i = m_Blocks.size(); i--; )
    {
        vma_delete(m_pAllocationCallbacks, m_Blocks[i]);
    }
}

void VmaDefragmentator::AddAllocation(VmaAllocation hAlloc, VkBool32* pChanged)
{
    AllocationInfo allocInfo;
    allocInfo.m_hAllocation = hAlloc;
    allocInfo.m_pChanged = pChanged;
    m_Allocations.push_back(allocInfo);
}

VkResult VmaDefragmentator::DefragmentRound(
    VkDeviceSize maxBytesToMove,
    uint32_t maxAllocationsToMove)
{
    if(m_Blocks.empty())
    {
        return VK_SUCCESS;
    }

    size_t srcBlockIndex = m_Blocks.size() - 1;
    size_t srcAllocIndex = SIZE_MAX;
    for(;;)
    {
        // 1. Find next allocation to move.
        // 1.1. Start from last to first m_Blocks - they are sorted from most "destination" to most "source".
        // 1.2. Then start from last to first m_Allocations - they are sorted from largest to smallest.
        while(srcAllocIndex >= m_Blocks[srcBlockIndex]->m_Allocations.size())
        {
            if(m_Blocks[srcBlockIndex]->m_Allocations.empty())
            {
                // Finished: no more allocations to process.
                if(srcBlockIndex == 0)
                {
                    return VK_SUCCESS;
                }
                else
                {
                    --srcBlockIndex;
                    srcAllocIndex = SIZE_MAX;
                }
            }
            else
            {
                srcAllocIndex = m_Blocks[srcBlockIndex]->m_Allocations.size() - 1;
            }
        }
        
        BlockInfo* pSrcBlockInfo = m_Blocks[srcBlockIndex];
        AllocationInfo& allocInfo = pSrcBlockInfo->m_Allocations[srcAllocIndex];

        const VkDeviceSize size = allocInfo.m_hAllocation->GetSize();
        const VkDeviceSize srcOffset = allocInfo.m_hAllocation->GetOffset();
        const VkDeviceSize alignment = allocInfo.m_hAllocation->GetAlignment();
        const VmaSuballocationType suballocType = allocInfo.m_hAllocation->GetSuballocationType();

        // 2. Try to find new place for this allocation in preceding or current block.
        for(size_t dstBlockIndex = 0; dstBlockIndex <= srcBlockIndex; ++dstBlockIndex)
        {
            BlockInfo* pDstBlockInfo = m_Blocks[dstBlockIndex];
            VmaAllocationRequest dstAllocRequest;
            if(pDstBlockInfo->m_pBlock->CreateAllocationRequest(
                m_CurrentFrameIndex,
                m_pBlockVector->GetFrameInUseCount(),
                m_pBlockVector->GetBufferImageGranularity(),
                size,
                alignment,
                suballocType,
                false, // canMakeOtherLost
                &dstAllocRequest) &&
            MoveMakesSense(
                dstBlockIndex, dstAllocRequest.offset, srcBlockIndex, srcOffset))
            {
                VMA_ASSERT(dstAllocRequest.itemsToMakeLostCount == 0);

                // Reached limit on number of allocations or bytes to move.
                if((m_AllocationsMoved + 1 > maxAllocationsToMove) ||
                    (m_BytesMoved + size > maxBytesToMove))
                {
                    return VK_INCOMPLETE;
                }

                void* pDstMappedData = VMA_NULL;
                VkResult res = pDstBlockInfo->EnsureMapping(m_hDevice, &pDstMappedData);
                if(res != VK_SUCCESS)
                {
                    return res;
                }

                void* pSrcMappedData = VMA_NULL;
                res = pSrcBlockInfo->EnsureMapping(m_hDevice, &pSrcMappedData);
                if(res != VK_SUCCESS)
                {
                    return res;
                }
                
                // THE PLACE WHERE ACTUAL DATA COPY HAPPENS.
                memcpy(
                    reinterpret_cast<char*>(pDstMappedData) + dstAllocRequest.offset,
                    reinterpret_cast<char*>(pSrcMappedData) + srcOffset,
                    static_cast<size_t>(size));
                
                pDstBlockInfo->m_pBlock->Alloc(dstAllocRequest, suballocType, size, allocInfo.m_hAllocation);
                pSrcBlockInfo->m_pBlock->Free(allocInfo.m_hAllocation);
                
                allocInfo.m_hAllocation->ChangeBlockAllocation(pDstBlockInfo->m_pBlock, dstAllocRequest.offset);
                
                if(allocInfo.m_pChanged != VMA_NULL)
                {
                    *allocInfo.m_pChanged = VK_TRUE;
                }

                ++m_AllocationsMoved;
                m_BytesMoved += size;

                VmaVectorRemove(pSrcBlockInfo->m_Allocations, srcAllocIndex);

                break;
            }
        }

        // If not processed, this allocInfo remains in pBlockInfo->m_Allocations for next round.

        if(srcAllocIndex > 0)
        {
            --srcAllocIndex;
        }
        else
        {
            if(srcBlockIndex > 0)
            {
                --srcBlockIndex;
                srcAllocIndex = SIZE_MAX;
            }
            else
            {
                return VK_SUCCESS;
            }
        }
    }
}

VkResult VmaDefragmentator::Defragment(
    VkDeviceSize maxBytesToMove,
    uint32_t maxAllocationsToMove)
{
    if(m_Allocations.empty())
    {
        return VK_SUCCESS;
    }

    // Create block info for each block.
    const size_t blockCount = m_pBlockVector->m_Blocks.size();
    for(size_t blockIndex = 0; blockIndex < blockCount; ++blockIndex)
    {
        BlockInfo* pBlockInfo = vma_new(m_pAllocationCallbacks, BlockInfo)(m_pAllocationCallbacks);
        pBlockInfo->m_pBlock = m_pBlockVector->m_Blocks[blockIndex];
        m_Blocks.push_back(pBlockInfo);
    }

    // Sort them by m_pBlock pointer value.
    VMA_SORT(m_Blocks.begin(), m_Blocks.end(), BlockPointerLess());

    // Move allocation infos from m_Allocations to appropriate m_Blocks[memTypeIndex].m_Allocations.
    for(size_t blockIndex = 0, allocCount = m_Allocations.size(); blockIndex < allocCount; ++blockIndex)
    {
        AllocationInfo& allocInfo = m_Allocations[blockIndex];
        // Now as we are inside VmaBlockVector::m_Mutex, we can make final check if this allocation was not lost.
        if(allocInfo.m_hAllocation->GetLastUseFrameIndex() != VMA_FRAME_INDEX_LOST)
        {
            VmaDeviceMemoryBlock* pBlock = allocInfo.m_hAllocation->GetBlock();
            BlockInfoVector::iterator it = VmaBinaryFindFirstNotLess(m_Blocks.begin(), m_Blocks.end(), pBlock, BlockPointerLess());
            if(it != m_Blocks.end() && (*it)->m_pBlock == pBlock)
            {
                (*it)->m_Allocations.push_back(allocInfo);
            }
            else
            {
                VMA_ASSERT(0);
            }
        }
    }
    m_Allocations.clear();

    for(size_t blockIndex = 0; blockIndex < blockCount; ++blockIndex)
    {
        BlockInfo* pBlockInfo = m_Blocks[blockIndex];
        pBlockInfo->CalcHasNonMovableAllocations();
        pBlockInfo->SortAllocationsBySizeDescecnding();
    }

    // Sort m_Blocks this time by the main criterium, from most "destination" to most "source" blocks.
    VMA_SORT(m_Blocks.begin(), m_Blocks.end(), BlockInfoCompareMoveDestination());

    // Execute defragmentation rounds (the main part).
    VkResult result = VK_SUCCESS;
    for(size_t round = 0; (round < 2) && (result == VK_SUCCESS); ++round)
    {
        result = DefragmentRound(maxBytesToMove, maxAllocationsToMove);
    }

    // Unmap blocks that were mapped for defragmentation.
    for(size_t blockIndex = 0; blockIndex < blockCount; ++blockIndex)
    {
        m_Blocks[blockIndex]->Unmap(m_hDevice);
    }

    return result;
}

bool VmaDefragmentator::MoveMakesSense(
        size_t dstBlockIndex, VkDeviceSize dstOffset,
        size_t srcBlockIndex, VkDeviceSize srcOffset)
{
    if(dstBlockIndex < srcBlockIndex)
    {
        return true;
    }
    if(dstBlockIndex > srcBlockIndex)
    {
        return false;
    }
    if(dstOffset < srcOffset)
    {
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
// VmaAllocator_T

VmaAllocator_T::VmaAllocator_T(const VmaAllocatorCreateInfo* pCreateInfo) :
    m_UseMutex((pCreateInfo->flags & VMA_ALLOCATOR_EXTERNALLY_SYNCHRONIZED_BIT) == 0),
    m_PhysicalDevice(pCreateInfo->physicalDevice),
    m_hDevice(pCreateInfo->device),
    m_AllocationCallbacksSpecified(pCreateInfo->pAllocationCallbacks != VMA_NULL),
    m_AllocationCallbacks(pCreateInfo->pAllocationCallbacks ?
        *pCreateInfo->pAllocationCallbacks : VmaEmptyAllocationCallbacks),
    m_UnmapPersistentlyMappedMemoryCounter(0),
    m_PreferredLargeHeapBlockSize(0),
    m_PreferredSmallHeapBlockSize(0),
    m_CurrentFrameIndex(0),
    m_Pools(VmaStlAllocator<VmaPool>(GetAllocationCallbacks()))
{
    VMA_ASSERT(pCreateInfo->physicalDevice && pCreateInfo->device);    

    memset(&m_DeviceMemoryCallbacks, 0 ,sizeof(m_DeviceMemoryCallbacks));
    memset(&m_MemProps, 0, sizeof(m_MemProps));
    memset(&m_PhysicalDeviceProperties, 0, sizeof(m_PhysicalDeviceProperties));
        
    memset(&m_pBlockVectors, 0, sizeof(m_pBlockVectors));
    memset(&m_pOwnAllocations, 0, sizeof(m_pOwnAllocations));

    if(pCreateInfo->pDeviceMemoryCallbacks != VMA_NULL)
    {
        m_DeviceMemoryCallbacks.pfnAllocate = pCreateInfo->pDeviceMemoryCallbacks->pfnAllocate;
        m_DeviceMemoryCallbacks.pfnFree = pCreateInfo->pDeviceMemoryCallbacks->pfnFree;
    }

    vkGetPhysicalDeviceProperties(m_PhysicalDevice, &m_PhysicalDeviceProperties);
    vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &m_MemProps);

    m_PreferredLargeHeapBlockSize = (pCreateInfo->preferredLargeHeapBlockSize != 0) ?
        pCreateInfo->preferredLargeHeapBlockSize : static_cast<VkDeviceSize>(VMA_DEFAULT_LARGE_HEAP_BLOCK_SIZE);
    m_PreferredSmallHeapBlockSize = (pCreateInfo->preferredSmallHeapBlockSize != 0) ?
        pCreateInfo->preferredSmallHeapBlockSize : static_cast<VkDeviceSize>(VMA_DEFAULT_SMALL_HEAP_BLOCK_SIZE);

    for(uint32_t memTypeIndex = 0; memTypeIndex < GetMemoryTypeCount(); ++memTypeIndex)
    {
        const VkDeviceSize preferredBlockSize = CalcPreferredBlockSize(memTypeIndex);

        for(size_t blockVectorTypeIndex = 0; blockVectorTypeIndex < VMA_BLOCK_VECTOR_TYPE_COUNT; ++blockVectorTypeIndex)
        {
            m_pBlockVectors[memTypeIndex][blockVectorTypeIndex] = vma_new(this, VmaBlockVector)(
                this,
                memTypeIndex,
                static_cast<VMA_BLOCK_VECTOR_TYPE>(blockVectorTypeIndex),
                preferredBlockSize,
                0,
                SIZE_MAX,
                GetBufferImageGranularity(),
                pCreateInfo->frameInUseCount,
                false); // isCustomPool
            // No need to call m_pBlockVectors[memTypeIndex][blockVectorTypeIndex]->CreateMinBlocks here,
            // becase minBlockCount is 0.
            m_pOwnAllocations[memTypeIndex][blockVectorTypeIndex] = vma_new(this, AllocationVectorType)(VmaStlAllocator<VmaAllocation>(GetAllocationCallbacks()));
        }
    }
}

VmaAllocator_T::~VmaAllocator_T()
{
    VMA_ASSERT(m_Pools.empty());

    for(size_t i = GetMemoryTypeCount(); i--; )
    {
        for(size_t j = VMA_BLOCK_VECTOR_TYPE_COUNT; j--; )
        {
            vma_delete(this, m_pOwnAllocations[i][j]);
            vma_delete(this, m_pBlockVectors[i][j]);
        }
    }
}

VkDeviceSize VmaAllocator_T::CalcPreferredBlockSize(uint32_t memTypeIndex)
{
    const VkDeviceSize heapSize = m_MemProps.memoryHeaps[MemoryTypeIndexToHeapIndex(memTypeIndex)].size;
    return (heapSize <= VMA_SMALL_HEAP_MAX_SIZE) ?
        m_PreferredSmallHeapBlockSize : m_PreferredLargeHeapBlockSize;
}

VkResult VmaAllocator_T::AllocateMemoryOfType(
    const VkMemoryRequirements& vkMemReq,
    const VmaAllocationCreateInfo& createInfo,
    uint32_t memTypeIndex,
    VmaSuballocationType suballocType,
    VmaAllocation* pAllocation)
{
    VMA_ASSERT(pAllocation != VMA_NULL);
    VMA_DEBUG_LOG("  AllocateMemory: MemoryTypeIndex=%u, Size=%llu", memTypeIndex, vkMemReq.size);

    uint32_t blockVectorType = VmaAllocationCreateFlagsToBlockVectorType(createInfo.flags);
    VmaBlockVector* const blockVector = m_pBlockVectors[memTypeIndex][blockVectorType];
    VMA_ASSERT(blockVector);

    const VkDeviceSize preferredBlockSize = blockVector->GetPreferredBlockSize();
    // Heuristics: Allocate own memory if requested size if greater than half of preferred block size.
    const bool ownMemory =
        (createInfo.flags & VMA_ALLOCATION_CREATE_OWN_MEMORY_BIT) != 0 ||
        VMA_DEBUG_ALWAYS_OWN_MEMORY ||
        ((createInfo.flags & VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT) == 0 &&
            vkMemReq.size > preferredBlockSize / 2);

    if(ownMemory)
    {
        if((createInfo.flags & VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT) != 0)
        {
            return VK_ERROR_OUT_OF_DEVICE_MEMORY;
        }
        else
        {
            return AllocateOwnMemory(
                vkMemReq.size,
                suballocType,
                memTypeIndex,
                (createInfo.flags & VMA_ALLOCATION_CREATE_PERSISTENT_MAP_BIT) != 0,
                createInfo.pUserData,
                pAllocation);
        }
    }
    else
    {
        VkResult res = blockVector->Allocate(
            VK_NULL_HANDLE, // hCurrentPool
            m_CurrentFrameIndex.load(),
            vkMemReq,
            createInfo,
            suballocType,
            pAllocation);
        if(res == VK_SUCCESS)
        {
            return res;
        }

        // 5. Try own memory.
        res = AllocateOwnMemory(
            vkMemReq.size,
            suballocType,
            memTypeIndex,
            (createInfo.flags & VMA_ALLOCATION_CREATE_PERSISTENT_MAP_BIT) != 0,
            createInfo.pUserData,
            pAllocation);
        if(res == VK_SUCCESS)
        {
            // Succeeded: AllocateOwnMemory function already filld pMemory, nothing more to do here.
            VMA_DEBUG_LOG("    Allocated as OwnMemory");
            return VK_SUCCESS;
        }
        else
        {
            // Everything failed: Return error code.
            VMA_DEBUG_LOG("    vkAllocateMemory FAILED");
            return res;
        }
    }
}

VkResult VmaAllocator_T::AllocateOwnMemory(
    VkDeviceSize size,
    VmaSuballocationType suballocType,
    uint32_t memTypeIndex,
    bool map,
    void* pUserData,
    VmaAllocation* pAllocation)
{
    VMA_ASSERT(pAllocation);

    VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.memoryTypeIndex = memTypeIndex;
    allocInfo.allocationSize = size;

    // Allocate VkDeviceMemory.
    VkDeviceMemory hMemory = VK_NULL_HANDLE;
    VkResult res = vkAllocateMemory(m_hDevice, &allocInfo, GetAllocationCallbacks(), &hMemory);
    if(res < 0)
    {
        VMA_DEBUG_LOG("    vkAllocateMemory FAILED");
        return res;
    }

    void* pMappedData = nullptr;
    if(map)
    {
        if(m_UnmapPersistentlyMappedMemoryCounter == 0)
        {
            res = vkMapMemory(m_hDevice, hMemory, 0, VK_WHOLE_SIZE, 0, &pMappedData);
            if(res < 0)
            {
                VMA_DEBUG_LOG("    vkMapMemory FAILED");
                vkFreeMemory(m_hDevice, hMemory, GetAllocationCallbacks());
                return res;
            }
        }
    }

    // Callback.
    if(m_DeviceMemoryCallbacks.pfnAllocate != VMA_NULL)
    {
        (*m_DeviceMemoryCallbacks.pfnAllocate)(this, memTypeIndex, hMemory, size);
    }

    *pAllocation = vma_new(this, VmaAllocation_T)(m_CurrentFrameIndex.load());
    (*pAllocation)->InitOwnAllocation(memTypeIndex, hMemory, suballocType, map, pMappedData, size, pUserData);

    // Register it in m_pOwnAllocations.
    {
        VmaMutexLock lock(m_OwnAllocationsMutex[memTypeIndex], m_UseMutex);
        AllocationVectorType* pOwnAllocations = m_pOwnAllocations[memTypeIndex][map ? VMA_BLOCK_VECTOR_TYPE_MAPPED : VMA_BLOCK_VECTOR_TYPE_UNMAPPED];
        VMA_ASSERT(pOwnAllocations);
        VmaVectorInsertSorted<VmaPointerLess>(*pOwnAllocations, *pAllocation);
    }

    VMA_DEBUG_LOG("    Allocated OwnMemory MemoryTypeIndex=#%u", memTypeIndex);

    return VK_SUCCESS;
}

VkResult VmaAllocator_T::AllocateMemory(
    const VkMemoryRequirements& vkMemReq,
    const VmaAllocationCreateInfo& createInfo,
    VmaSuballocationType suballocType,
    VmaAllocation* pAllocation)
{
    if((createInfo.flags & VMA_ALLOCATION_CREATE_OWN_MEMORY_BIT) != 0 &&
        (createInfo.flags & VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT) != 0)
    {
        VMA_ASSERT(0 && "Specifying VMA_ALLOCATION_CREATE_OWN_MEMORY_BIT together with VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT makes no sense.");
        return VK_ERROR_OUT_OF_DEVICE_MEMORY;
    }
    if((createInfo.pool != VK_NULL_HANDLE) &&
        ((createInfo.flags & (VMA_ALLOCATION_CREATE_OWN_MEMORY_BIT)) != 0))
    {
        VMA_ASSERT(0 && "Specifying VMA_ALLOCATION_CREATE_OWN_MEMORY_BIT when pool != null is invalid.");
        return VK_ERROR_OUT_OF_DEVICE_MEMORY;
    }

    if(createInfo.pool != VK_NULL_HANDLE)
    {
        return createInfo.pool->m_BlockVector.Allocate(
            createInfo.pool,
            m_CurrentFrameIndex.load(),
            vkMemReq,
            createInfo,
            suballocType,
            pAllocation);
    }
    else
    {
        // Bit mask of memory Vulkan types acceptable for this allocation.
        uint32_t memoryTypeBits = vkMemReq.memoryTypeBits;
        uint32_t memTypeIndex = UINT32_MAX;
        VkResult res = vmaFindMemoryTypeIndex(this, memoryTypeBits, &createInfo, &memTypeIndex);
        if(res == VK_SUCCESS)
        {
            res = AllocateMemoryOfType(vkMemReq, createInfo, memTypeIndex, suballocType, pAllocation);
            // Succeeded on first try.
            if(res == VK_SUCCESS)
            {
                return res;
            }
            // Allocation from this memory type failed. Try other compatible memory types.
            else
            {
                for(;;)
                {
                    // Remove old memTypeIndex from list of possibilities.
                    memoryTypeBits &= ~(1u << memTypeIndex);
                    // Find alternative memTypeIndex.
                    res = vmaFindMemoryTypeIndex(this, memoryTypeBits, &createInfo, &memTypeIndex);
                    if(res == VK_SUCCESS)
                    {
                        res = AllocateMemoryOfType(vkMemReq, createInfo, memTypeIndex, suballocType, pAllocation);
                        // Allocation from this alternative memory type succeeded.
                        if(res == VK_SUCCESS)
                        {
                            return res;
                        }
                        // else: Allocation from this memory type failed. Try next one - next loop iteration.
                    }
                    // No other matching memory type index could be found.
                    else
                    {
                        // Not returning res, which is VK_ERROR_FEATURE_NOT_PRESENT, because we already failed to allocate once.
                        return VK_ERROR_OUT_OF_DEVICE_MEMORY;
                    }
                }
            }
        }
        // Can't find any single memory type maching requirements. res is VK_ERROR_FEATURE_NOT_PRESENT.
        else
            return res;
    }
}

void VmaAllocator_T::FreeMemory(const VmaAllocation allocation)
{
    VMA_ASSERT(allocation);

    if(allocation->CanBecomeLost() == false ||
        allocation->GetLastUseFrameIndex() != VMA_FRAME_INDEX_LOST)
    {
        switch(allocation->GetType())
        {
        case VmaAllocation_T::ALLOCATION_TYPE_BLOCK:
            {
                VmaBlockVector* pBlockVector = VMA_NULL;
                VmaPool hPool = allocation->GetPool();
                if(hPool != VK_NULL_HANDLE)
                {
                    pBlockVector = &hPool->m_BlockVector;
                }
                else
                {
                    const uint32_t memTypeIndex = allocation->GetMemoryTypeIndex();
                    const VMA_BLOCK_VECTOR_TYPE blockVectorType = allocation->GetBlockVectorType();
                    pBlockVector = m_pBlockVectors[memTypeIndex][blockVectorType];
                }
                pBlockVector->Free(allocation);
            }
            break;
        case VmaAllocation_T::ALLOCATION_TYPE_OWN:
            FreeOwnMemory(allocation);
            break;
        default:
            VMA_ASSERT(0);
        }
    }

    vma_delete(this, allocation);
}

void VmaAllocator_T::CalculateStats(VmaStats* pStats)
{
    // Initialize.
    InitStatInfo(pStats->total);
    for(size_t i = 0; i < VK_MAX_MEMORY_TYPES; ++i)
        InitStatInfo(pStats->memoryType[i]);
    for(size_t i = 0; i < VK_MAX_MEMORY_HEAPS; ++i)
        InitStatInfo(pStats->memoryHeap[i]);
    
    // Process default pools.
    for(uint32_t memTypeIndex = 0; memTypeIndex < GetMemoryTypeCount(); ++memTypeIndex)
    {
        const uint32_t heapIndex = MemoryTypeIndexToHeapIndex(memTypeIndex);
        for(uint32_t blockVectorType = 0; blockVectorType < VMA_BLOCK_VECTOR_TYPE_COUNT; ++blockVectorType)
        {
            VmaBlockVector* const pBlockVector = m_pBlockVectors[memTypeIndex][blockVectorType];
            VMA_ASSERT(pBlockVector);
            pBlockVector->AddStats(pStats);
        }
    }

    // Process custom pools.
    {
        VmaMutexLock lock(m_PoolsMutex, m_UseMutex);
        for(size_t poolIndex = 0, poolCount = m_Pools.size(); poolIndex < poolCount; ++poolIndex)
        {
            m_Pools[poolIndex]->GetBlockVector().AddStats(pStats);
        }
    }

    // Process own allocations.
    for(uint32_t memTypeIndex = 0; memTypeIndex < GetMemoryTypeCount(); ++memTypeIndex)
    {
        const uint32_t memHeapIndex = MemoryTypeIndexToHeapIndex(memTypeIndex);
        VmaMutexLock ownAllocationsLock(m_OwnAllocationsMutex[memTypeIndex], m_UseMutex);
        for(uint32_t blockVectorType = 0; blockVectorType < VMA_BLOCK_VECTOR_TYPE_COUNT; ++blockVectorType)
        {
            AllocationVectorType* const pOwnAllocVector = m_pOwnAllocations[memTypeIndex][blockVectorType];
            VMA_ASSERT(pOwnAllocVector);
            for(size_t allocIndex = 0, allocCount = pOwnAllocVector->size(); allocIndex < allocCount; ++allocIndex)
            {
                VmaStatInfo allocationStatInfo;
                (*pOwnAllocVector)[allocIndex]->OwnAllocCalcStatsInfo(allocationStatInfo);
                VmaAddStatInfo(pStats->total, allocationStatInfo);
                VmaAddStatInfo(pStats->memoryType[memTypeIndex], allocationStatInfo);
                VmaAddStatInfo(pStats->memoryHeap[memHeapIndex], allocationStatInfo);
            }
        }
    }

    // Postprocess.
    VmaPostprocessCalcStatInfo(pStats->total);
    for(size_t i = 0; i < GetMemoryTypeCount(); ++i)
        VmaPostprocessCalcStatInfo(pStats->memoryType[i]);
    for(size_t i = 0; i < GetMemoryHeapCount(); ++i)
        VmaPostprocessCalcStatInfo(pStats->memoryHeap[i]);
}

static const uint32_t VMA_VENDOR_ID_AMD = 4098;

void VmaAllocator_T::UnmapPersistentlyMappedMemory()
{
    if(m_UnmapPersistentlyMappedMemoryCounter++ == 0)
    {
        if(m_PhysicalDeviceProperties.vendorID == VMA_VENDOR_ID_AMD)
        {
            for(uint32_t memTypeIndex = m_MemProps.memoryTypeCount; memTypeIndex--; )
            {
                const VkMemoryPropertyFlags memFlags = m_MemProps.memoryTypes[memTypeIndex].propertyFlags;
                if((memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0 &&
                    (memFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0)
                {
                    // Process OwnAllocations.
                    {
                        VmaMutexLock lock(m_OwnAllocationsMutex[memTypeIndex], m_UseMutex);
                        AllocationVectorType* pOwnAllocationsVector = m_pOwnAllocations[memTypeIndex][VMA_BLOCK_VECTOR_TYPE_MAPPED];
                        for(size_t ownAllocIndex = pOwnAllocationsVector->size(); ownAllocIndex--; )
                        {
                            VmaAllocation hAlloc = (*pOwnAllocationsVector)[ownAllocIndex];
                            hAlloc->OwnAllocUnmapPersistentlyMappedMemory(m_hDevice);
                        }
                    }

                    // Process normal Allocations.
                    {
                        VmaBlockVector* pBlockVector = m_pBlockVectors[memTypeIndex][VMA_BLOCK_VECTOR_TYPE_MAPPED];
                        pBlockVector->UnmapPersistentlyMappedMemory();
                    }
                }
            }

            // Process custom pools.
            {
                VmaMutexLock lock(m_PoolsMutex, m_UseMutex);
                for(size_t poolIndex = 0, poolCount = m_Pools.size(); poolIndex < poolCount; ++poolIndex)
                {
                    m_Pools[poolIndex]->GetBlockVector().UnmapPersistentlyMappedMemory();
                }
            }
        }
    }
}

VkResult VmaAllocator_T::MapPersistentlyMappedMemory()
{
    VMA_ASSERT(m_UnmapPersistentlyMappedMemoryCounter > 0);
    if(--m_UnmapPersistentlyMappedMemoryCounter == 0)
    {
        VkResult finalResult = VK_SUCCESS;
        if(m_PhysicalDeviceProperties.vendorID == VMA_VENDOR_ID_AMD)
        {
            // Process custom pools.
            {
                VmaMutexLock lock(m_PoolsMutex, m_UseMutex);
                for(size_t poolIndex = 0, poolCount = m_Pools.size(); poolIndex < poolCount; ++poolIndex)
                {
                    m_Pools[poolIndex]->GetBlockVector().MapPersistentlyMappedMemory();
                }
            }

            for(uint32_t memTypeIndex = 0; memTypeIndex < m_MemProps.memoryTypeCount; ++memTypeIndex)
            {
                const VkMemoryPropertyFlags memFlags = m_MemProps.memoryTypes[memTypeIndex].propertyFlags;
                if((memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0 &&
                    (memFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0)
                {
                    // Process OwnAllocations.
                    {
                        VmaMutexLock lock(m_OwnAllocationsMutex[memTypeIndex], m_UseMutex);
                        AllocationVectorType* pAllocationsVector = m_pOwnAllocations[memTypeIndex][VMA_BLOCK_VECTOR_TYPE_MAPPED];
                        for(size_t ownAllocIndex = 0, ownAllocCount = pAllocationsVector->size(); ownAllocIndex < ownAllocCount; ++ownAllocIndex)
                        {
                            VmaAllocation hAlloc = (*pAllocationsVector)[ownAllocIndex];
                            hAlloc->OwnAllocMapPersistentlyMappedMemory(m_hDevice);
                        }
                    }

                    // Process normal Allocations.
                    {
                        VmaBlockVector* pBlockVector = m_pBlockVectors[memTypeIndex][VMA_BLOCK_VECTOR_TYPE_MAPPED];
                        VkResult localResult = pBlockVector->MapPersistentlyMappedMemory();
                        if(localResult != VK_SUCCESS)
                        {
                            finalResult = localResult;
                        }
                    }
                }
            }
        }
        return finalResult;
    }
    else
        return VK_SUCCESS;
}

VkResult VmaAllocator_T::Defragment(
    VmaAllocation* pAllocations,
    size_t allocationCount,
    VkBool32* pAllocationsChanged,
    const VmaDefragmentationInfo* pDefragmentationInfo,
    VmaDefragmentationStats* pDefragmentationStats)
{
    if(pAllocationsChanged != VMA_NULL)
    {
        memset(pAllocationsChanged, 0, sizeof(*pAllocationsChanged));
    }
    if(pDefragmentationStats != VMA_NULL)
    {
        memset(pDefragmentationStats, 0, sizeof(*pDefragmentationStats));
    }

    if(m_UnmapPersistentlyMappedMemoryCounter > 0)
    {
        VMA_DEBUG_LOG("ERROR: Cannot defragment when inside vmaUnmapPersistentlyMappedMemory.");
        return VK_ERROR_MEMORY_MAP_FAILED;
    }

    const uint32_t currentFrameIndex = m_CurrentFrameIndex.load();

    VmaMutexLock poolsLock(m_PoolsMutex, m_UseMutex);

    const size_t poolCount = m_Pools.size();

    // Dispatch pAllocations among defragmentators. Create them in BlockVectors when necessary.
    for(size_t allocIndex = 0; allocIndex < allocationCount; ++allocIndex)
    {
        VmaAllocation hAlloc = pAllocations[allocIndex];
        VMA_ASSERT(hAlloc);
        const uint32_t memTypeIndex = hAlloc->GetMemoryTypeIndex();
        // OwnAlloc cannot be defragmented.
        if((hAlloc->GetType() == VmaAllocation_T::ALLOCATION_TYPE_BLOCK) &&
            // Only HOST_VISIBLE memory types can be defragmented.
            ((m_MemProps.memoryTypes[memTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0) &&
            // Lost allocation cannot be defragmented.
            (hAlloc->GetLastUseFrameIndex() != VMA_FRAME_INDEX_LOST))
        {
            VmaBlockVector* pAllocBlockVector = nullptr;

            const VmaPool hAllocPool = hAlloc->GetPool();
            // This allocation belongs to custom pool.
            if(hAllocPool != VK_NULL_HANDLE)
            {
                pAllocBlockVector = &hAllocPool->GetBlockVector();
            }
            // This allocation belongs to general pool.
            else
            {
                pAllocBlockVector = m_pBlockVectors[memTypeIndex][hAlloc->GetBlockVectorType()];
            }

            VmaDefragmentator* const pDefragmentator = pAllocBlockVector->EnsureDefragmentator(
                m_hDevice,
                GetAllocationCallbacks(),
                currentFrameIndex);

            VkBool32* const pChanged = (pAllocationsChanged != VMA_NULL) ?
                &pAllocationsChanged[allocIndex] : VMA_NULL;
            pDefragmentator->AddAllocation(hAlloc, pChanged);
        }
    }

    VkResult result = VK_SUCCESS;

    // ======== Main processing.

    VkDeviceSize maxBytesToMove = SIZE_MAX;
    uint32_t maxAllocationsToMove = UINT32_MAX;
    if(pDefragmentationInfo != VMA_NULL)
    {
        maxBytesToMove = pDefragmentationInfo->maxBytesToMove;
        maxAllocationsToMove = pDefragmentationInfo->maxAllocationsToMove;
    }

    // Process standard memory.
    for(uint32_t memTypeIndex = 0;
        (memTypeIndex < GetMemoryTypeCount()) && (result == VK_SUCCESS);
        ++memTypeIndex)
    {
        // Only HOST_VISIBLE memory types can be defragmented.
        if((m_MemProps.memoryTypes[memTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)
        {
            for(uint32_t blockVectorType = 0;
                (blockVectorType < VMA_BLOCK_VECTOR_TYPE_COUNT) && (result == VK_SUCCESS);
                ++blockVectorType)
            {
                result = m_pBlockVectors[memTypeIndex][blockVectorType]->Defragment(
                    pDefragmentationStats,
                    maxBytesToMove,
                    maxAllocationsToMove);
            }
        }
    }

    // Process custom pools.
    for(size_t poolIndex = 0; (poolIndex < poolCount) && (result == VK_SUCCESS); ++poolIndex)
    {
        result = m_Pools[poolIndex]->GetBlockVector().Defragment(
            pDefragmentationStats,
            maxBytesToMove,
            maxAllocationsToMove);
    }

    // ========  Destroy defragmentators.

    // Process custom pools.
    for(size_t poolIndex = poolCount; poolIndex--; )
    {
        m_Pools[poolIndex]->GetBlockVector().DestroyDefragmentator();
    }

    // Process standard memory.
    for(uint32_t memTypeIndex = GetMemoryTypeCount(); memTypeIndex--; )
    {
        if((m_MemProps.memoryTypes[memTypeIndex].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)
        {
            for(size_t blockVectorType = VMA_BLOCK_VECTOR_TYPE_COUNT; blockVectorType--; )
            {
                m_pBlockVectors[memTypeIndex][blockVectorType]->DestroyDefragmentator();
            }
        }
    }

    return result;
}

void VmaAllocator_T::GetAllocationInfo(VmaAllocation hAllocation, VmaAllocationInfo* pAllocationInfo)
{
    if(hAllocation->CanBecomeLost())
    {
        /*
        Warning: This is a carefully designed algorithm.
        Do not modify unless you really know what you're doing :)
        */
        uint32_t localCurrFrameIndex = m_CurrentFrameIndex.load();
        uint32_t localLastUseFrameIndex = hAllocation->GetLastUseFrameIndex();
        for(;;)
        {
            if(localLastUseFrameIndex == VMA_FRAME_INDEX_LOST)
            {
                pAllocationInfo->memoryType = UINT32_MAX;
                pAllocationInfo->deviceMemory = VK_NULL_HANDLE;
                pAllocationInfo->offset = 0;
                pAllocationInfo->size = hAllocation->GetSize();
                pAllocationInfo->pMappedData = VMA_NULL;
                pAllocationInfo->pUserData = hAllocation->GetUserData();
                return;
            }
            else if(localLastUseFrameIndex == localCurrFrameIndex)
            {
                pAllocationInfo->memoryType = hAllocation->GetMemoryTypeIndex();
                pAllocationInfo->deviceMemory = hAllocation->GetMemory();
                pAllocationInfo->offset = hAllocation->GetOffset();
                pAllocationInfo->size = hAllocation->GetSize();
                pAllocationInfo->pMappedData = hAllocation->GetMappedData();
                pAllocationInfo->pUserData = hAllocation->GetUserData();
                return;
            }
            else // Last use time earlier than current time.
            {
                if(hAllocation->CompareExchangeLastUseFrameIndex(localLastUseFrameIndex, localCurrFrameIndex))
                {
                    localLastUseFrameIndex = localCurrFrameIndex;
                }
            }
        }
    }
    // We could use the same code here, but for performance reasons we don't need to use the hAllocation.LastUseFrameIndex atomic.
    else
    {
        pAllocationInfo->memoryType = hAllocation->GetMemoryTypeIndex();
        pAllocationInfo->deviceMemory = hAllocation->GetMemory();
        pAllocationInfo->offset = hAllocation->GetOffset();
        pAllocationInfo->size = hAllocation->GetSize();
        pAllocationInfo->pMappedData = hAllocation->GetMappedData();
        pAllocationInfo->pUserData = hAllocation->GetUserData();
    }
}

VkResult VmaAllocator_T::CreatePool(const VmaPoolCreateInfo* pCreateInfo, VmaPool* pPool)
{
    VMA_DEBUG_LOG("  CreatePool: MemoryTypeIndex=%u", pCreateInfo->memoryTypeIndex);

    VmaPoolCreateInfo newCreateInfo = *pCreateInfo;

    if(newCreateInfo.maxBlockCount == 0)
    {
        newCreateInfo.maxBlockCount = SIZE_MAX;
    }
    if(newCreateInfo.blockSize == 0)
    {
        newCreateInfo.blockSize = CalcPreferredBlockSize(newCreateInfo.memoryTypeIndex);
    }

    *pPool = vma_new(this, VmaPool_T)(this, newCreateInfo);

    VkResult res = (*pPool)->m_BlockVector.CreateMinBlocks();
    if(res != VK_SUCCESS)
    {
        vma_delete(this, *pPool);
        *pPool = VMA_NULL;
        return res;
    }

    // Add to m_Pools.
    {
        VmaMutexLock lock(m_PoolsMutex, m_UseMutex);
        VmaVectorInsertSorted<VmaPointerLess>(m_Pools, *pPool);
    }

    return VK_SUCCESS;
}

void VmaAllocator_T::DestroyPool(VmaPool pool)
{
    // Remove from m_Pools.
    {
        VmaMutexLock lock(m_PoolsMutex, m_UseMutex);
        bool success = VmaVectorRemoveSorted<VmaPointerLess>(m_Pools, pool);
        VMA_ASSERT(success && "Pool not found in Allocator.");
    }

    vma_delete(this, pool);
}

void VmaAllocator_T::GetPoolStats(VmaPool pool, VmaPoolStats* pPoolStats)
{
    pool->m_BlockVector.GetPoolStats(pPoolStats);
}

void VmaAllocator_T::SetCurrentFrameIndex(uint32_t frameIndex)
{
    m_CurrentFrameIndex.store(frameIndex);
}

void VmaAllocator_T::MakePoolAllocationsLost(
    VmaPool hPool,
    size_t* pLostAllocationCount)
{
    hPool->m_BlockVector.MakePoolAllocationsLost(
        m_CurrentFrameIndex.load(),
        pLostAllocationCount);
}

void VmaAllocator_T::CreateLostAllocation(VmaAllocation* pAllocation)
{
    *pAllocation = vma_new(this, VmaAllocation_T)(VMA_FRAME_INDEX_LOST);
    (*pAllocation)->InitLost();
}

void VmaAllocator_T::FreeOwnMemory(VmaAllocation allocation)
{
    VMA_ASSERT(allocation && allocation->GetType() == VmaAllocation_T::ALLOCATION_TYPE_OWN);

    const uint32_t memTypeIndex = allocation->GetMemoryTypeIndex();
    {
        VmaMutexLock lock(m_OwnAllocationsMutex[memTypeIndex], m_UseMutex);
        AllocationVectorType* const pOwnAllocations = m_pOwnAllocations[memTypeIndex][allocation->GetBlockVectorType()];
        VMA_ASSERT(pOwnAllocations);
        bool success = VmaVectorRemoveSorted<VmaPointerLess>(*pOwnAllocations, allocation);
        VMA_ASSERT(success);
    }

    VkDeviceMemory hMemory = allocation->GetMemory();
    
    // Callback.
    if(m_DeviceMemoryCallbacks.pfnFree != VMA_NULL)
    {
        (*m_DeviceMemoryCallbacks.pfnFree)(this, memTypeIndex, hMemory, allocation->GetSize());
    }

    if(allocation->GetMappedData() != VMA_NULL)
    {
        vkUnmapMemory(m_hDevice, hMemory);
    }
    
    vkFreeMemory(m_hDevice, hMemory, GetAllocationCallbacks());

    VMA_DEBUG_LOG("    Freed OwnMemory MemoryTypeIndex=%u", memTypeIndex);
}

#if VMA_STATS_STRING_ENABLED

void VmaAllocator_T::PrintDetailedMap(VmaJsonWriter& json)
{
    bool ownAllocationsStarted = false;
    for(uint32_t memTypeIndex = 0; memTypeIndex < GetMemoryTypeCount(); ++memTypeIndex)
    {
        VmaMutexLock ownAllocationsLock(m_OwnAllocationsMutex[memTypeIndex], m_UseMutex);
        for(uint32_t blockVectorType = 0; blockVectorType < VMA_BLOCK_VECTOR_TYPE_COUNT; ++blockVectorType)
        {
            AllocationVectorType* const pOwnAllocVector = m_pOwnAllocations[memTypeIndex][blockVectorType];
            VMA_ASSERT(pOwnAllocVector);
            if(pOwnAllocVector->empty() == false)
            {
                if(ownAllocationsStarted == false)
                {
                    ownAllocationsStarted = true;
                    json.WriteString("OwnAllocations");
                    json.BeginObject();
                }

                json.BeginString("Type ");
                json.ContinueString(memTypeIndex);
                if(blockVectorType == VMA_BLOCK_VECTOR_TYPE_MAPPED)
                {
                    json.ContinueString(" Mapped");
                }
                json.EndString();
                
                json.BeginArray();

                for(size_t i = 0; i < pOwnAllocVector->size(); ++i)
                {
                    const VmaAllocation hAlloc = (*pOwnAllocVector)[i];
                    json.BeginObject(true);
                    
                    json.WriteString("Size");
                    json.WriteNumber(hAlloc->GetSize());

                    json.WriteString("Type");
                    json.WriteString(VMA_SUBALLOCATION_TYPE_NAMES[hAlloc->GetSuballocationType()]);

                    json.EndObject();
                }

                json.EndArray();
            }
        }
    }
    if(ownAllocationsStarted)
    {
        json.EndObject();
    }

    {
        bool allocationsStarted = false;
        for(uint32_t memTypeIndex = 0; memTypeIndex < GetMemoryTypeCount(); ++memTypeIndex)
        {
            for(uint32_t blockVectorType = 0; blockVectorType < VMA_BLOCK_VECTOR_TYPE_COUNT; ++blockVectorType)
            {
                if(m_pBlockVectors[memTypeIndex][blockVectorType]->IsEmpty() == false)
                {
                    if(allocationsStarted == false)
                    {
                        allocationsStarted = true;
                        json.WriteString("DefaultPools");
                        json.BeginObject();
                    }

                    json.BeginString("Type ");
                    json.ContinueString(memTypeIndex);
                    if(blockVectorType == VMA_BLOCK_VECTOR_TYPE_MAPPED)
                    {
                        json.ContinueString(" Mapped");
                    }
                    json.EndString();

                    m_pBlockVectors[memTypeIndex][blockVectorType]->PrintDetailedMap(json);
                }
            }
        }
        if(allocationsStarted)
        {
            json.EndObject();
        }
    }

    {
        VmaMutexLock lock(m_PoolsMutex, m_UseMutex);
        const size_t poolCount = m_Pools.size();
        if(poolCount > 0)
        {
            json.WriteString("Pools");
            json.BeginArray();
            for(size_t poolIndex = 0; poolIndex < poolCount; ++poolIndex)
            {
                m_Pools[poolIndex]->m_BlockVector.PrintDetailedMap(json);
            }
            json.EndArray();
        }
    }
}

#endif // #if VMA_STATS_STRING_ENABLED

static VkResult AllocateMemoryForImage(
    VmaAllocator allocator,
    VkImage image,
    const VmaAllocationCreateInfo* pAllocationCreateInfo,
    VmaSuballocationType suballocType,
    VmaAllocation* pAllocation)
{
    VMA_ASSERT(allocator && (image != VK_NULL_HANDLE) && pAllocationCreateInfo && pAllocation);
    
    VkMemoryRequirements vkMemReq = {};
    vkGetImageMemoryRequirements(allocator->m_hDevice, image, &vkMemReq);

    return allocator->AllocateMemory(
        vkMemReq,
        *pAllocationCreateInfo,
        suballocType,
        pAllocation);
}

////////////////////////////////////////////////////////////////////////////////
// Public interface

VkResult vmaCreateAllocator(
    const VmaAllocatorCreateInfo* pCreateInfo,
    VmaAllocator* pAllocator)
{
    VMA_ASSERT(pCreateInfo && pAllocator);
    VMA_DEBUG_LOG("vmaCreateAllocator");
    *pAllocator = vma_new(pCreateInfo->pAllocationCallbacks, VmaAllocator_T)(pCreateInfo);
    return VK_SUCCESS;
}

void vmaDestroyAllocator(
    VmaAllocator allocator)
{
    if(allocator != VK_NULL_HANDLE)
    {
        VMA_DEBUG_LOG("vmaDestroyAllocator");
        VkAllocationCallbacks allocationCallbacks = allocator->m_AllocationCallbacks;
        vma_delete(&allocationCallbacks, allocator);
    }
}

void vmaGetPhysicalDeviceProperties(
    VmaAllocator allocator,
    const VkPhysicalDeviceProperties **ppPhysicalDeviceProperties)
{
    VMA_ASSERT(allocator && ppPhysicalDeviceProperties);
    *ppPhysicalDeviceProperties = &allocator->m_PhysicalDeviceProperties;
}

void vmaGetMemoryProperties(
    VmaAllocator allocator,
    const VkPhysicalDeviceMemoryProperties** ppPhysicalDeviceMemoryProperties)
{
    VMA_ASSERT(allocator && ppPhysicalDeviceMemoryProperties);
    *ppPhysicalDeviceMemoryProperties = &allocator->m_MemProps;
}

void vmaGetMemoryTypeProperties(
    VmaAllocator allocator,
    uint32_t memoryTypeIndex,
    VkMemoryPropertyFlags* pFlags)
{
    VMA_ASSERT(allocator && pFlags);
    VMA_ASSERT(memoryTypeIndex < allocator->GetMemoryTypeCount());
    *pFlags = allocator->m_MemProps.memoryTypes[memoryTypeIndex].propertyFlags;
}

void vmaSetCurrentFrameIndex(
    VmaAllocator allocator,
    uint32_t frameIndex)
{
    VMA_ASSERT(allocator);
    VMA_ASSERT(frameIndex != VMA_FRAME_INDEX_LOST);

    VMA_DEBUG_GLOBAL_MUTEX_LOCK

    allocator->SetCurrentFrameIndex(frameIndex);
}

void vmaCalculateStats(
    VmaAllocator allocator,
    VmaStats* pStats)
{
    VMA_ASSERT(allocator && pStats);
    VMA_DEBUG_GLOBAL_MUTEX_LOCK
    allocator->CalculateStats(pStats);
}

#if VMA_STATS_STRING_ENABLED

void vmaBuildStatsString(
    VmaAllocator allocator,
    char** ppStatsString,
    VkBool32 detailedMap)
{
    VMA_ASSERT(allocator && ppStatsString);
    VMA_DEBUG_GLOBAL_MUTEX_LOCK

    VmaStringBuilder sb(allocator);
    {
        VmaJsonWriter json(allocator->GetAllocationCallbacks(), sb);
        json.BeginObject();

        VmaStats stats;
        allocator->CalculateStats(&stats);

        json.WriteString("Total");
        VmaPrintStatInfo(json, stats.total);
    
        for(uint32_t heapIndex = 0; heapIndex < allocator->GetMemoryHeapCount(); ++heapIndex)
        {
            json.BeginString("Heap ");
            json.ContinueString(heapIndex);
            json.EndString();
            json.BeginObject();

            json.WriteString("Size");
            json.WriteNumber(allocator->m_MemProps.memoryHeaps[heapIndex].size);

            json.WriteString("Flags");
            json.BeginArray(true);
            if((allocator->m_MemProps.memoryHeaps[heapIndex].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0)
            {
                json.WriteString("DEVICE_LOCAL");
            }
            json.EndArray();

            if(stats.memoryHeap[heapIndex].BlockCount > 0)
            {
                json.WriteString("Stats");
                VmaPrintStatInfo(json, stats.memoryHeap[heapIndex]);
            }

            for(uint32_t typeIndex = 0; typeIndex < allocator->GetMemoryTypeCount(); ++typeIndex)
            {
                if(allocator->MemoryTypeIndexToHeapIndex(typeIndex) == heapIndex)
                {
                    json.BeginString("Type ");
                    json.ContinueString(typeIndex);
                    json.EndString();

                    json.BeginObject();

                    json.WriteString("Flags");
                    json.BeginArray(true);
                    VkMemoryPropertyFlags flags = allocator->m_MemProps.memoryTypes[typeIndex].propertyFlags;
                    if((flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) != 0)
                    {
                        json.WriteString("DEVICE_LOCAL");
                    }
                    if((flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)
                    {
                        json.WriteString("HOST_VISIBLE");
                    }
                    if((flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0)
                    {
                        json.WriteString("HOST_COHERENT");
                    }
                    if((flags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) != 0)
                    {
                        json.WriteString("HOST_CACHED");
                    }
                    if((flags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) != 0)
                    {
                        json.WriteString("LAZILY_ALLOCATED");
                    }
                    json.EndArray();

                    if(stats.memoryType[typeIndex].BlockCount > 0)
                    {
                        json.WriteString("Stats");
                        VmaPrintStatInfo(json, stats.memoryType[typeIndex]);
                    }

                    json.EndObject();
                }
            }

            json.EndObject();
        }
        if(detailedMap == VK_TRUE)
        {
            allocator->PrintDetailedMap(json);
        }

        json.EndObject();
    }

    const size_t len = sb.GetLength();
    char* const pChars = vma_new_array(allocator, char, len + 1);
    if(len > 0)
    {
        memcpy(pChars, sb.GetData(), len);
    }
    pChars[len] = '\0';
    *ppStatsString = pChars;
}

void vmaFreeStatsString(
    VmaAllocator allocator,
    char* pStatsString)
{
    if(pStatsString != VMA_NULL)
    {
        VMA_ASSERT(allocator);
        size_t len = strlen(pStatsString);
        vma_delete_array(allocator, pStatsString, len + 1);
    }
}

#endif // #if VMA_STATS_STRING_ENABLED

/** This function is not protected by any mutex because it just reads immutable data.
*/
VkResult vmaFindMemoryTypeIndex(
    VmaAllocator allocator,
    uint32_t memoryTypeBits,
    const VmaAllocationCreateInfo* pAllocationCreateInfo,
    uint32_t* pMemoryTypeIndex)
{
    VMA_ASSERT(allocator != VK_NULL_HANDLE);
    VMA_ASSERT(pAllocationCreateInfo != VMA_NULL);
    VMA_ASSERT(pMemoryTypeIndex != VMA_NULL);
    
    uint32_t requiredFlags = pAllocationCreateInfo->requiredFlags;
    uint32_t preferredFlags = pAllocationCreateInfo->preferredFlags;
    if(preferredFlags == 0)
    {
        preferredFlags = requiredFlags;
    }
    // preferredFlags, if not 0, must be a superset of requiredFlags.
    VMA_ASSERT((requiredFlags & ~preferredFlags) == 0);

    // Convert usage to requiredFlags and preferredFlags.
    switch(pAllocationCreateInfo->usage)
    {
    case VMA_MEMORY_USAGE_UNKNOWN:
        break;
    case VMA_MEMORY_USAGE_GPU_ONLY:
        preferredFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        break;
    case VMA_MEMORY_USAGE_CPU_ONLY:
        requiredFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        break;
    case VMA_MEMORY_USAGE_CPU_TO_GPU:
        requiredFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        preferredFlags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        break;
    case VMA_MEMORY_USAGE_GPU_TO_CPU:
        requiredFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        preferredFlags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        break;
    default:
        break;
    }

    if((pAllocationCreateInfo->flags & VMA_ALLOCATION_CREATE_PERSISTENT_MAP_BIT) != 0)
    {
        requiredFlags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    }

    *pMemoryTypeIndex = UINT32_MAX;
    uint32_t minCost = UINT32_MAX;
    for(uint32_t memTypeIndex = 0, memTypeBit = 1;
        memTypeIndex < allocator->GetMemoryTypeCount();
        ++memTypeIndex, memTypeBit <<= 1)
    {
        // This memory type is acceptable according to memoryTypeBits bitmask.
        if((memTypeBit & memoryTypeBits) != 0)
        {
            const VkMemoryPropertyFlags currFlags =
                allocator->m_MemProps.memoryTypes[memTypeIndex].propertyFlags;
            // This memory type contains requiredFlags.
            if((requiredFlags & ~currFlags) == 0)
            {
                // Calculate cost as number of bits from preferredFlags not present in this memory type.
                uint32_t currCost = CountBitsSet(preferredFlags & ~currFlags);
                // Remember memory type with lowest cost.
                if(currCost < minCost)
                {
                    *pMemoryTypeIndex = memTypeIndex;
                    if(currCost == 0)
                    {
                        return VK_SUCCESS;
                    }
                    minCost = currCost;
                }
            }
        }
    }
    return (*pMemoryTypeIndex != UINT32_MAX) ? VK_SUCCESS : VK_ERROR_FEATURE_NOT_PRESENT;
}

VkResult vmaCreatePool(
	VmaAllocator allocator,
	const VmaPoolCreateInfo* pCreateInfo,
	VmaPool* pPool)
{
    VMA_ASSERT(allocator && pCreateInfo && pPool);

    VMA_DEBUG_LOG("vmaCreatePool");

    VMA_DEBUG_GLOBAL_MUTEX_LOCK

    return allocator->CreatePool(pCreateInfo, pPool);
}

void vmaDestroyPool(
    VmaAllocator allocator,
    VmaPool pool)
{
    VMA_ASSERT(allocator && pool);

    VMA_DEBUG_LOG("vmaDestroyPool");

    VMA_DEBUG_GLOBAL_MUTEX_LOCK

    allocator->DestroyPool(pool);
}

void vmaGetPoolStats(
    VmaAllocator allocator,
    VmaPool pool,
    VmaPoolStats* pPoolStats)
{
    VMA_ASSERT(allocator && pool && pPoolStats);

    VMA_DEBUG_GLOBAL_MUTEX_LOCK

    allocator->GetPoolStats(pool, pPoolStats);
}

void vmaMakePoolAllocationsLost(
    VmaAllocator allocator,
    VmaPool pool,
    size_t* pLostAllocationCount)
{
    VMA_ASSERT(allocator && pool);

    VMA_DEBUG_GLOBAL_MUTEX_LOCK

    allocator->MakePoolAllocationsLost(pool, pLostAllocationCount);
}

VkResult vmaAllocateMemory(
    VmaAllocator allocator,
    const VkMemoryRequirements* pVkMemoryRequirements,
    const VmaAllocationCreateInfo* pCreateInfo,
    VmaAllocation* pAllocation,
    VmaAllocationInfo* pAllocationInfo)
{
    VMA_ASSERT(allocator && pVkMemoryRequirements && pCreateInfo && pAllocation);

    VMA_DEBUG_LOG("vmaAllocateMemory");

    VMA_DEBUG_GLOBAL_MUTEX_LOCK

	VkResult result = allocator->AllocateMemory(
        *pVkMemoryRequirements,
        *pCreateInfo,
        VMA_SUBALLOCATION_TYPE_UNKNOWN,
        pAllocation);

    if(pAllocationInfo && result == VK_SUCCESS)
    {
        allocator->GetAllocationInfo(*pAllocation, pAllocationInfo);
    }

	return result;
}

VkResult vmaAllocateMemoryForBuffer(
    VmaAllocator allocator,
    VkBuffer buffer,
    const VmaAllocationCreateInfo* pCreateInfo,
    VmaAllocation* pAllocation,
    VmaAllocationInfo* pAllocationInfo)
{
    VMA_ASSERT(allocator && buffer != VK_NULL_HANDLE && pCreateInfo && pAllocation);

    VMA_DEBUG_LOG("vmaAllocateMemoryForBuffer");

    VMA_DEBUG_GLOBAL_MUTEX_LOCK

    VkMemoryRequirements vkMemReq = {};
    vkGetBufferMemoryRequirements(allocator->m_hDevice, buffer, &vkMemReq);

    VkResult result = allocator->AllocateMemory(
        vkMemReq,
        *pCreateInfo,
        VMA_SUBALLOCATION_TYPE_BUFFER,
        pAllocation);

    if(pAllocationInfo && result == VK_SUCCESS)
    {
        allocator->GetAllocationInfo(*pAllocation, pAllocationInfo);
    }

	return result;
}

VkResult vmaAllocateMemoryForImage(
    VmaAllocator allocator,
    VkImage image,
    const VmaAllocationCreateInfo* pCreateInfo,
    VmaAllocation* pAllocation,
    VmaAllocationInfo* pAllocationInfo)
{
    VMA_ASSERT(allocator && image != VK_NULL_HANDLE && pCreateInfo && pAllocation);

    VMA_DEBUG_LOG("vmaAllocateMemoryForImage");

    VMA_DEBUG_GLOBAL_MUTEX_LOCK

    VkResult result = AllocateMemoryForImage(
        allocator,
        image,
        pCreateInfo,
        VMA_SUBALLOCATION_TYPE_IMAGE_UNKNOWN,
        pAllocation);

    if(pAllocationInfo && result == VK_SUCCESS)
    {
        allocator->GetAllocationInfo(*pAllocation, pAllocationInfo);
    }

	return result;
}

void vmaFreeMemory(
    VmaAllocator allocator,
    VmaAllocation allocation)
{
    VMA_ASSERT(allocator && allocation);

    VMA_DEBUG_LOG("vmaFreeMemory");

    VMA_DEBUG_GLOBAL_MUTEX_LOCK

    allocator->FreeMemory(allocation);
}

void vmaGetAllocationInfo(
    VmaAllocator allocator,
    VmaAllocation allocation,
    VmaAllocationInfo* pAllocationInfo)
{
    VMA_ASSERT(allocator && allocation && pAllocationInfo);

    VMA_DEBUG_GLOBAL_MUTEX_LOCK

    allocator->GetAllocationInfo(allocation, pAllocationInfo);
}

void vmaSetAllocationUserData(
    VmaAllocator allocator,
    VmaAllocation allocation,
    void* pUserData)
{
    VMA_ASSERT(allocator && allocation);

    VMA_DEBUG_GLOBAL_MUTEX_LOCK

    allocation->SetUserData(pUserData);
}

void vmaCreateLostAllocation(
    VmaAllocator allocator,
    VmaAllocation* pAllocation)
{
    VMA_ASSERT(allocator && pAllocation);

    VMA_DEBUG_GLOBAL_MUTEX_LOCK;

    allocator->CreateLostAllocation(pAllocation);
}

VkResult vmaMapMemory(
    VmaAllocator allocator,
    VmaAllocation allocation,
    void** ppData)
{
    VMA_ASSERT(allocator && allocation && ppData);

    VMA_DEBUG_GLOBAL_MUTEX_LOCK

    return vkMapMemory(allocator->m_hDevice, allocation->GetMemory(),
        allocation->GetOffset(), allocation->GetSize(), 0, ppData);
}

void vmaUnmapMemory(
    VmaAllocator allocator,
    VmaAllocation allocation)
{
    VMA_ASSERT(allocator && allocation);

    VMA_DEBUG_GLOBAL_MUTEX_LOCK

    vkUnmapMemory(allocator->m_hDevice, allocation->GetMemory());
}

void vmaUnmapPersistentlyMappedMemory(VmaAllocator allocator)
{
    VMA_ASSERT(allocator);

    VMA_DEBUG_GLOBAL_MUTEX_LOCK

    allocator->UnmapPersistentlyMappedMemory();
}

VkResult vmaMapPersistentlyMappedMemory(VmaAllocator allocator)
{
    VMA_ASSERT(allocator);

    VMA_DEBUG_GLOBAL_MUTEX_LOCK

    return allocator->MapPersistentlyMappedMemory();
}

VkResult vmaDefragment(
    VmaAllocator allocator,
    VmaAllocation* pAllocations,
    size_t allocationCount,
    VkBool32* pAllocationsChanged,
    const VmaDefragmentationInfo *pDefragmentationInfo,
    VmaDefragmentationStats* pDefragmentationStats)
{
    VMA_ASSERT(allocator && pAllocations);

    VMA_DEBUG_LOG("vmaDefragment");

    VMA_DEBUG_GLOBAL_MUTEX_LOCK

    return allocator->Defragment(pAllocations, allocationCount, pAllocationsChanged, pDefragmentationInfo, pDefragmentationStats);
}

VkResult vmaCreateBuffer(
    VmaAllocator allocator,
    const VkBufferCreateInfo* pBufferCreateInfo,
    const VmaAllocationCreateInfo* pAllocationCreateInfo,
    VkBuffer* pBuffer,
    VmaAllocation* pAllocation,
    VmaAllocationInfo* pAllocationInfo)
{
    VMA_ASSERT(allocator && pBufferCreateInfo && pAllocationCreateInfo && pBuffer && pAllocation);
    
    VMA_DEBUG_LOG("vmaCreateBuffer");
    
    VMA_DEBUG_GLOBAL_MUTEX_LOCK

    *pBuffer = VK_NULL_HANDLE;
    *pAllocation = VK_NULL_HANDLE;

    // 1. Create VkBuffer.
    VkResult res = vkCreateBuffer(allocator->m_hDevice, pBufferCreateInfo, allocator->GetAllocationCallbacks(), pBuffer);
    if(res >= 0)
    {
        // 2. vkGetBufferMemoryRequirements.
        VkMemoryRequirements vkMemReq = {};
        vkGetBufferMemoryRequirements(allocator->m_hDevice, *pBuffer, &vkMemReq);

        // 3. Allocate memory using allocator.
        res = allocator->AllocateMemory(
            vkMemReq,
            *pAllocationCreateInfo,
            VMA_SUBALLOCATION_TYPE_BUFFER,
            pAllocation);
        if(res >= 0)
        {
            // 3. Bind buffer with memory.
            res = vkBindBufferMemory(allocator->m_hDevice, *pBuffer, (*pAllocation)->GetMemory(), (*pAllocation)->GetOffset());
            if(res >= 0)
            {
                // All steps succeeded.
                if(pAllocationInfo != VMA_NULL)
                {
                    allocator->GetAllocationInfo(*pAllocation, pAllocationInfo);
                }
                return VK_SUCCESS;
            }
            allocator->FreeMemory(*pAllocation);
            *pAllocation = VK_NULL_HANDLE;
            return res;
        }
        vkDestroyBuffer(allocator->m_hDevice, *pBuffer, allocator->GetAllocationCallbacks());
        *pBuffer = VK_NULL_HANDLE;
        return res;
    }
    return res;
}

void vmaDestroyBuffer(
    VmaAllocator allocator,
    VkBuffer buffer,
    VmaAllocation allocation)
{
    if(buffer != VK_NULL_HANDLE)
    {
        VMA_ASSERT(allocator);

        VMA_DEBUG_LOG("vmaDestroyBuffer");

        VMA_DEBUG_GLOBAL_MUTEX_LOCK

        vkDestroyBuffer(allocator->m_hDevice, buffer, allocator->GetAllocationCallbacks());
        
        allocator->FreeMemory(allocation);
    }
}

VkResult vmaCreateImage(
    VmaAllocator allocator,
    const VkImageCreateInfo* pImageCreateInfo,
    const VmaAllocationCreateInfo* pAllocationCreateInfo,
    VkImage* pImage,
    VmaAllocation* pAllocation,
    VmaAllocationInfo* pAllocationInfo)
{
    VMA_ASSERT(allocator && pImageCreateInfo && pAllocationCreateInfo && pImage && pAllocation);

    VMA_DEBUG_LOG("vmaCreateImage");

    VMA_DEBUG_GLOBAL_MUTEX_LOCK

    *pImage = VK_NULL_HANDLE;
    *pAllocation = VK_NULL_HANDLE;

    // 1. Create VkImage.
    VkResult res = vkCreateImage(allocator->m_hDevice, pImageCreateInfo, allocator->GetAllocationCallbacks(), pImage);
    if(res >= 0)
    {
        VmaSuballocationType suballocType = pImageCreateInfo->tiling == VK_IMAGE_TILING_OPTIMAL ?
            VMA_SUBALLOCATION_TYPE_IMAGE_OPTIMAL :
            VMA_SUBALLOCATION_TYPE_IMAGE_LINEAR;
        
        // 2. Allocate memory using allocator.
        res = AllocateMemoryForImage(allocator, *pImage, pAllocationCreateInfo, suballocType, pAllocation);
        if(res >= 0)
        {
            // 3. Bind image with memory.
            res = vkBindImageMemory(allocator->m_hDevice, *pImage, (*pAllocation)->GetMemory(), (*pAllocation)->GetOffset());
            if(res >= 0)
            {
                // All steps succeeded.
                if(pAllocationInfo != VMA_NULL)
                {
                    allocator->GetAllocationInfo(*pAllocation, pAllocationInfo);
                }
                return VK_SUCCESS;
            }
            allocator->FreeMemory(*pAllocation);
            *pAllocation = VK_NULL_HANDLE;
            return res;
        }
        vkDestroyImage(allocator->m_hDevice, *pImage, allocator->GetAllocationCallbacks());
        *pImage = VK_NULL_HANDLE;
        return res;
    }
    return res;
}

void vmaDestroyImage(
    VmaAllocator allocator,
    VkImage image,
    VmaAllocation allocation)
{
    if(image != VK_NULL_HANDLE)
    {
        VMA_ASSERT(allocator);

        VMA_DEBUG_LOG("vmaDestroyImage");

        VMA_DEBUG_GLOBAL_MUTEX_LOCK

        vkDestroyImage(allocator->m_hDevice, image, allocator->GetAllocationCallbacks());

        allocator->FreeMemory(allocation);
    }
}

#endif // #ifdef VMA_IMPLEMENTATION
