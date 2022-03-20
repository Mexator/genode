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
    using Genode::Ram_dataspace_capability;
    using Genode::Ram_allocator;
    using Genode::Reconstructible;
    using Genode::Region_map;
    using Genode::size_t;

    class Heap;
}

/**
 * Copy of Genode::Heap modified to save/restore memory state
 */
class Persalloc::Heap : public Genode::Allocator
{
private:

    class Dataspace : public List<Dataspace>::Element
    {
    private:

        /*
         * Noncopyable
         */
        Dataspace(Dataspace const &);
        Dataspace &operator = (Dataspace const &);

    public:

        Ram_dataspace_capability cap;
        void  *local_addr;
        size_t size;

        Dataspace(Ram_dataspace_capability c, void *local_addr, size_t size)
                : cap(c), local_addr(local_addr), size(size) { }
    };

    /*
     * This structure exists only to make sure that the dataspaces are
     * destroyed after the AVL allocator.
     */
    class Dataspace_pool : public List<Dataspace>
    {
    private:

        /*
         * Noncopyable
         */
        Dataspace_pool(Dataspace_pool const &);
        Dataspace_pool &operator = (Dataspace_pool const &);

    public:

        Ram_allocator *ram_alloc; /* backing store */
        Region_map    *region_map;

        Dataspace_pool(Ram_allocator *ram, Region_map *rm)
                : ram_alloc(ram), region_map(rm) { }

        ~Dataspace_pool();

        void remove_and_free(Dataspace &);

        void reassign_resources(Ram_allocator *ram, Region_map *rm) {
            ram_alloc = ram, region_map = rm; }
    };

    Mutex                  mutable _mutex { };
    Reconstructible<Allocator_avl> _alloc;        /* local allocator    */
    Dataspace_pool                 _ds_pool;      /* list of dataspaces */
    size_t                         _quota_limit { 0 };
    size_t                         _quota_used  { 0 };
    size_t                         _chunk_size  { 0 };

    using Alloc_ds_result = Attempt<Dataspace *, Alloc_error>;

    /**
     * Allocate a new dataspace of the specified size
     *
     * \param size                       number of bytes to allocate
     * \param enforce_separate_metadata  if true, the new dataspace
     *                                   will not contain any meta data
     *
     * \param local_addr                if not 0, will try to attach dataspace at local address
     */
    Alloc_ds_result _allocate_dataspace(size_t size, bool enforce_separate_metadata, Genode::addr_t local_addr = 0);

    /**
     * Try to allocate block at our local allocator
     */
    Alloc_result _try_local_alloc(size_t size);

    /**
     * Unsynchronized implementation of 'try_alloc'
     */
    Alloc_result _unsynchronized_alloc(size_t size);

public:

    enum { UNLIMITED = ~0 };

    Heap(Ram_allocator *ram_allocator,
         Region_map    *region_map,
         size_t         quota_limit = UNLIMITED,
         void          *static_addr = 0,
         size_t         static_size = 0);

    Heap(Ram_allocator &ram, Region_map &rm) : Heap(&ram, &rm) { }

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
        _ds_pool.reassign_resources(ram, rm); }

    /**
     * Call 'fn' with the start and size of each backing-store region
     */
    template <typename FN>
    void for_each_region(FN const &fn) const
    {
        Mutex::Guard guard(_mutex);
        for (Dataspace const *ds = _ds_pool.first(); ds; ds = ds->next())
            fn(ds->local_addr, ds->size);
    }

    /*****************
     ** My bullshit **
     *****************/

    Alloc_result alloc_addr(size_t size, addr_t addr);

    /*************************
     ** Allocator interface **
     *************************/

    Alloc_result try_alloc(size_t)           override;
    void         free(void *, size_t)        override;
    size_t       consumed()            const override { return _quota_used; }
    size_t       overhead(size_t size) const override { return _alloc->overhead(size); }
    bool         need_size_for_free()  const override { return false; }
};


#endif //_PTCP_PERS_HEAP_H_
