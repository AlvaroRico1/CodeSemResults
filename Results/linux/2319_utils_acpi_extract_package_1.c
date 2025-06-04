acpi_extract_package(union acpi_object *package,
		     struct acpi_buffer *format, struct acpi_buffer *buffer)
{
	u32 size_required = 0;
	u32 tail_offset = 0;
	char *format_string = NULL;
	u32 format_count = 0;
	u32 i = 0;
	u8 *head = NULL;
	u8 *tail = NULL;


	if (!package || (package->type != ACPI_TYPE_PACKAGE)
	    || (package->package.count < 1)) {
		printk(KERN_WARNING PREFIX "Invalid package argument\n");
		return AE_BAD_PARAMETER;
	}

	if (!format || !format->pointer || (format->length < 1)) {
		printk(KERN_WARNING PREFIX "Invalid format argument\n");
		return AE_BAD_PARAMETER;
	}

	if (!buffer) {
		printk(KERN_WARNING PREFIX "Invalid buffer argument\n");
		return AE_BAD_PARAMETER;
	}

	format_count = (format->length / sizeof(char)) - 1;
	if (format_count > package->package.count) {
		printk(KERN_WARNING PREFIX "Format specifies more objects [%d]"
			      " than exist in package [%d].\n",
			      format_count, package->package.count);
		return AE_BAD_DATA;
	}

	format_string = format->pointer;

	/*
	 * Calculate size_required.
	 */
	for (i = 0; i < format_count; i++) {

		union acpi_object *element = &(package->package.elements[i]);

		switch (element->type) {

		case ACPI_TYPE_INTEGER:
			switch (format_string[i]) {
			case 'N':
				size_required += sizeof(u64);
				tail_offset += sizeof(u64);
				break;
			case 'S':
				size_required +=
				    sizeof(char *) + sizeof(u64) +
				    sizeof(char);
				tail_offset += sizeof(char *);
				break;
			default:
				printk(KERN_WARNING PREFIX "Invalid package element"
					      " [%d]: got number, expecting"
					      " [%c]\n",
					      i, format_string[i]);
				return AE_BAD_DATA;
				break;
			}
			break;

		case ACPI_TYPE_STRING:
		case ACPI_TYPE_BUFFER:
			switch (format_string[i]) {
			case 'S':
				size_required +=
				    sizeof(char *) +
				    (element->string.length * sizeof(char)) +
				    sizeof(char);
				tail_offset += sizeof(char *);
				break;
			case 'B':
				size_required +=
				    sizeof(u8 *) + element->buffer.length;
				tail_offset += sizeof(u8 *);
				break;
			default:
				printk(KERN_WARNING PREFIX "Invalid package element"
					      " [%d] got string/buffer,"
					      " expecting [%c]\n",
					      i, format_string[i]);
				return AE_BAD_DATA;
				break;
			}
			break;
		case ACPI_TYPE_LOCAL_REFERENCE:
			switch (format_string[i]) {
			case 'R':
				size_required += sizeof(void *);
				tail_offset += sizeof(void *);
				break;
			default:
				printk(KERN_WARNING PREFIX "Invalid package element"
					      " [%d] got reference,"
					      " expecting [%c]\n",
					      i, format_string[i]);
				return AE_BAD_DATA;
				break;
			}
			break;

		case ACPI_TYPE_PACKAGE:
		default:
			ACPI_DEBUG_PRINT((ACPI_DB_INFO,
					  "Found unsupported element at index=%d\n",
					  i));
			/* TBD: handle nested packages... */
			return AE_SUPPORT;
			break;
		}
	}

	/*
	 * Validate output buffer.
	 */
	if (buffer->length == ACPI_ALLOCATE_BUFFER) {
		buffer->pointer = ACPI_ALLOCATE_ZEROED(size_required);
		if (!buffer->pointer)
			return AE_NO_MEMORY;
		buffer->length = size_required;
	} else {
		if (buffer->length < size_required) {
			buffer->length = size_required;
			return AE_BUFFER_OVERFLOW;
		} else if (buffer->length != size_required ||
			   !buffer->pointer) {
			return AE_BAD_PARAMETER;
		}
	}

	head = buffer->pointer;
	tail = buffer->pointer + tail_offset;

	/*
	 * Extract package data.
	 */
	for (i = 0; i < format_count; i++) {

		u8 **pointer = NULL;
		union acpi_object *element = &(package->package.elements[i]);

		switch (element->type) {

		case ACPI_TYPE_INTEGER:
			switch (format_string[i]) {
			case 'N':
				*((u64 *) head) =
				    element->integer.value;
				head += sizeof(u64);
				break;
			case 'S':
				pointer = (u8 **) head;
				*pointer = tail;
				*((u64 *) tail) =
				    element->integer.value;
				head += sizeof(u64 *);
				tail += sizeof(u64);
				/* NULL terminate string */
				*tail = (char)0;
				tail += sizeof(char);
				break;
			default:
				/* Should never get here */
				break;
			}
			break;

		case ACPI_TYPE_STRING:
		case ACPI_TYPE_BUFFER:
			switch (format_string[i]) {
			case 'S':
				pointer = (u8 **) head;
				*pointer = tail;
				memcpy(tail, element->string.pointer,
				       element->string.length);
				head += sizeof(char *);
				tail += element->string.length * sizeof(char);
				/* NULL terminate string */
				*tail = (char)0;
				tail += sizeof(char);
				break;
			case 'B':
				pointer = (u8 **) head;
				*pointer = tail;
				memcpy(tail, element->buffer.pointer,
				       element->buffer.length);
				head += sizeof(u8 *);
				tail += element->buffer.length;
				break;
			default:
				/* Should never get here */
				break;
			}
			break;
		case ACPI_TYPE_LOCAL_REFERENCE:
			switch (format_string[i]) {
			case 'R':
				*(void **)head =
				    (void *)element->reference.handle;
				head += sizeof(void *);
				break;
			default:
				/* Should never get here */
				break;
			}
			break;
		case ACPI_TYPE_PACKAGE:
			/* TBD: handle nested packages... */
		default:
			/* Should never get here */
			break;
		}
	}


// Source: utils.c
// Lines 42-262
