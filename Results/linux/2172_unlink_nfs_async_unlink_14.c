nfs_async_unlink(struct dentry *dentry, const struct qstr *name)
{
	struct nfs_unlinkdata *data;
	int status = -ENOMEM;
	void *devname_garbage = NULL;

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (data == NULL)
		goto out;
	data->args.name.name = kstrdup(name->name, GFP_KERNEL);
	if (!data->args.name.name)
		goto out_free;
	data->args.name.len = name->len;

	data->cred = get_current_cred();
	data->res.dir_attr = &data->dir_attr;
	init_waitqueue_head(&data->wq);

	status = -EBUSY;
	spin_lock(&dentry->d_lock);
	if (dentry->d_flags & DCACHE_NFSFS_RENAMED)
		goto out_unlock;
	dentry->d_flags |= DCACHE_NFSFS_RENAMED;
	devname_garbage = dentry->d_fsdata;
	dentry->d_fsdata = data;
	spin_unlock(&dentry->d_lock);
	/*
	 * If we'd displaced old cached devname, free it.  At that
	 * point dentry is definitely not a root, so we won't need
	 * that anymore.
	 */
	kfree(devname_garbage);
	return 0;
out_unlock:
	spin_unlock(&dentry->d_lock);
	put_cred(data->cred);
	kfree(data->args.name.name);
out_free:
	kfree(data);
out:
	return status;
}


// Source: unlink.c
// Lines 167-208
