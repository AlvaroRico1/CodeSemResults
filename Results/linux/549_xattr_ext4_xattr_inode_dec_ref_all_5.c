ext4_xattr_inode_dec_ref_all(handle_t *handle, struct inode *parent,
			     struct buffer_head *bh,
			     struct ext4_xattr_entry *first, bool block_csum,
			     struct ext4_xattr_inode_array **ea_inode_array,
			     int extra_credits, bool skip_quota)
{
	struct inode *ea_inode;
	struct ext4_xattr_entry *entry;
	bool dirty = false;
	unsigned int ea_ino;
	int err;
	int credits;

	/* One credit for dec ref on ea_inode, one for orphan list addition, */
	credits = 2 + extra_credits;

	for (entry = first; !IS_LAST_ENTRY(entry);
	     entry = EXT4_XATTR_NEXT(entry)) {
		if (!entry->e_value_inum)
			continue;
		ea_ino = le32_to_cpu(entry->e_value_inum);
		err = ext4_xattr_inode_iget(parent, ea_ino,
					    le32_to_cpu(entry->e_hash),
					    &ea_inode);
		if (err)
			continue;

		err = ext4_expand_inode_array(ea_inode_array, ea_inode);
		if (err) {
			ext4_warning_inode(ea_inode,
					   "Expand inode array err=%d", err);
			iput(ea_inode);
			continue;
		}

		err = ext4_xattr_ensure_credits(handle, parent, credits, bh,
						dirty, block_csum);
		if (err) {
			ext4_warning_inode(ea_inode, "Ensure credits err=%d",
					   err);
			continue;
		}

		err = ext4_xattr_inode_dec_ref(handle, ea_inode);
		if (err) {
			ext4_warning_inode(ea_inode, "ea_inode dec ref err=%d",
					   err);
			continue;
		}

		if (!skip_quota)
			ext4_xattr_inode_free_quota(parent, ea_inode,
					      le32_to_cpu(entry->e_value_size));

		/*
		 * Forget about ea_inode within the same transaction that
		 * decrements the ref count. This avoids duplicate decrements in
		 * case the rest of the work spills over to subsequent
		 * transactions.
		 */
		entry->e_value_inum = 0;
		entry->e_value_size = 0;

		dirty = true;
	}

	if (dirty) {
		/*
		 * Note that we are deliberately skipping csum calculation for
		 * the final update because we do not expect any journal
		 * restarts until xattr block is freed.
		 */

		err = ext4_handle_dirty_metadata(handle, NULL, bh);
		if (err)
			ext4_warning_inode(parent,
					   "handle dirty metadata err=%d", err);
	}
}


// Source: xattr.c
// Lines 1153-1231
