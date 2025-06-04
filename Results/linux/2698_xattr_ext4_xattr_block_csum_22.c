static __le32 ext4_xattr_block_csum(struct inode *inode,
				    sector_t block_nr,
				    struct ext4_xattr_header *hdr)
{
	struct ext4_sb_info *sbi = EXT4_SB(inode->i_sb);
	__u32 csum;
	__le64 dsk_block_nr = cpu_to_le64(block_nr);
	__u32 dummy_csum = 0;
	int offset = offsetof(struct ext4_xattr_header, h_checksum);

	csum = ext4_chksum(sbi, sbi->s_csum_seed, (__u8 *)&dsk_block_nr,
			   sizeof(dsk_block_nr));
	csum = ext4_chksum(sbi, csum, (__u8 *)hdr, offset);
	csum = ext4_chksum(sbi, csum, (__u8 *)&dummy_csum, sizeof(dummy_csum));
	offset += sizeof(dummy_csum);
	csum = ext4_chksum(sbi, csum, (__u8 *)hdr + offset,
			   EXT4_BLOCK_SIZE(inode->i_sb) - offset);

	return cpu_to_le32(csum);
}


// Source: xattr.c
// Lines 128-147
