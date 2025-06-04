static ssize_t driver_override_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	char *driver_override, *old, *cp;

	/* We need to keep extra room for a newline */
	if (count >= (PAGE_SIZE - 1))
		return -EINVAL;

	driver_override = kstrndup(buf, count, GFP_KERNEL);
	if (!driver_override)
		return -ENOMEM;

	cp = strchr(driver_override, '\n');
	if (cp)
		*cp = '\0';

	device_lock(dev);
	old = pdev->driver_override;
	if (strlen(driver_override)) {
		pdev->driver_override = driver_override;
	} else {
		kfree(driver_override);
		pdev->driver_override = NULL;
	}
	device_unlock(dev);

	kfree(old);

	return count;
}


// Source: pci-sysfs.c
// Lines 702-734
