struct dentry *nfs_fs_mount_common(struct nfs_server *server,
				   int flags, const char *dev_name,
				   struct nfs_mount_info *mount_info,
				   struct nfs_subversion *nfs_mod)
{
	struct super_block *s;
	struct dentry *mntroot = ERR_PTR(-ENOMEM);
	int (*compare_super)(struct super_block *, void *) = nfs_compare_super;
	struct nfs_sb_mountdata sb_mntdata = {
		.mntflags = flags,
		.server = server,
	};
	int error;

	if (server->flags & NFS_MOUNT_UNSHARED)
		compare_super = NULL;

	/* -o noac implies -o sync */
	if (server->flags & NFS_MOUNT_NOAC)
		sb_mntdata.mntflags |= SB_SYNCHRONOUS;

	if (mount_info->cloned != NULL && mount_info->cloned->sb != NULL)
		if (mount_info->cloned->sb->s_flags & SB_SYNCHRONOUS)
			sb_mntdata.mntflags |= SB_SYNCHRONOUS;

	/* Get a superblock - note that we may end up sharing one that already exists */
	s = sget(nfs_mod->nfs_fs, compare_super, nfs_set_super, flags, &sb_mntdata);
	if (IS_ERR(s)) {
		mntroot = ERR_CAST(s);
		goto out_err_nosb;
	}

	if (s->s_fs_info != server) {
		nfs_free_server(server);
		server = NULL;
	} else {
		error = super_setup_bdi_name(s, "%u:%u", MAJOR(server->s_dev),
					     MINOR(server->s_dev));
		if (error) {
			mntroot = ERR_PTR(error);
			goto error_splat_super;
		}
		s->s_bdi->ra_pages = server->rpages * NFS_MAX_READAHEAD;
		server->super = s;
	}

	if (!s->s_root) {
		/* initial superblock/root creation */
		mount_info->fill_super(s, mount_info);
		nfs_get_cache_cookie(s, mount_info->parsed, mount_info->cloned);
		if (!(server->flags & NFS_MOUNT_UNSHARED))
			s->s_iflags |= SB_I_MULTIROOT;
	}

	mntroot = nfs_get_root(s, mount_info->mntfh, dev_name);
	if (IS_ERR(mntroot))
		goto error_splat_super;

	error = mount_info->set_security(s, mntroot, mount_info);
	if (error)
		goto error_splat_root;

	s->s_flags |= SB_ACTIVE;

out:
	return mntroot;

out_err_nosb:
	nfs_free_server(server);
	goto out;

error_splat_root:
	dput(mntroot);
	mntroot = ERR_PTR(error);
error_splat_super:
	deactivate_locked_super(s);
	goto out;
}


// Source: super.c
// Lines 2630-2707
