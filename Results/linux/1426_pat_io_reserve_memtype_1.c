int io_reserve_memtype(resource_size_t start, resource_size_t end,
			enum page_cache_mode *type)
{
	resource_size_t size = end - start;
	enum page_cache_mode req_type = *type;
	enum page_cache_mode new_type;
	int ret;

	WARN_ON_ONCE(iomem_map_sanity_check(start, size));

	ret = reserve_memtype(start, end, req_type, &new_type);
	if (ret)
		goto out_err;

	if (!is_new_memtype_allowed(start, size, req_type, new_type))
		goto out_free;

	if (kernel_map_sync_memtype(start, size, new_type) < 0)
		goto out_free;

	*type = new_type;
	return 0;

out_free:
	free_memtype(start, end);
	ret = -EBUSY;
out_err:
	return ret;
}


// Source: pat.c
// Lines 735-763
