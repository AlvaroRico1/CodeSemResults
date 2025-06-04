static ssize_t proc_bus_pci_read(struct file *file, char __user *buf,
				 size_t nbytes, loff_t *ppos)
{
	struct pci_dev *dev = PDE_DATA(file_inode(file));
	unsigned int pos = *ppos;
	unsigned int cnt, size;

	/*
	 * Normal users can read only the standardized portion of the
	 * configuration space as several chips lock up when trying to read
	 * undefined locations (think of Intel PIIX4 as a typical example).
	 */

	if (capable(CAP_SYS_ADMIN))
		size = dev->cfg_size;
	else if (dev->hdr_type == PCI_HEADER_TYPE_CARDBUS)
		size = 128;
	else
		size = 64;

	if (pos >= size)
		return 0;
	if (nbytes >= size)
		nbytes = size;
	if (pos + nbytes > size)
		nbytes = size - pos;
	cnt = nbytes;

	if (!access_ok(buf, cnt))
		return -EINVAL;

	pci_config_pm_runtime_get(dev);

	if ((pos & 1) && cnt) {
		unsigned char val;
		pci_user_read_config_byte(dev, pos, &val);
		__put_user(val, buf);
		buf++;
		pos++;
		cnt--;
	}

	if ((pos & 3) && cnt > 2) {
		unsigned short val;
		pci_user_read_config_word(dev, pos, &val);
		__put_user(cpu_to_le16(val), (__le16 __user *) buf);
		buf += 2;
		pos += 2;
		cnt -= 2;
	}


// Source: proc.c
// Lines 27-76
