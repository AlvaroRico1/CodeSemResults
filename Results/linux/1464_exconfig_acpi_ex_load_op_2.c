acpi_ex_load_op(union acpi_operand_object *obj_desc,
		union acpi_operand_object *target,
		struct acpi_walk_state *walk_state)
{
	union acpi_operand_object *ddb_handle;
	struct acpi_table_header *table_header;
	struct acpi_table_header *table;
	u32 table_index;
	acpi_status status;
	u32 length;

	ACPI_FUNCTION_TRACE(ex_load_op);

	/* Source Object can be either an op_region or a Buffer/Field */

	switch (obj_desc->common.type) {
	case ACPI_TYPE_REGION:

		ACPI_DEBUG_PRINT((ACPI_DB_EXEC,
				  "Load table from Region %p\n", obj_desc));

		/* Region must be system_memory (from ACPI spec) */

		if (obj_desc->region.space_id != ACPI_ADR_SPACE_SYSTEM_MEMORY) {
			return_ACPI_STATUS(AE_AML_OPERAND_TYPE);
		}

		/*
		 * If the Region Address and Length have not been previously
		 * evaluated, evaluate them now and save the results.
		 */
		if (!(obj_desc->common.flags & AOPOBJ_DATA_VALID)) {
			status = acpi_ds_get_region_arguments(obj_desc);
			if (ACPI_FAILURE(status)) {
				return_ACPI_STATUS(status);
			}
		}

		/* Get the table header first so we can get the table length */

		table_header = ACPI_ALLOCATE(sizeof(struct acpi_table_header));
		if (!table_header) {
			return_ACPI_STATUS(AE_NO_MEMORY);
		}

		status =
		    acpi_ex_region_read(obj_desc,
					sizeof(struct acpi_table_header),
					ACPI_CAST_PTR(u8, table_header));
		length = table_header->length;
		ACPI_FREE(table_header);

		if (ACPI_FAILURE(status)) {
			return_ACPI_STATUS(status);
		}

		/* Must have at least an ACPI table header */

		if (length < sizeof(struct acpi_table_header)) {
			return_ACPI_STATUS(AE_INVALID_TABLE_LENGTH);
		}

		/*
		 * The original implementation simply mapped the table, with no copy.
		 * However, the memory region is not guaranteed to remain stable and
		 * we must copy the table to a local buffer. For example, the memory
		 * region is corrupted after suspend on some machines. Dynamically
		 * loaded tables are usually small, so this overhead is minimal.
		 *
		 * The latest implementation (5/2009) does not use a mapping at all.
		 * We use the low-level operation region interface to read the table
		 * instead of the obvious optimization of using a direct mapping.
		 * This maintains a consistent use of operation regions across the
		 * entire subsystem. This is important if additional processing must
		 * be performed in the (possibly user-installed) operation region
		 * handler. For example, acpi_exec and ASLTS depend on this.
		 */

		/* Allocate a buffer for the table */

		table = ACPI_ALLOCATE(length);
		if (!table) {
			return_ACPI_STATUS(AE_NO_MEMORY);
		}

		/* Read the entire table */

		status = acpi_ex_region_read(obj_desc, length,
					     ACPI_CAST_PTR(u8, table));
		if (ACPI_FAILURE(status)) {
			ACPI_FREE(table);
			return_ACPI_STATUS(status);
		}
		break;

	case ACPI_TYPE_BUFFER:	/* Buffer or resolved region_field */

		ACPI_DEBUG_PRINT((ACPI_DB_EXEC,
				  "Load table from Buffer or Field %p\n",
				  obj_desc));

		/* Must have at least an ACPI table header */

		if (obj_desc->buffer.length < sizeof(struct acpi_table_header)) {
			return_ACPI_STATUS(AE_INVALID_TABLE_LENGTH);
		}

		/* Get the actual table length from the table header */

		table_header =
		    ACPI_CAST_PTR(struct acpi_table_header,
				  obj_desc->buffer.pointer);
		length = table_header->length;

		/* Table cannot extend beyond the buffer */

		if (length > obj_desc->buffer.length) {
			return_ACPI_STATUS(AE_AML_BUFFER_LIMIT);
		}
		if (length < sizeof(struct acpi_table_header)) {
			return_ACPI_STATUS(AE_INVALID_TABLE_LENGTH);
		}

		/*
		 * Copy the table from the buffer because the buffer could be
		 * modified or even deleted in the future
		 */
		table = ACPI_ALLOCATE(length);
		if (!table) {
			return_ACPI_STATUS(AE_NO_MEMORY);
		}

		memcpy(table, table_header, length);
		break;

	default:

		return_ACPI_STATUS(AE_AML_OPERAND_TYPE);
	}

	/* Install the new table into the local data structures */

	ACPI_INFO(("Dynamic OEM Table Load:"));
	acpi_ex_exit_interpreter();
	status = acpi_tb_install_and_load_table(ACPI_PTR_TO_PHYSADDR(table),
						ACPI_TABLE_ORIGIN_INTERNAL_VIRTUAL,
						TRUE, &table_index);
	acpi_ex_enter_interpreter();
	if (ACPI_FAILURE(status)) {

		/* Delete allocated table buffer */

		ACPI_FREE(table);
		return_ACPI_STATUS(status);
	}

	/*
	 * Add the table to the namespace.
	 *
	 * Note: Load the table objects relative to the root of the namespace.
	 * This appears to go against the ACPI specification, but we do it for
	 * compatibility with other ACPI implementations.
	 */
	status = acpi_ex_add_table(table_index, &ddb_handle);
	if (ACPI_FAILURE(status)) {

		/* On error, table_ptr was deallocated above */

		return_ACPI_STATUS(status);
	}

	/* Complete the initialization/resolution of new objects */

	acpi_ex_exit_interpreter();
	acpi_ns_initialize_objects();
	acpi_ex_enter_interpreter();

	/* Store the ddb_handle into the Target operand */

	status = acpi_ex_store(ddb_handle, target, walk_state);
	if (ACPI_FAILURE(status)) {
		(void)acpi_ex_unload_table(ddb_handle);

		/* table_ptr was deallocated above */

		acpi_ut_remove_reference(ddb_handle);
		return_ACPI_STATUS(status);
	}

	/* Remove the reference by added by acpi_ex_store above */

	acpi_ut_remove_reference(ddb_handle);
	return_ACPI_STATUS(status);
}


// Source: exconfig.c
// Lines 268-461
