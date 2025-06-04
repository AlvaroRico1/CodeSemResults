static int do_proc_dointvec_minmax_conv(bool *negp, unsigned long *lvalp,
					int *valp,
					int write, void *data)
{
	int tmp, ret;
	struct do_proc_dointvec_minmax_conv_param *param = data;
	/*
	 * If writing, first do so via a temporary local int so we can
	 * bounds-check it before touching *valp.
	 */
	int *ip = write ? &tmp : valp;

	ret = do_proc_dointvec_conv(negp, lvalp, ip, write, data);
	if (ret)
		return ret;

	if (write) {
		if ((param->min && *param->min > tmp) ||
		    (param->max && *param->max < tmp))
			return -EINVAL;
		*valp = tmp;
	}

	return 0;
}


// Source: sysctl.c
// Lines 2662-2686
