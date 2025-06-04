static char *nfs4_path(struct dentry *dentry, char *buffer, ssize_t buflen)
{
	char *limit;
	char *path = nfs_path(&limit, dentry, buffer, buflen,
			      NFS_PATH_CANONICAL);
	if (!IS_ERR(path)) {
		char *path_component = nfs_path_component(path, limit);
		if (path_component)
			return path_component;
	}
	return path;
}


// Source: nfs4namespace.c
// Lines 84-95
