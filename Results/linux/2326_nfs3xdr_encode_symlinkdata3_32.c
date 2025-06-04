static void encode_symlinkdata3(struct xdr_stream *xdr,
				const void *data,
				struct user_namespace *userns)
{
	const struct nfs3_symlinkargs *args = data;

	encode_sattr3(xdr, args->sattr, userns);
	encode_nfspath3(xdr, args->pages, args->pathlen);
}


// Source: nfs3xdr.c
// Lines 1077-1085
