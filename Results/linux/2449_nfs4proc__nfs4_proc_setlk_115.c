static int _nfs4_proc_setlk(struct nfs4_state *state, int cmd, struct file_lock *request)
{
	struct nfs_inode *nfsi = NFS_I(state->inode);
	struct nfs4_state_owner *sp = state->owner;
	unsigned char fl_flags = request->fl_flags;
	int status;

	request->fl_flags |= FL_ACCESS;
	status = locks_lock_inode_wait(state->inode, request);
	if (status < 0)
		goto out;
	mutex_lock(&sp->so_delegreturn_mutex);
	down_read(&nfsi->rwsem);
	if (test_bit(NFS_DELEGATED_STATE, &state->flags)) {
		/* Yes: cache locks! */
		/* ...but avoid races with delegation recall... */
		request->fl_flags = fl_flags & ~FL_SLEEP;
		status = locks_lock_inode_wait(state->inode, request);
		up_read(&nfsi->rwsem);
		mutex_unlock(&sp->so_delegreturn_mutex);
		goto out;
	}
	up_read(&nfsi->rwsem);
	mutex_unlock(&sp->so_delegreturn_mutex);
	status = _nfs4_do_setlk(state, cmd, request, NFS_LOCK_NEW);
out:
	request->fl_flags = fl_flags;
	return status;
}


// Source: nfs4proc.c
// Lines 6906-6934
