int oid_object_info_extended(struct repository *r, const struct object_id *oid,
			     struct object_info *oi, unsigned flags)
{
	int ret;
	obj_read_lock();
	ret = do_oid_object_info_extended(r, oid, oi, flags);
	obj_read_unlock();
	return ret;
}


// Source: object-file.c
// Lines 1589-1597
