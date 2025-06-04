static int __init reserve_setup(char *str)
{
	static int reserved;
	static struct resource reserve[MAXRESERVE];

	for (;;) {
		unsigned int io_start, io_num;
		int x = reserved;
		struct resource *parent;

		if (get_option(&str, &io_start) != 2)
			break;
		if (get_option(&str, &io_num) == 0)
			break;
		if (x < MAXRESERVE) {
			struct resource *res = reserve + x;

			/*
			 * If the region starts below 0x10000, we assume it's
			 * I/O port space; otherwise assume it's memory.
			 */
			if (io_start < 0x10000) {
				res->flags = IORESOURCE_IO;
				parent = &ioport_resource;
			} else {
				res->flags = IORESOURCE_MEM;
				parent = &iomem_resource;
			}
			res->name = "reserved";
			res->start = io_start;
			res->end = io_start + io_num - 1;
			res->flags |= IORESOURCE_BUSY;
			res->desc = IORES_DESC_NONE;
			res->child = NULL;
			if (request_resource(parent, res) == 0)
				reserved = x+1;
		}
	}


// Source: resource.c
// Lines 1486-1523
