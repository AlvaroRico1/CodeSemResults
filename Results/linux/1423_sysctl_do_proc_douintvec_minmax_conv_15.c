static int do_proc_douintvec_minmax_conv(unsigned long *lvalp,
					 unsigned int *valp,
					 int write, void *data)
{
	int ret;
	unsigned int tmp;
	struct do_proc_douintvec_minmax_conv_param *param = data;
	/* write via temporary local uint for bounds-checking */
	unsigned int *up = write ? &tmp : valp;

	ret = do_proc_douintvec_conv(lvalp, up, write, data);
	if (ret)
		return ret;

	if (write) {
		if ((param->min && *param->min > tmp) ||
		    (param->max && *param->max < tmp))
			return -ERANGE;

		*valp = tmp;
	}

	return 0;
}


// Source: sysctl.c
// Lines 2729-2752
