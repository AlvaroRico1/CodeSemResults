static void write_completed_directory(struct merge_options *opt,
				      const char *new_directory_name,
				      struct directory_versions *info)
{
	const char *prev_dir;
	struct merged_info *dir_info = NULL;
	unsigned int offset;

	/*
	 * Some explanation of info->versions and info->offsets...
	 *
	 * process_entries() iterates over all relevant files AND
	 * directories in reverse lexicographic order, and calls this
	 * function.  Thus, an example of the paths that process_entries()
	 * could operate on (along with the directories for those paths
	 * being shown) is:
	 *
	 *     xtract.c             ""
	 *     tokens.txt           ""
	 *     src/moduleB/umm.c    src/moduleB
	 *     src/moduleB/stuff.h  src/moduleB
	 *     src/moduleB/baz.c    src/moduleB
	 *     src/moduleB          src
	 *     src/moduleA/foo.c    src/moduleA
	 *     src/moduleA/bar.c    src/moduleA
	 *     src/moduleA          src
	 *     src                  ""
	 *     Makefile             ""
	 *
	 * info->versions:
	 *
	 *     always contains the unprocessed entries and their
	 *     version_info information.  For example, after the first five
	 *     entries above, info->versions would be:
	 *
	 *     	   xtract.c     <xtract.c's version_info>
	 *     	   token.txt    <token.txt's version_info>
	 *     	   umm.c        <src/moduleB/umm.c's version_info>
	 *     	   stuff.h      <src/moduleB/stuff.h's version_info>
	 *     	   baz.c        <src/moduleB/baz.c's version_info>
	 *
	 *     Once a subdirectory is completed we remove the entries in
	 *     that subdirectory from info->versions, writing it as a tree
	 *     (write_tree()).  Thus, as soon as we get to src/moduleB,
	 *     info->versions would be updated to
	 *
	 *     	   xtract.c     <xtract.c's version_info>
	 *     	   token.txt    <token.txt's version_info>
	 *     	   moduleB      <src/moduleB's version_info>
	 *
	 * info->offsets:
	 *
	 *     helps us track which entries in info->versions correspond to
	 *     which directories.  When we are N directories deep (e.g. 4
	 *     for src/modA/submod/subdir/), we have up to N+1 unprocessed
	 *     directories (+1 because of toplevel dir).  Corresponding to
	 *     the info->versions example above, after processing five entries
	 *     info->offsets will be:
	 *
	 *     	   ""           0
	 *     	   src/moduleB  2
	 *
	 *     which is used to know that xtract.c & token.txt are from the
	 *     toplevel dirctory, while umm.c & stuff.h & baz.c are from the
	 *     src/moduleB directory.  Again, following the example above,
	 *     once we need to process src/moduleB, then info->offsets is
	 *     updated to
	 *
	 *     	   ""           0
	 *     	   src          2
	 *
	 *     which says that moduleB (and only moduleB so far) is in the
	 *     src directory.
	 *
	 *     One unique thing to note about info->offsets here is that
	 *     "src" was not added to info->offsets until there was a path
	 *     (a file OR directory) immediately below src/ that got
	 *     processed.
	 *
	 * Since process_entry() just appends new entries to info->versions,
	 * write_completed_directory() only needs to do work if the next path
	 * is in a directory that is different than the last directory found
	 * in info->offsets.
	 */

	/*
	 * If we are working with the same directory as the last entry, there
	 * is no work to do.  (See comments above the directory_name member of
	 * struct merged_info for why we can use pointer comparison instead of
	 * strcmp here.)
	 */
	if (new_directory_name == info->last_directory)
		return;

	/*
	 * If we are just starting (last_directory is NULL), or last_directory
	 * is a prefix of the current directory, then we can just update
	 * info->offsets to record the offset where we started this directory
	 * and update last_directory to have quick access to it.
	 */
	if (info->last_directory == NULL ||
	    !strncmp(new_directory_name, info->last_directory,
		     info->last_directory_len)) {
		uintptr_t offset = info->versions.nr;

		info->last_directory = new_directory_name;
		info->last_directory_len = strlen(info->last_directory);
		/*
		 * Record the offset into info->versions where we will
		 * start recording basenames of paths found within
		 * new_directory_name.
		 */
		string_list_append(&info->offsets,
				   info->last_directory)->util = (void*)offset;
		return;
	}

