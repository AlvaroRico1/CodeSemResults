void enable_obj_read_lock(void)
{
	if (obj_read_use_lock)
		return;

	obj_read_use_lock = 1;
	init_recursive_mutex(&obj_read_mutex);
}


// Source: object-file.c
// Lines 1472-1479
