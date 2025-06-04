static void nfs4_opendata_free(struct kref *kref)
{
	struct nfs4_opendata *p = container_of(kref,
			struct nfs4_opendata, kref);
	struct super_block *sb = p->dentry->d_sb;

	nfs4_lgopen_release(p->lgp);
	nfs_free_seqid(p->o_arg.seqid);
	nfs4_sequence_free_slot(&p->o_res.seq_res);
	if (p->state != NULL)
		nfs4_put_open_state(p->state);
	nfs4_put_state_owner(p->owner);

	nfs4_label_free(p->a_label);
	nfs4_label_free(p->f_label);

	dput(p->dir);
	dput(p->dentry);
	nfs_sb_deactive(sb);
	nfs_fattr_free_names(&p->f_attr);
	kfree(p->f_attr.mdsthreshold);
	kfree(p);
}


// Source: nfs4proc.c
// Lines 1355-1377
