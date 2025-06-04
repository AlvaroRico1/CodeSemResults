struct inode *__ext4_iget(struct super_block *sb, unsigned long ino,
			  ext4_iget_flags flags, const char *function,
			  unsigned int line)
{
	struct ext4_iloc iloc;
	struct ext4_inode *raw_inode;
	struct ext4_inode_info *ei;
	struct inode *inode;
	journal_t *journal = EXT4_SB(sb)->s_journal;
	long ret;
	loff_t size;
	int block;
	uid_t i_uid;
	gid_t i_gid;
	projid_t i_projid;

	if ((!(flags & EXT4_IGET_SPECIAL) &&
	     (ino < EXT4_FIRST_INO(sb) && ino != EXT4_ROOT_INO)) ||
	    (ino < EXT4_ROOT_INO) ||
	    (ino > le32_to_cpu(EXT4_SB(sb)->s_es->s_inodes_count))) {
		if (flags & EXT4_IGET_HANDLE)
			return ERR_PTR(-ESTALE);
		__ext4_error(sb, function, line,
			     "inode #%lu: comm %s: iget: illegal inode #",
			     ino, current->comm);
		return ERR_PTR(-EFSCORRUPTED);
	}

	inode = iget_locked(sb, ino);
	if (!inode)
		return ERR_PTR(-ENOMEM);
	if (!(inode->i_state & I_NEW))
		return inode;

	ei = EXT4_I(inode);
	iloc.bh = NULL;

	ret = __ext4_get_inode_loc(inode, &iloc, 0);
	if (ret < 0)
		goto bad_inode;
	raw_inode = ext4_raw_inode(&iloc);

	if ((ino == EXT4_ROOT_INO) && (raw_inode->i_links_count == 0)) {
		ext4_error_inode(inode, function, line, 0,
				 "iget: root inode unallocated");
		ret = -EFSCORRUPTED;
		goto bad_inode;
	}

	if ((flags & EXT4_IGET_HANDLE) &&
	    (raw_inode->i_links_count == 0) && (raw_inode->i_mode == 0)) {
		ret = -ESTALE;
		goto bad_inode;
	}

	if (EXT4_INODE_SIZE(inode->i_sb) > EXT4_GOOD_OLD_INODE_SIZE) {
		ei->i_extra_isize = le16_to_cpu(raw_inode->i_extra_isize);
		if (EXT4_GOOD_OLD_INODE_SIZE + ei->i_extra_isize >
			EXT4_INODE_SIZE(inode->i_sb) ||
		    (ei->i_extra_isize & 3)) {
			ext4_error_inode(inode, function, line, 0,
					 "iget: bad extra_isize %u "
					 "(inode size %u)",
					 ei->i_extra_isize,
					 EXT4_INODE_SIZE(inode->i_sb));
			ret = -EFSCORRUPTED;
			goto bad_inode;
		}
	} else
		ei->i_extra_isize = 0;

	/* Precompute checksum seed for inode metadata */
	if (ext4_has_metadata_csum(sb)) {
		struct ext4_sb_info *sbi = EXT4_SB(inode->i_sb);
		__u32 csum;
		__le32 inum = cpu_to_le32(inode->i_ino);
		__le32 gen = raw_inode->i_generation;
		csum = ext4_chksum(sbi, sbi->s_csum_seed, (__u8 *)&inum,
				   sizeof(inum));
		ei->i_csum_seed = ext4_chksum(sbi, csum, (__u8 *)&gen,
					      sizeof(gen));
	}


// Source: inode.c
// Lines 4844-4925
