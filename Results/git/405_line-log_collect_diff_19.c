static int collect_diff(mmfile_t *parent, mmfile_t *target, struct diff_ranges *out)
{
	struct collect_diff_cbdata cbdata = {NULL};
	xpparam_t xpp;
	xdemitconf_t xecfg;
	xdemitcb_t ecb;

	memset(&xpp, 0, sizeof(xpp));
	memset(&xecfg, 0, sizeof(xecfg));
	xecfg.ctxlen = xecfg.interhunkctxlen = 0;

	cbdata.diff = out;
	xecfg.hunk_func = collect_diff_cb;
	memset(&ecb, 0, sizeof(ecb));
	ecb.priv = &cbdata;
	return xdi_diff(parent, target, &xpp, &xecfg, &ecb);
}


// Source: line-log.c
// Lines 329-345
