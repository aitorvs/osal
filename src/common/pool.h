#ifndef __POOL__POOL_H__
#define __POOL__POOL_H__

struct s_pool {

    uint8_t * memory_area;

    uint32_t memory_area_size;

    uint32_t data_size;

    uint32_t free_blocks;

    void * free_blocks_list;
    

};

#define MINIMUM_ELEMENT_SIZE 4

static inline void pool_init(struct s_pool * pool)
{
    int i;
    for (i = 0; i < sizeof(struct s_pool); i++)
        *(((uint8_t *)pool) + i) = 0;
}

/*
 * This function initializes the memory pool. The user must provide the memory
 * area (statically allocated), its size, and the size of the data. The size of
 * the indexed data may be the actual size of the structures or another
 * alignment. The assigned block will be assigned in addresses multiple of
 * <data_size>
 */

static inline void pool_init_memory(struct s_pool * pool,
                                    uint8_t * address,
                                    uint32_t memsize,
                                    uint32_t datasize)
{
    int i;
    void * ptr;

    pool->memory_area = address;
    pool->memory_area_size = memsize;
    pool->data_size = (datasize > MINIMUM_ELEMENT_SIZE) ? datasize : 
                                                          MINIMUM_ELEMENT_SIZE;

    pool->free_blocks_list = ptr = (void *)address;
    pool->free_blocks = memsize/datasize;

    for (i = 0; i < pool->free_blocks - 1; i++)
    {
        *((int *)ptr) = ((int)(ptr + datasize));
        ptr = ptr + datasize;
    }
    *((int *)ptr) = 0;

}

static inline void * pool_alloc_elem(struct s_pool * pool)
{
    void * ptr;

    ptr = pool->free_blocks_list;
    if (pool->free_blocks > 0)
    {
        pool->free_blocks_list = (void *)(*((int *)pool->free_blocks_list));
        pool->free_blocks--;
    }
    return ptr;

}

static inline void * pool_zalloc_elem(struct s_pool * pool)
{
    void * ptr;
    int i;
    ptr = pool->free_blocks_list;

    if (pool->free_blocks > 0)
    {
        pool->free_blocks_list = (void *)(*((int *)pool->free_blocks_list));
        pool->free_blocks--;
        for (i = 0; i < pool->data_size; i++)
        {
            *(((uint8_t *)ptr) + i) = 0;
        }
    }
    return ptr;
}

static inline uint32_t pool_alloc_nelem(struct s_pool * pool, void * ptrarray[], uint32_t num_elements)
{
    int allocated;
    int elements;

    for (elements = num_elements, allocated = 0; 
        ((pool->free_blocks > 0) && (elements > 0));
         elements--, allocated++, pool->free_blocks--)
    {
        ptrarray[allocated] = pool->free_blocks_list;
        pool->free_blocks_list = (void *)(*((int *)pool->free_blocks_list));
    }
    return allocated;

}

static inline uint32_t pool_zalloc_nelem(struct s_pool * pool, void * ptrarray[], uint32_t num_elements)
{
    int i;
    int allocated;
    int elements;

    for (elements = num_elements, allocated = 0; 
        ((pool->free_blocks > 0) && (elements > 0));
         elements--, allocated++, pool->free_blocks--)
    {
        ptrarray[allocated] = pool->free_blocks_list;
        pool->free_blocks_list = (void *)(*((int *)pool->free_blocks_list));
        for (i = 0; i < pool->data_size; i++)
        {
            *(((uint8_t *)ptrarray[allocated]) + i) = 0;
        }
    }
    return allocated;
}


static inline void pool_free_elem(struct s_pool * pool, void * elem)
{
    void * ptr = elem;
    // Sanity check
    if (unlikely(((int)elem >= (int)pool->memory_area) &&
       ((int)elem <= (int)(pool->memory_area + pool->memory_area_size))))
    {
        *((int *)ptr) = (int)pool->free_blocks_list;

        pool->free_blocks_list = ptr;
        pool->free_blocks++;
    }
}

static inline void pool_zfree_elem(struct s_pool * pool, void * elem)
{
    void * ptr = elem;
    int i;
    // Sanity check
    if (unlikely(((int)elem >= (int)pool->memory_area) &&
       ((int)elem <= (int)(pool->memory_area + pool->memory_area_size))))
    {

        for (i = 0; i < pool->data_size; i++)
            *(((uint8_t *)ptr) + i) = 0;
        *((int *)ptr) = (int)pool->free_blocks_list;

        pool->free_blocks_list = ptr;
        pool->free_blocks++;
    }
}

static inline void pool_free_nelem(struct s_pool * pool, void * ptrarray[], uint32_t num_elements)
{
    int i;
    // Sanity check
    for (i = 0; i < num_elements; i++) 
    {
        if (unlikely(((int)ptrarray[i] >= (int)pool->memory_area) &&
           ((int)ptrarray[i] <= (int)(pool->memory_area + pool->memory_area_size))))
        {
            *((int *)ptrarray[i]) = (int)pool->free_blocks_list;

            pool->free_blocks_list = ptrarray[i];
            pool->free_blocks++;
        }
    }
}

static inline void pool_zfree_nelem(struct s_pool * pool, void * ptrarray[], uint32_t num_elements)
{
    int i, j;
    // Sanity check
    for (i = 0; i < num_elements; i++) 
    {
        if (unlikely(((int)ptrarray[i] >= (int)pool->memory_area) &&
           ((int)ptrarray[i] <= (int)(pool->memory_area + pool->memory_area_size))))
        {
            for (j = 0; j < pool->data_size; j++)
                *(((uint8_t *)ptrarray[i]) + j) = 0;

            *((int *)ptrarray[i]) = (int)pool->free_blocks_list;

            pool->free_blocks_list = ptrarray[i];
            pool->free_blocks++;
        }
    }
}

static inline uint32_t pool_num_free_elements(struct s_pool * pool)
{
    return pool->free_blocks;
}

#endif // __POOL__POOL_H__
