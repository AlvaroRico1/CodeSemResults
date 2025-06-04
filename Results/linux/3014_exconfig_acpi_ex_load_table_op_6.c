acpi_ex_load_table_op(struct acpi_walk_state *walk_state,
		      union acpi_operand_object **return_desc)
{
	acpi_status status;
	union acpi_operand_object **operand = &walk_state->operands[0];
	struct acpi_namespace_node *parent_node;
	struct acpi_namespace_node *start_node;
	struct acpi_namespace_node *parameter_node = NULL;
	union acpi_operand_object *ddb_handle;
	u32 table_index;

	ACPI_FUNCTION_TRACE(ex_load_table_op);

	/* Find the ACPI table in the RSDT/XSDT */

	acpi_ex_exit_interpreter();
	status = acpi_tb_find_table(operand[0]->string.pointer,
				    operand[1]->string.pointer,
				    operand[2]->string.pointer, &table_index);
	acpi_ex_enter_interpreter();
	if (ACPI_FAILURE(status)) {
		if (status != AE_NOT_FOUND) {
			return_ACPI_STATUS(status);
		}

		/* Table not found, return an Integer=0 and AE_OK */

		ddb_handle = acpi_ut_create_integer_object((u64) 0);
		if (!ddb_handle) {
			return_ACPI_STATUS(AE_NO_MEMORY);
		}

		*return_desc = ddb_handle;
		return_ACPI_STATUS(AE_OK);
	}

	/* Default nodes */

	start_node = walk_state->scope_info->scope.node;
	parent_node = acpi_gbl_root_node;

	/* root_path (optional parameter) */

	if (operand[3]->string.length > 0) {
		/*
		 * Find the node referenced by the root_path_string. This is the
		 * location within the namespace where the table will be loaded.
		 */
		status = acpi_ns_get_node_unlocked(start_node,
						   operand[3]->string.pointer,
						   ACPI_NS_SEARCH_PARENT,
						   &parent_node);
		if (ACPI_FAILURE(status)) {
			return_ACPI_STATUS(status);
		}
	}

	/* parameter_path (optional parameter) */

	if (operand[4]->string.length > 0) {
		if ((operand[4]->string.pointer[0] != AML_ROOT_PREFIX) &&
		    (operand[4]->string.pointer[0] != AML_PARENT_PREFIX)) {
			/*
			 * Path is not absolute, so it will be relative to the node
			 * referenced by the root_path_string (or the NS root if omitted)
			 */
			start_node = parent_node;
		}

		/* Find the node referenced by the parameter_path_string */

		status = acpi_ns_get_node_unlocked(start_node,
						   operand[4]->string.pointer,
						   ACPI_NS_SEARCH_PARENT,
						   &parameter_node);
		if (ACPI_FAILURE(status)) {
			return_ACPI_STATUS(status);
		}
	}

	/* Load the table into the namespace */

	ACPI_INFO(("Dynamic OEM Table Load:"));
	acpi_ex_exit_interpreter();
	status = acpi_tb_load_table(table_index, parent_node);
	acpi_ex_enter_interpreter();
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	status = acpi_ex_add_table(table_index, &ddb_handle);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	/* Complete the initialization/resolution of new objects */

	acpi_ex_exit_interpreter();
	acpi_ns_initialize_objects();
	acpi_ex_enter_interpreter();

	/* Parameter Data (optional) */

	if (parameter_node) {

		/* Store the parameter data into the optional parameter object */

		status = acpi_ex_store(operand[5],
				       ACPI_CAST_PTR(union acpi_operand_object,
						     parameter_node),
				       walk_state);
		if (ACPI_FAILURE(status)) {
			(void)acpi_ex_unload_table(ddb_handle);

			acpi_ut_remove_reference(ddb_handle);
			return_ACPI_STATUS(status);
		}
	}

	*return_desc = ddb_handle;
	return_ACPI_STATUS(status);
}


// Source: exconfig.c
// Lines 82-203
