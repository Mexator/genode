#include <persalloc/pers_heap.h>
#include <logging/mylog.h>

using namespace Persalloc;
using Genode::align_addr;
using Genode::construct_at;
using Genode::error;
using Genode::log2;
using Genode::min;
using Genode::Out_of_caps;
using Genode::Out_of_ram;
using Genode::Range_allocator;
using Genode::warning;
using Genode::umword_t;

namespace {

    enum {
        MIN_CHUNK_SIZE = 4 * 1024,  /* in machine words */
        MAX_CHUNK_SIZE = 256 * 1024,
        /*
         * Allocation sizes >= this value are considered as big
         * allocations, which get their own dataspace. In contrast
         * to smaller allocations, this memory is released to
         * the RAM session when 'free()' is called.
         */
        BIG_ALLOCATION_THRESHOLD = 64 * 1024 /* in bytes */
    };
}


void Heap::Dataspace_pool::remove_and_free(Dataspace &ds) {
    /*
     * read dataspace capability and modify _ds_list before detaching
     * possible backing store for Dataspace - we rely on LIFO list
     * manipulation here!
     */

    Ram_dataspace_capability ds_cap = ds.cap;
    Region_map_address ds_local_addr = ds.local_addr;

    remove(&ds);

    /*
     * Call 'Dataspace' destructor to properly release the RAM dataspace
     * capabilities. Note that we don't free the 'Dataspace' object at the
     * local allocator because this is already done by the 'Heap'
     * destructor prior executing the 'Dataspace_pool' destructor.
     */
    ds.~Dataspace();

    region_map->detach(ds_local_addr);
    ram_alloc->free(ds_cap);
}


Heap::Dataspace_pool::~Dataspace_pool() {
    /* free all ram_dataspaces */
    for (Dataspace *ds; (ds = first());)
        remove_and_free(*ds);
}


int Heap::quota_limit(size_t new_quota_limit) {
    if (new_quota_limit < _quota_used) return -1;
    _quota_limit = new_quota_limit;
    return 0;
}


Heap::Alloc_ds_result
Heap::_allocate_dataspace(size_t size, bool use_local_addr, Region_map_address local_addr) {
    debug_log(LOG_PERSHEAP_ALLOC, __func__, "requested size = ", size);
    using Result = Alloc_ds_result;

    return _ds_pool.ram_alloc->try_alloc(size).convert<Result>(

            [&](Ram_dataspace_capability ds_cap) -> Result {

                struct Alloc_guard {
                    Ram_allocator &ram;
                    Ram_dataspace_capability ds;
                    bool keep = false;

                    Alloc_guard(Ram_allocator &ram, Ram_dataspace_capability ds)
                            : ram(ram), ds(ds) {}

                    ~Alloc_guard() { if (!keep) ram.free(ds); }

                } alloc_guard(*_ds_pool.ram_alloc, ds_cap);

                struct Attach_guard {
                    Region_map &rm;
                    struct {
                        void *ptr = nullptr;
                    };
                    bool keep = false;

                    Attach_guard(Region_map &rm) : rm(rm) {}

                    ~Attach_guard() { if (!keep && ptr) rm.detach(ptr); }

                } attach_guard(*_ds_pool.region_map);

                try {
                    attach_guard.ptr = _ds_pool.region_map->attach(ds_cap, 0, 0, use_local_addr, local_addr);
                }
                catch (Out_of_ram) { return Alloc_error::OUT_OF_RAM; }
                catch (Out_of_caps) { return Alloc_error::OUT_OF_CAPS; }
                catch (Region_map::Invalid_dataspace) { return Alloc_error::DENIED; }
                catch (Region_map::Region_conflict) { return Alloc_error::DENIED; }

                Alloc_result metadata = _md_alloc.try_alloc(sizeof(Heap::Dataspace));

                return metadata.convert<Result>(
                        [&](void *md_ptr) -> Result {
                            Dataspace &ds = *construct_at<Dataspace>(md_ptr, ds_cap,
                                                                     attach_guard.ptr, size);
                            _ds_pool.insert(&ds);
                            alloc_guard.keep = attach_guard.keep = true;
                            return &ds;
                        },
                        [&](Alloc_error error) {
                            return error;
                        });
            },
            [&](Alloc_error error) {
                return error;
            });
}


Allocator::Alloc_result Heap::_unsynchronized_alloc(size_t size) {
    size_t const dataspace_size = align_addr(size, 12);

    return _allocate_dataspace(dataspace_size).convert<Alloc_result>(

            [&](Dataspace *ds_ptr) {
                _quota_used += ds_ptr->size;
                debug_log(LOG_PERSHEAP_ALLOC, "rm addr ", ds_ptr->local_addr);
                debug_log(LOG_PERSHEAP_ALLOC, "local addr ", region_addr_to_local(ds_ptr->local_addr));
                return region_addr_to_local(ds_ptr->local_addr);
            },

            [&](Alloc_error error) {
                return error;
            });
}


Allocator::Alloc_result Heap::try_alloc(size_t size) {
    debug_log(LOG_PERSHEAP_ALLOC, "Heap::try_alloc, size = ", size);
    if (size == 0)
        error("attempt to allocate zero-size block from heap");

    /* serialize access of heap functions */
    Mutex::Guard guard(_mutex);

    /* check requested allocation against quota limit */
    if (size + _quota_used > _quota_limit)
        return Alloc_error::DENIED;

    return _unsynchronized_alloc(size);
}


void Heap::free(void *addr, size_t) {
    /* serialize access of heap functions */
    Mutex::Guard guard(_mutex);

    /*
     * Block could not be found in local allocator. So it is either a big
     * allocation or invalid address.
     */

    Region_map_address address = local_addr_to_region(addr);
    Heap::Dataspace *ds = nullptr;
    for (ds = _ds_pool.first(); ds; ds = ds->next())
        if (((addr_t) address >= (addr_t) ds->local_addr) &&
            ((addr_t) address <= (addr_t) ds->local_addr + ds->size - 1))
            break;

    if (!ds) {
        warning("heap could not free memory block");
        return;
    }

    _quota_used -= ds->size;

    _ds_pool.remove_and_free(*ds);
}


Heap::Heap(Ram_allocator *ram_alloc,
           Region_map *region_map,
           Allocator &md_alloc,
           addr_t rm_attach_addr,
           size_t quota_limit,
           void *static_addr,
           size_t static_size)
        :
        _md_alloc(md_alloc),
        _rm_attach_addr(Local_address(rm_attach_addr)),
        _ds_pool(ram_alloc, region_map),
        _quota_limit(quota_limit), _quota_used(0),
        _chunk_size(MIN_CHUNK_SIZE) {
}


Heap::~Heap() {
}

Allocator::Alloc_result Heap::alloc_addr(size_t size, Region_map_address addr) {

    /* serialize access of heap functions */
    Mutex::Guard guard(_mutex);

    size_t const dataspace_size = align_addr(size, 12);

    return _allocate_dataspace(dataspace_size, true, addr).convert<Alloc_result>(

            [&](Dataspace *ds_ptr) {
                _quota_used += ds_ptr->size;
                return ds_ptr->local_addr;
            },

            [&](Alloc_error error) {
                return error;
            });
}
