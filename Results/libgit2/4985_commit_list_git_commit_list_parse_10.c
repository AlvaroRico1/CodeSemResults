int git_commit_list_parse(git_revwalk *walk, git_commit_list_node *commit)
{
	git_odb_object *obj;
	git_commit_graph_file *cgraph_file = NULL;
	int error;

	if (commit->parsed)
		return 0;

	/* Let's try to use the commit graph first. */
	git_odb__get_commit_graph_file(&cgraph_file, walk->odb);
	if (cgraph_file) {
		git_commit_graph_entry e;

		error = git_commit_graph_entry_find(&e, cgraph_file, &commit->oid, GIT_OID_RAWSZ);
		if (error == 0 && git__is_uint16(e.parent_count)) {
			size_t i;
			commit->generation = (uint32_t)e.generation;
			commit->time = e.commit_time;
			commit->out_degree = (uint16_t)e.parent_count;
			commit->parents = alloc_parents(walk, commit, commit->out_degree);
			GIT_ERROR_CHECK_ALLOC(commit->parents);

			for (i = 0; i < commit->out_degree; ++i) {
				git_commit_graph_entry parent;
				error = git_commit_graph_entry_parent(&parent, cgraph_file, &e, i);
				if (error < 0)
					return error;
				commit->parents[i] = git_revwalk__commit_lookup(walk, &parent.sha1);
			}
			commit->parsed = 1;
			return 0;
		}
	}

	if ((error = git_odb_read(&obj, walk->odb, &commit->oid)) < 0)
		return error;

	if (obj->cached.type != GIT_OBJECT_COMMIT) {
		git_error_set(GIT_ERROR_INVALID, "object is no commit object");
		error = -1;
	} else
		error = commit_quick_parse(walk, commit, obj);

	git_odb_object_free(obj);
	return error;
}


// Source: commit_list.c
// Lines 161-207
