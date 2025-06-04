static const struct object_id *oid_access(size_t index, const void *table)
{
	const struct object_id *array = table;
	return &array[index];
}


// Source: oid-array.c
// Lines 25-29
