static int find_next_id(struct qtree_mem_dqinfo *info, qid_t *id,
			unsigned int blk, int depth)
{
	char *buf = getdqbuf(info->dqi_usable_bs);
	__le32 *ref = (__le32 *)buf;
	ssize_t ret;
	unsigned int epb = info->dqi_usable_bs >> 2;
	unsigned int level_inc = 1;
	int i;

	if (!buf)
		return -ENOMEM;

	for (i = depth; i < info->dqi_qtree_depth - 1; i++)
		level_inc *= epb;

	ret = read_blk(info, blk, buf);
	if (ret < 0) {
		quota_error(info->dqi_sb,
			    "Can't read quota tree block %u", blk);
		goto out_buf;
	}
	for (i = __get_index(info, *id, depth); i < epb; i++) {
		if (ref[i] == cpu_to_le32(0)) {
			*id += level_inc;
			continue;
		}
		if (depth == info->dqi_qtree_depth - 1) {
			ret = 0;
			goto out_buf;
		}
		ret = find_next_id(info, id, le32_to_cpu(ref[i]), depth + 1);
		if (ret != -ENOENT)
			break;
	}
	if (i == epb) {
		ret = -ENOENT;
		goto out_buf;
	}
out_buf:
	kfree(buf);
	return ret;
}


// Source: quota_tree.c
// Lines 679-721