	/*
	 * The next entry that will be processed will be within
	 * new_directory_name.  Since at this point we know that
	 * new_directory_name is within a different directory than
	 * info->last_directory, we have all entries for info->last_directory
	 * in info->versions and we need to create a tree object for them.
	 */
	dir_info = strmap_get(&opt->priv->paths, info->last_directory);
	assert(dir_info);
	offset = (uintptr_t)info->offsets.items[info->offsets.nr-1].util;
	if (offset == info->versions.nr) {
		/*
		 * Actually, we don't need to create a tree object in this
		 * case.  Whenever all files within a directory disappear
		 * during the merge (e.g. unmodified on one side and
		 * deleted on the other, or files were renamed elsewhere),
		 * then we get here and the directory itself needs to be
		 * omitted from its parent tree as well.
		 */
		dir_info->is_null = 1;
	} else {
		/*
		 * Write out the tree to the git object directory, and also
		 * record the mode and oid in dir_info->result.
		 */
		dir_info->is_null = 0;
		dir_info->result.mode = S_IFDIR;
		write_tree(&dir_info->result.oid, &info->versions, offset,
			   opt->repo->hash_algo->rawsz);
	}

	/*
	 * We've now used several entries from info->versions and one entry
	 * from info->offsets, so we get rid of those values.
	 */
	info->offsets.nr--;
	info->versions.nr = offset;

	/*
	 * Now we've taken care of the completed directory, but we need to
	 * prepare things since future entries will be in
	 * new_directory_name.  (In particular, process_entry() will be
	 * appending new entries to info->versions.)  So, we need to make
	 * sure new_directory_name is the last entry in info->offsets.
	 */
	prev_dir = info->offsets.nr == 0 ? NULL :
		   info->offsets.items[info->offsets.nr-1].string;
	if (new_directory_name != prev_dir) {
		uintptr_t c = info->versions.nr;
		string_list_append(&info->offsets,
				   new_directory_name)->util = (void*)c;
	}

	/* And, of course, we need to update last_directory to match. */
	info->last_directory = new_directory_name;
	info->last_directory_len = strlen(info->last_directory);
}

/* Per entry merge function */
static void process_entry(struct merge_options *opt,
			  const char *path,
			  struct conflict_info *ci,
			  struct directory_versions *dir_metadata)
{
	int df_file_index = 0;

	VERIFY_CI(ci);
	assert(ci->filemask >= 0 && ci->filemask <= 7);
	/* ci->match_mask == 7 was handled in collect_merge_info_callback() */
	assert(ci->match_mask == 0 || ci->match_mask == 3 ||
	       ci->match_mask == 5 || ci->match_mask == 6);

	if (ci->dirmask) {
		record_entry_for_tree(dir_metadata, path, &ci->merged);
		if (ci->filemask == 0)
			/* nothing else to handle */
			return;
		assert(ci->df_conflict);
	}

	if (ci->df_conflict && ci->merged.result.mode == 0) {
		int i;

		/*
		 * directory no longer in the way, but we do have a file we
		 * need to place here so we need to clean away the "directory
		 * merges to nothing" result.
		 */
		ci->df_conflict = 0;
		assert(ci->filemask != 0);
		ci->merged.clean = 0;
		ci->merged.is_null = 0;
		/* and we want to zero out any directory-related entries */
		ci->match_mask = (ci->match_mask & ~ci->dirmask);
		ci->dirmask = 0;
		for (i = MERGE_BASE; i <= MERGE_SIDE2; i++) {
			if (ci->filemask & (1 << i))
				continue;
			ci->stages[i].mode = 0;
			oidcpy(&ci->stages[i].oid, null_oid());
		}


// Source: merge-ort.c
// Lines 3376-3593
