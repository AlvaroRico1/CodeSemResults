static Key_field *merge_key_fields(Key_field *start, Key_field *new_fields,
                                   Key_field *end, uint and_level) {
  if (start == new_fields) return start;  // Impossible or
  if (new_fields == end) return start;    // No new fields, skip all

  Key_field *first_free = new_fields;

  /* Mark all found fields in old array */
  for (; new_fields != end; new_fields++) {
    const Field *const new_field = new_fields->item_field->field;

    for (Key_field *old = start; old != first_free; old++) {
      const Field *const old_field = old->item_field->field;

      /*
        Check that the Field objects are the same, as we may have several
        Item_field objects pointing to the same Field:
      */
      if (old_field == new_field) {
        /*
          NOTE: below const_item() call really works as "!used_tables()", i.e.
          it can return false where it is feasible to make it return true.

          The cause is as follows: Some of the tables are already known to be
          const tables (the detection code is in JOIN::make_join_plan(),
          above the update_ref_and_keys() call), but we didn't propagate
          information about this: TABLE::const_table is not set to true, and
          Item::update_used_tables() hasn't been called for each item.
          The result of this is that we're missing some 'ref' accesses.
          TODO: OptimizerTeam: Fix this
        */
        if (!new_fields->val->const_item()) {
          /*
            If the value matches, we can use the key reference.
            If not, we keep it until we have examined all new values
          */
          if (old->val->eq(new_fields->val, old_field->binary())) {
            old->level = and_level;
            old->optimize =
                ((old->optimize & new_fields->optimize & KEY_OPTIMIZE_EXISTS) |
                 ((old->optimize | new_fields->optimize) &
                  KEY_OPTIMIZE_REF_OR_NULL));
            old->null_rejecting =
                (old->null_rejecting && new_fields->null_rejecting);
          }
        } else if (old->eq_func && new_fields->eq_func &&
                   old->val->eq_by_collation(new_fields->val,
                                             old_field->binary(),
                                             old_field->charset())) {
          old->level = and_level;
          old->optimize =
              ((old->optimize & new_fields->optimize & KEY_OPTIMIZE_EXISTS) |
               ((old->optimize | new_fields->optimize) &
                KEY_OPTIMIZE_REF_OR_NULL));
          old->null_rejecting =
              (old->null_rejecting && new_fields->null_rejecting);
        } else if (old->eq_func && new_fields->eq_func &&
                   ((old->val->const_item() && old->val->is_null()) ||
                    new_fields->val->is_null())) {
          /* field = expression OR field IS NULL */
          old->level = and_level;
          old->optimize = KEY_OPTIMIZE_REF_OR_NULL;
          /*
            Remember the NOT NULL value unless the value does not depend
            on other tables.
          */
          if (!old->val->used_tables() && old->val->is_null())
            old->val = new_fields->val;
          /* The referred expression can be NULL: */
          old->null_rejecting = false;
        } else {
          /*
            We are comparing two different const.  In this case we can't
            use a key-lookup on this so it's better to remove the value
            and let the range optimizer handle it
          */
          if (old == --first_free)  // If last item
            break;
          *old = *first_free;  // Remove old value
          old--;               // Retry this value
        }
      }
    }
  }


// Source: sql_optimizer.cc
// Lines 6527-6610
