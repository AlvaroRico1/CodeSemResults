static void attr_cache__free(git_attr_cache *cache)
{
	bool unlock;

	if (!cache)
		return;

	unlock = (attr_cache_lock(cache) == 0);

	if (cache->files != NULL) {
		git_attr_file_entry *entry;
		git_attr_file *file;
		int i;

		git_strmap_foreach_value(cache->files, entry, {
			for (i = 0; i < GIT_ATTR_FILE_NUM_SOURCES; ++i) {
				if ((file = git_atomic_swap(entry->file[i], NULL)) != NULL) {
					GIT_REFCOUNT_OWN(file, NULL);
					git_attr_file__free(file);
				}
			}
		});
		git_strmap_free(cache->files);
	}

	if (cache->macros != NULL) {
		git_attr_rule *rule;

		git_strmap_foreach_value(cache->macros, rule, {
			git_attr_rule__free(rule);
		});
		git_strmap_free(cache->macros);
	}


// Source: attrcache.c
// Lines 319-351
