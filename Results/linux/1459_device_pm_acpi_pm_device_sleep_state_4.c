int acpi_pm_device_sleep_state(struct device *dev, int *d_min_p, int d_max_in)
{
	struct acpi_device *adev;
	int ret, d_min, d_max;

	if (d_max_in < ACPI_STATE_D0 || d_max_in > ACPI_STATE_D3_COLD)
		return -EINVAL;

	if (d_max_in > ACPI_STATE_D2) {
		enum pm_qos_flags_status stat;

		stat = dev_pm_qos_flags(dev, PM_QOS_FLAG_NO_POWER_OFF);
		if (stat == PM_QOS_FLAGS_ALL)
			d_max_in = ACPI_STATE_D2;
	}

	adev = ACPI_COMPANION(dev);
	if (!adev) {
		dev_dbg(dev, "ACPI companion missing in %s!\n", __func__);
		return -ENODEV;
	}

	ret = acpi_dev_pm_get_state(dev, adev, acpi_target_system_state(),
				    &d_min, &d_max);
	if (ret)
		return ret;

	if (d_max_in < d_min)
		return -EINVAL;

	if (d_max > d_max_in) {
		for (d_max = d_max_in; d_max > d_min; d_max--) {
			if (adev->power.states[d_max].flags.valid)
				break;
		}
	}

	if (d_min_p)
		*d_min_p = d_min;

	return d_max;
}


// Source: device_pm.c
// Lines 686-727
