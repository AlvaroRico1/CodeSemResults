static int do_insert_tree(struct qtree_mem_dqinfo *info, struct dquot *dquot,
			  uint *treeblk, int depth)
{
	char *buf = getdqbuf(info->dqi_usable_bs);
	int ret = 0, newson = 0, newact = 0;
	__le32 *ref;
	uint newblk;

	if (!buf)
		return -ENOMEM;
	if (!*treeblk) {
		ret = get_free_dqblk(info);
		if (ret < 0)
			goto out_buf;
		*treeblk = ret;
		memset(buf, 0, info->dqi_usable_bs);
		newact = 1;
	} else {
		ret = read_blk(info, *treeblk, buf);
		if (ret < 0) {
			quota_error(dquot->dq_sb, "Can't read tree quota "
				    "block %u", *treeblk);
			goto out_buf;
		}
	}
	ref = (__le32 *)buf;
	newblk = le32_to_cpu(ref[get_index(info, dquot->dq_id, depth)]);
	if (!newblk)
		newson = 1;
	if (depth == info->dqi_qtree_depth - 1) {
#ifdef __QUOTA_QT_PARANOIA
		if (newblk) {
			quota_error(dquot->dq_sb, "Inserting already present "
				    "quota entry (block %u)",
				    le32_to_cpu(ref[get_index(info,
						dquot->dq_id, depth)]));
			ret = -EIO;
			goto out_buf;
		}
#endif
		newblk = find_free_dqentry(info, dquot, &ret);
	} else {
		ret = do_insert_tree(info, dquot, &newblk, depth+1);
	}
	if (newson && ret >= 0) {
		ref[get_index(info, dquot->dq_id, depth)] =
							cpu_to_le32(newblk);
		ret = write_blk(info, *treeblk, buf);
	} else if (newact && ret < 0) {
		put_free_dqblk(info, buf, *treeblk);
	}
out_buf:
	kfree(buf);
	return ret;
}


// Source: quota_tree.c
// Lines 298-352
