#include "dpdk_memcached.hh"
#include "dpdk_basic.hh"
#include "dpdk_error.hh"


memcached_return_t initialize_query(Memcached *self, bool increment_query_id)
{
    if (self == NULL)
    {
        return MEMCACHED_INVALID_ARGUMENTS;
    }

    if (increment_query_id)
    {
        self->query_id++;
    }

    if (self->state.is_time_for_rebuild)
    {
        memcached_reset(self);
    }

    memcached_error_free(*self);
    memcached_result_reset(&self->result);

    return MEMCACHED_SUCCESS;
}

struct memcached_array_st
{
    Memcached *root;
    size_t size;
    char c_str[];
};

size_t memcached_array_size(memcached_array_st *array)
{
    if (array)
    {
        return array->size;
    }

    return 0;
}

const char *memcached_array_string(memcached_array_st *array)
{
    if (array)
    {
        return array->c_str;
    }

    return NULL;
}


uint32_t hashkit_digest(const hashkit_st *self, const char *key, size_t key_length)
{
    return self->base_hash.function(key, key_length, self->base_hash.context);
}


static inline uint32_t generate_hash(const Memcached *ptr, const char *key, size_t key_length)
{
    return hashkit_digest(&ptr->hashkit, key, key_length);
}



static inline uint32_t _generate_hash_wrapper(const Memcached *ptr, const char *key, size_t key_length)
{
    if (memcached_server_count(ptr) == 1)
        return 0;

    if (ptr->flags.hash_with_namespace)
    {
        size_t temp_length= memcached_array_size(ptr->_namespace) + key_length;
        char temp[MEMCACHED_MAX_KEY];

        if (temp_length > MEMCACHED_MAX_KEY -1)
            return 0;

        strncpy(temp, memcached_array_string(ptr->_namespace), memcached_array_size(ptr->_namespace));
        strncpy(temp + memcached_array_size(ptr->_namespace), key, key_length);

        return generate_hash(ptr, temp, temp_length);
    }
    else
    {
        return generate_hash(ptr, key, key_length);
    }
}

uint32_t memcached_generate_hash_with_redistribution(memcached_st *ptr, const char *key, size_t key_length)
{
    uint32_t hash= _generate_hash_wrapper(ptr, key, key_length);

    _regen_for_auto_eject(ptr);

    return dispatch_host(ptr, hash);
}



static inline memcached_return_t memcached_send(memcached_st *shell,
                                                const char *group_key, size_t group_key_length,
                                                const char *key, size_t key_length,
                                                const char *value, size_t value_length,
                                                const time_t expiration,
                                                const uint32_t flags,
                                                const uint64_t cas,
                                                memcached_storage_action_t verb)
{
    Memcached* ptr= memcached2Memcached(shell);
    memcached_return_t rc;
    if (memcached_failed(rc= initialize_query(ptr, true)))
    {
        return rc;
    }

    uint32_t server_key= memcached_generate_hash_with_redistribution(ptr, group_key, group_key_length);
//    memcached_instance_st* instance= memcached_instance_fetch(ptr, server_key);
//
//    WATCHPOINT_SET(instance->io_wait_count.read= 0);
//    WATCHPOINT_SET(instance->io_wait_count.write= 0);
//
//    bool flush= true;
//    if (memcached_is_buffering(instance->root) and verb == SET_OP)
//    {
//        flush= false;
//    }
//
//    bool reply= memcached_is_replying(ptr);
//
//    hashkit_string_st* destination= NULL;
//
//    if (memcached_is_encrypted(ptr))
//    {
//        if (can_by_encrypted(verb) == false)
//        {
//            return memcached_set_error(*ptr, MEMCACHED_NOT_SUPPORTED, MEMCACHED_AT,
//                                       memcached_literal_param("Operation not allowed while encyrption is enabled"));
//        }
//
//        if ((destination= hashkit_encrypt(&ptr->hashkit, value, value_length)) == NULL)
//        {
//            return rc;
//        }
//        value= hashkit_string_c_str(destination);
//        value_length= hashkit_string_length(destination);
//    }
//
//    if (memcached_is_binary(ptr))
//    {
//        rc= memcached_send_binary(ptr, instance, server_key,
//                                  key, key_length,
//                                  value, value_length, expiration,
//                                  flags, cas, flush, reply, verb);
//    }
//    else
//    {
//        rc= memcached_send_ascii(ptr, instance,
//                                 key, key_length,
//                                 value, value_length, expiration,
//                                 flags, cas, flush, reply, verb);
//    }
//
//    hashkit_string_free(destination);

    return rc;
}



memcached_return_t dpdk_memcached_set(memcached_st *ptr, const char *key, size_t key_length,
                                 const char *value, size_t value_length,
                                 time_t expiration,
                                 uint32_t flags)
{
    std::cout << "dpdk_memcached_set BEGIN" << std::endl;
    memcached_return_t rc;
    LIBMEMCACHED_MEMCACHED_SET_START();
    rc= memcached_send(ptr, key, key_length,
                       key, key_length, value, value_length,
                       expiration, flags, 0, SET_OP);
    LIBMEMCACHED_MEMCACHED_SET_END();

    std::cout << "dpdk_memcached_set END" << std::endl;
    return rc;
}

