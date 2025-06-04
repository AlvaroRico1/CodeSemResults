int nfs_map_group_to_gid(const struct nfs_server *server, const char *name, size_t namelen, kgid_t *gid)
{
	struct idmap *idmap = server->nfs_client->cl_idmap;
	__u32 id = -1;
	int ret = 0;

	if (!nfs_map_string_to_numeric(name, namelen, &id))
		ret = nfs_idmap_lookup_id(name, namelen, "gid", &id, idmap);
	if (ret == 0) {
		*gid = make_kgid(idmap_userns(idmap), id);
		if (!gid_valid(*gid))
			ret = -ERANGE;
	}
	trace_nfs4_map_group_to_gid(name, namelen, id, ret);
	return ret;
}


// Source: nfs4idmap.c
// Lines 759-774
