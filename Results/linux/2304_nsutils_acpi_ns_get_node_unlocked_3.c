acpi_ns_get_node_unlocked(struct acpi_namespace_node *prefix_node,
			  const char *pathname,
			  u32 flags, struct acpi_namespace_node **return_node)
{
	union acpi_generic_state scope_info;
	acpi_status status;
	char *internal_path;

	ACPI_FUNCTION_TRACE_PTR(ns_get_node_unlocked,
				ACPI_CAST_PTR(char, pathname));

	/* Simplest case is a null pathname */

	if (!pathname) {
		*return_node = prefix_node;
		if (!prefix_node) {
			*return_node = acpi_gbl_root_node;
		}

		return_ACPI_STATUS(AE_OK);
	}

	/* Quick check for a reference to the root */

	if (ACPI_IS_ROOT_PREFIX(pathname[0]) && (!pathname[1])) {
		*return_node = acpi_gbl_root_node;
		return_ACPI_STATUS(AE_OK);
	}

	/* Convert path to internal representation */

	status = acpi_ns_internalize_name(pathname, &internal_path);
	if (ACPI_FAILURE(status)) {
		return_ACPI_STATUS(status);
	}

	/* Setup lookup scope (search starting point) */

	scope_info.scope.node = prefix_node;

	/* Lookup the name in the namespace */

	status = acpi_ns_lookup(&scope_info, internal_path, ACPI_TYPE_ANY,
				ACPI_IMODE_EXECUTE,
				(flags | ACPI_NS_DONT_OPEN_SCOPE), NULL,
				return_node);
	if (ACPI_FAILURE(status)) {
		ACPI_DEBUG_PRINT((ACPI_DB_EXEC, "%s, %s\n",
				  pathname, acpi_format_exception(status)));
	}

	ACPI_FREE(internal_path);
	return_ACPI_STATUS(status);
}


// Source: nsutils.c
// Lines 635-688
