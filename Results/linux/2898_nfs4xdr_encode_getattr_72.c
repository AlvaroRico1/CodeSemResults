static void encode_getattr(struct xdr_stream *xdr,
		const __u32 *bitmap, const __u32 *mask, size_t len,
		struct compound_hdr *hdr)
{
	__u32 masked_bitmap[nfs4_fattr_bitmap_maxsz];

	encode_op_hdr(xdr, OP_GETATTR, decode_getattr_maxsz, hdr);
	if (mask) {
		if (WARN_ON_ONCE(len > ARRAY_SIZE(masked_bitmap)))
			len = ARRAY_SIZE(masked_bitmap);
		len = mask_bitmap4(bitmap, mask, masked_bitmap, len);
		bitmap = masked_bitmap;
	}
	xdr_encode_bitmap4(xdr, bitmap, len);
}


// Source: nfs4xdr.c
// Lines 1245-1259
