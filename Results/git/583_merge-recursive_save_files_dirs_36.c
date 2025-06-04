static int save_files_dirs(const struct object_id *oid,
			   struct strbuf *base, const char *path,
			   unsigned int mode, void *context)
{
	struct path_hashmap_entry *entry;
	int baselen = base->len;
	struct merge_options *opt = context;

	strbuf_addstr(base, path);

	FLEX_ALLOC_MEM(entry, path, base->buf, base->len);
	hashmap_entry_init(&entry->e, fspathhash(entry->path));
	hashmap_add(&opt->priv->current_file_dir_set, &entry->e);

	strbuf_setlen(base, baselen);
	return (S_ISDIR(mode) ? READ_TREE_RECURSIVE : 0);
}


// Source: merge-recursive.c
// Lines 451-467
