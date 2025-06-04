static ssize_t energy_perf_bias_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	unsigned int cpu = dev->id;
	u64 epb, val;
	int ret;

	ret = __sysfs_match_string(energy_perf_strings,
				   ARRAY_SIZE(energy_perf_strings), buf);
	if (ret >= 0)
		val = energ_perf_values[ret];
	else if (kstrtou64(buf, 0, &val) || val > MAX_EPB)
		return -EINVAL;

	ret = rdmsrl_on_cpu(cpu, MSR_IA32_ENERGY_PERF_BIAS, &epb);
	if (ret < 0)
		return ret;

	ret = wrmsrl_on_cpu(cpu, MSR_IA32_ENERGY_PERF_BIAS,
			    (epb & ~EPB_MASK) | val);
	if (ret < 0)
		return ret;

	return count;
}


// Source: intel_epb.c
// Lines 135-160
