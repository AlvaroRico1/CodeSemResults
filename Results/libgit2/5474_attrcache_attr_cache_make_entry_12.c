static int attr_cache_make_entry(
	git_attr_file_entry **out, git_repository *repo, const char *path)
{
	git_attr_cache *cache = git_repository_attr_cache(repo);
	git_attr_file_entry *entry = NULL;
	int error;

	if ((error = git_attr_cache__alloc_file_entry(&entry, repo,
		git_repository_workdir(repo), path, &cache->pool)) < 0)
		return error;

	if ((error = git_strmap_set(cache->files, entry->path, entry)) < 0)
		return error;

	*out = entry;
	return error;
}


// Source: attrcache.c
// Lines 84-100
