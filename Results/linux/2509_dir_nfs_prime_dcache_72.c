void nfs_prime_dcache(struct dentry *parent, struct nfs_entry *entry)
{
	struct qstr filename = QSTR_INIT(entry->name, entry->len);
	DECLARE_WAIT_QUEUE_HEAD_ONSTACK(wq);
	struct dentry *dentry;
	struct dentry *alias;
	struct inode *dir = d_inode(parent);
	struct inode *inode;
	int status;

	if (!(entry->fattr->valid & NFS_ATTR_FATTR_FILEID))
		return;
	if (!(entry->fattr->valid & NFS_ATTR_FATTR_FSID))
		return;
	if (filename.len == 0)
		return;
	/* Validate that the name doesn't contain any illegal '\0' */
	if (strnlen(filename.name, filename.len) != filename.len)
		return;
	/* ...or '/' */
	if (strnchr(filename.name, filename.len, '/'))
		return;
	if (filename.name[0] == '.') {
		if (filename.len == 1)
			return;
		if (filename.len == 2 && filename.name[1] == '.')
			return;
	}
	filename.hash = full_name_hash(parent, filename.name, filename.len);

	dentry = d_lookup(parent, &filename);
again:
	if (!dentry) {
		dentry = d_alloc_parallel(parent, &filename, &wq);
		if (IS_ERR(dentry))
			return;
	}
	if (!d_in_lookup(dentry)) {
		/* Is there a mountpoint here? If so, just exit */
		if (!nfs_fsid_equal(&NFS_SB(dentry->d_sb)->fsid,
					&entry->fattr->fsid))
			goto out;
		if (nfs_same_file(dentry, entry)) {
			if (!entry->fh->size)
				goto out;
			nfs_set_verifier(dentry, nfs_save_change_attribute(dir));
			status = nfs_refresh_inode(d_inode(dentry), entry->fattr);
			if (!status)
				nfs_setsecurity(d_inode(dentry), entry->fattr, entry->label);
			goto out;
		} else {
			d_invalidate(dentry);
			dput(dentry);
			dentry = NULL;
			goto again;
		}
	}
	if (!entry->fh->size) {
		d_lookup_done(dentry);
		goto out;
	}

	inode = nfs_fhget(dentry->d_sb, entry->fh, entry->fattr, entry->label);
	alias = d_splice_alias(inode, dentry);
	d_lookup_done(dentry);
	if (alias) {
		if (IS_ERR(alias))
			goto out;
		dput(dentry);
		dentry = alias;
	}
	nfs_set_verifier(dentry, nfs_save_change_attribute(dir));
out:
	dput(dentry);
}


// Source: dir.c
// Lines 445-519
