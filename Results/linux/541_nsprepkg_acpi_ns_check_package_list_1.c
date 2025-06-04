acpi_ns_check_package_list(struct acpi_evaluate_info *info,
			   const union acpi_predefined_info *package,
			   union acpi_operand_object **elements, u32 count)
{
	union acpi_operand_object *sub_package;
	union acpi_operand_object **sub_elements;
	acpi_status status;
	u32 expected_count;
	u32 i;
	u32 j;

	/*
	 * Validate each subpackage in the parent Package
	 *
	 * NOTE: assumes list of subpackages contains no NULL elements.
	 * Any NULL elements should have been removed by earlier call
	 * to acpi_ns_remove_null_elements.
	 */
	for (i = 0; i < count; i++) {
		sub_package = *elements;
		sub_elements = sub_package->package.elements;
		info->parent_package = sub_package;

		/* Each sub-object must be of type Package */

		status = acpi_ns_check_object_type(info, &sub_package,
						   ACPI_RTYPE_PACKAGE, i);
		if (ACPI_FAILURE(status)) {
			return (status);
		}

		/* Examine the different types of expected subpackages */

		info->parent_package = sub_package;
		switch (package->ret_info.type) {
		case ACPI_PTYPE2:
		case ACPI_PTYPE2_PKG_COUNT:
		case ACPI_PTYPE2_REV_FIXED:

			/* Each subpackage has a fixed number of elements */

			expected_count =
			    package->ret_info.count1 + package->ret_info.count2;
			if (sub_package->package.count < expected_count) {
				goto package_too_small;
			}

			status =
			    acpi_ns_check_package_elements(info, sub_elements,
							   package->ret_info.
							   object_type1,
							   package->ret_info.
							   count1,
							   package->ret_info.
							   object_type2,
							   package->ret_info.
							   count2, 0);
			if (ACPI_FAILURE(status)) {
				return (status);
			}
			break;

		case ACPI_PTYPE2_FIX_VAR:
			/*
			 * Each subpackage has a fixed number of elements and an
			 * optional element
			 */
			expected_count =
			    package->ret_info.count1 + package->ret_info.count2;
			if (sub_package->package.count < expected_count) {
				goto package_too_small;
			}

			status =
			    acpi_ns_check_package_elements(info, sub_elements,
							   package->ret_info.
							   object_type1,
							   package->ret_info.
							   count1,
							   package->ret_info.
							   object_type2,
							   sub_package->package.
							   count -
							   package->ret_info.
							   count1, 0);
			if (ACPI_FAILURE(status)) {
				return (status);
			}
			break;

		case ACPI_PTYPE2_VAR_VAR:
			/*
			 * Each subpackage has a fixed or variable number of elements
			 */
			break;

		case ACPI_PTYPE2_FIXED:

			/* Each subpackage has a fixed length */

			expected_count = package->ret_info2.count;
			if (sub_package->package.count < expected_count) {
				goto package_too_small;
			}

			/* Check the type of each subpackage element */

			for (j = 0; j < expected_count; j++) {
				status =
				    acpi_ns_check_object_type(info,
							      &sub_elements[j],
							      package->
							      ret_info2.
							      object_type[j],
							      j);
				if (ACPI_FAILURE(status)) {
					return (status);
				}
			}
			break;

		case ACPI_PTYPE2_MIN:

			/* Each subpackage has a variable but minimum length */

			expected_count = package->ret_info.count1;
			if (sub_package->package.count < expected_count) {
				goto package_too_small;
			}

			/* Check the type of each subpackage element */

			status =
			    acpi_ns_check_package_elements(info, sub_elements,
							   package->ret_info.
							   object_type1,
							   sub_package->package.
							   count, 0, 0, 0);
			if (ACPI_FAILURE(status)) {
				return (status);
			}
			break;

		case ACPI_PTYPE2_COUNT:
			/*
			 * First element is the (Integer) count of elements, including
			 * the count field (the ACPI name is num_elements)
			 */
			status = acpi_ns_check_object_type(info, sub_elements,
							   ACPI_RTYPE_INTEGER,
							   0);
			if (ACPI_FAILURE(status)) {
				return (status);
			}

			/*
			 * Make sure package is large enough for the Count and is
			 * is as large as the minimum size
			 */
			expected_count = (u32)(*sub_elements)->integer.value;
			if (sub_package->package.count < expected_count) {
				goto package_too_small;
			}

			if (sub_package->package.count <
			    package->ret_info.count1) {
				expected_count = package->ret_info.count1;
				goto package_too_small;
			}

			if (expected_count == 0) {
				/*
				 * Either the num_entries element was originally zero or it was
				 * a NULL element and repaired to an Integer of value zero.
				 * In either case, repair it by setting num_entries to be the
				 * actual size of the subpackage.
				 */
				expected_count = sub_package->package.count;
				(*sub_elements)->integer.value = expected_count;
			}

			/* Check the type of each subpackage element */

			status =
			    acpi_ns_check_package_elements(info,
							   (sub_elements + 1),
							   package->ret_info.
							   object_type1,
							   (expected_count - 1),
							   0, 0, 1);
			if (ACPI_FAILURE(status)) {
				return (status);
			}
			break;

		default:	/* Should not get here, type was validated by caller */

			ACPI_ERROR((AE_INFO, "Invalid Package type: %X",
				    package->ret_info.type));
			return (AE_AML_INTERNAL);
		}

		elements++;
	}

	return (AE_OK);

package_too_small:

	/* The subpackage count was smaller than required */

	ACPI_WARN_PREDEFINED((AE_INFO, info->full_pathname, info->node_flags,
			      "Return SubPackage[%u] is too small - found %u elements, expected %u",
			      i, sub_package->package.count, expected_count));

	return (AE_AML_OPERAND_VALUE);
}


// Source: nsprepkg.c
// Lines 386-602
