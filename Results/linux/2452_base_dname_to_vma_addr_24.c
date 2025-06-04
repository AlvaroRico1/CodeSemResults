static int dname_to_vma_addr(struct dentry *dentry,
			     unsigned long *start, unsigned long *end)
{
	const char *str = dentry->d_name.name;
	unsigned long long sval, eval;
	unsigned int len;

	if (str[0] == '0' && str[1] != '-')
		return -EINVAL;
	len = _parse_integer(str, 16, &sval);
	if (len & KSTRTOX_OVERFLOW)
		return -EINVAL;
	if (sval != (unsigned long)sval)
		return -EINVAL;
	str += len;

	if (*str != '-')
		return -EINVAL;
	str++;

	if (str[0] == '0' && str[1])
		return -EINVAL;
	len = _parse_integer(str, 16, &eval);
	if (len & KSTRTOX_OVERFLOW)
		return -EINVAL;
	if (eval != (unsigned long)eval)
		return -EINVAL;
	str += len;

	if (*str != '\0')
		return -EINVAL;

	*start = sval;
	*end = eval;

	return 0;
}


// Source: base.c
// Lines 1922-1958
