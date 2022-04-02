#ifndef _PTCP_PERS_HEAP_H_
#define _PTCP_PERS_HEAP_H_

#include <base/allocator.h>
#include <base/allocator_avl.h>
#include <region_map/region_map.h>
#include <util/list.h>
#include <util/reconstructible.h>

namespace Persalloc {
    using Genode::addr_t;
    using Genode::List;
    using Genode::Mutex;
    using Genode::Allocator;
    using Genode::Allocator_avl;
    using Genode::Attempt;
    using Genode::Noncopyable;
    using Genode::Ram_dataspace_capability;
    using Genode::Ram_allocator;
    using Genode::Reconstructible;
    using Genode::Region_map;
    using Genode::size_t;
    using Genode::Tslab;

    class Heap;
}

/**
 * Copy of Genode::Heap modified to save/restore memory state
 */
class Persalloc::Heap : public Genode::Allocator {
public:
    // Address in component's address space
    typedef void *Local_address;
    // Address inside a region map that this heap uses
    typedef void *Region_map_address;

    /**
     * Metadata used to track allocated addresses within a region map
     */
    class Dataspace : public List<Dataspace>::Element, Noncopyable {

    public:

        Ram_dataspace_capability cap;
        Region_map_address local_addr;
        size_t size;

        Dataspace(Ram_dataspace_capability c, Heap::Region_map_address local_addr, size_t size)
                : cap(c), local_addr(local_addr), size(size) {}
    };

    /*
     * This structure exists only to make sure that the dataspaces are
     * destroyed after the AVL allocator.
     */
    class Dataspace_pool : public List<Dataspace>, Noncopyable {

    public:

        Ram_allocator *ram_alloc; /* backing store */
        Region_map *region_map;

        Dataspace_pool(Ram_allocator *ram, Region_map *rm)
                : ram_alloc(ram), region_map(rm) {}

        ~Dataspace_pool();

        void remove_and_free(Dataspace &);

        void reassign_resources(Ram_allocator *ram, Region_map *rm) {
            ram_alloc = ram, region_map = rm;
        }
    };

private:
    Mutex mutable _mutex{};
    Allocator &_md_alloc;     /* Allocator to alloc metadata */
    Local_address _rm_attach_addr; /* Address of region_map within component's address space */
    Dataspace_pool _ds_pool;      /* list of dataspaces */
    size_t _quota_limit{0};
    size_t _quota_used{0};
    size_t _chunk_size{0};

    using Alloc_ds_result = Attempt<Dataspace *, Alloc_error>;

    /**
     * Allocate a new dataspace of the specified size
     *
     * \param size                       number of bytes to allocate
     * \param use_local_addr             if true, dataspace will be attached at local_addr
     * \param local_addr
     */
    Alloc_ds_result _allocate_dataspace(size_t size, bool use_local_addr = false, Region_map_address local_addr = 0);

    /**
     * Unsynchronized implementation of 'try_alloc'
     */
    Alloc_result _unsynchronized_alloc(size_t size);

public:

    enum {
        UNLIMITED = ~0
    };

    Heap(Ram_allocator *ram_allocator,
         Region_map *region_map,
         Allocator &md_alloc,
         addr_t rm_attach_addr,
         size_t quota_limit = UNLIMITED,
         void *static_addr = 0,
         size_t static_size = 0);

    Heap(Ram_allocator &ram, Region_map &rm, Allocator &md_alloc, addr_t rm_attach_addr) :
            Heap(&ram, &rm, md_alloc, rm_attach_addr) {}

    ~Heap();

    /**
     * Reconfigure quota limit
     *
     * \return  negative error code if new quota limit is higher than
     *          currently used quota.
     */
    int quota_limit(size_t new_quota_limit);

    /**
     * Re-assign RAM allocator and region map
     */
    void reassign_resources(Ram_allocator *ram, Region_map *rm) {
        _ds_pool.reassign_resources(ram, rm);
    }

    /**
     * Call 'fn' with the start and size of each backing-store region
     */
    template<typename FN>
    void for_each_region(FN const &fn) const {
        Mutex::Guard guard(_mutex);
        for (Dataspace const *ds = _ds_pool.first(); ds; ds = ds->next())
            fn(ds->local_addr, ds->size);
    }

    /********************************
     ** Persistence heap additions **
     *******************************/

    /**
     * Allocates size bytes at addr. Later this area to be populated
     * with previously saved memory
     */
    Alloc_result alloc_addr(size_t size, Region_map_address addr, bool add_to_local);

    /**
     * Helper for debugging
     */
    Dataspace_pool &ds_pool() { return _ds_pool; }

    /* Helpers to map addresses to each other */
    Local_address region_addr_to_local(Region_map_address reg) {
        return Local_address(addr_t(_rm_attach_addr) + addr_t(reg));
    }

    Region_map_address local_addr_to_region(Local_address loc) {
        return Region_map_address(addr_t(loc) - addr_t(_rm_attach_addr));
    }

    /*************************
     ** Allocator interface **
     *************************/

    Alloc_result try_alloc(size_t) override;

    void free(void *, size_t) override;

    size_t consumed() const override { return _quota_used; }

    size_t overhead(size_t size) const override { return sizeof(Dataspace); }

    bool need_size_for_free() const override { return false; }
};


#endif //_PTCP_PERS_HEAP_H_
