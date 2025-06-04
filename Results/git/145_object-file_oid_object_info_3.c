int oid_object_info(struct repository *r,
		    const struct object_id *oid,
		    unsigned long *sizep)
{
	enum object_type type;
	struct object_info oi = OBJECT_INFO_INIT;

	oi.typep = &type;
	oi.sizep = sizep;
	if (oid_object_info_extended(r, oid, &oi,
				      OBJECT_INFO_LOOKUP_REPLACE) < 0)
		return -1;
	return type;
}


// Source: object-file.c
// Lines 1601-1614
