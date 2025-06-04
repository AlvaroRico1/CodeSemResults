static void index_entry_cpy(
	git_index_entry *tgt,
	const git_index_entry *src)
{
	const char *tgt_path = tgt->path;
	memcpy(tgt, src, sizeof(*tgt));
	tgt->path = tgt_path;
}


// Source: index.c
// Lines 1066-1073
