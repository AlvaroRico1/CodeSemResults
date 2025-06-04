static void *lazy_dir_thread_proc(void *_data)
{
	struct lazy_dir_thread_data *d = _data;
	struct strbuf prefix = STRBUF_INIT;
	handle_range_1(d->istate, d->k_start, d->k_end, NULL, &prefix, d->lazy_entries);
	strbuf_release(&prefix);
	return NULL;
}


// Source: name-hash.c
// Lines 462-469
