void JOIN::finalize_derived_keys() {
  assert(query_block->materialized_derived_table_count);
  ASSERT_BEST_REF_IN_JOIN_ORDER(this);

  bool adjust_key_count = false;
  table_map processed_tables = 0;

  for (uint i = 0; i < tables; i++) {
    JOIN_TAB *tab = best_ref[i];
    TABLE *table = tab->table();
    TABLE_LIST *table_ref = tab->table_ref;
    /*
     Save chosen key description if:
     1) it's a materialized derived table
     2) it's not yet instantiated
     3) some keys are defined for it
    */
    if (table && table_ref->uses_materialization() &&  // (1)
        !table->is_created() &&                        // (2)
        table->s->keys > 0)                            // (3)
    {
      /*
        If there are two local references to the same CTE, and they use
        the same key, the iteration for the second reference is unnecessary.
      */
      if (processed_tables & table_ref->map()) continue;

      adjust_key_count = true;

      Key_map used_keys;

      // Mark all unique indexes as in use, since they have an effect
      // (deduplication) whether any expression refers to them or not.
      // In particular, they are used if we want to materialize a UNION DISTINCT
      // directly into the derived table.
      for (uint key_idx = 0; key_idx < table->s->keys; ++key_idx) {
        if (table->key_info[key_idx].flags & HA_NOSAME) {
          used_keys.set_bit(key_idx);
        }
      }

      // Same for the hash key used for manual deduplication, if any. (It always
      // has index 0 if it exists.)
      if (table->hash_field) {
        used_keys.set_bit(0);
      }

      Key_use *const keyuse = tab->position()->key;
      if (keyuse == nullptr && used_keys.is_clear_all()) {
        // Nothing uses any keys.
        tab->keys().clear_all();
        tab->const_keys.clear_all();
        continue;
      }

      Derived_refs_iterator it(table_ref);
      while (TABLE *t = it.get_next()) {
        /*
          Eliminate possible keys created by this JOIN and which it
          doesn't use.
          Collect all keys of this table which are used by any reference in
          this query block. Any other query block doesn't matter as:
          - either it was optimized before, so it's not using a key we may
          want to drop.
          - or it was optimized in this same window, so:
            * either we own the window, then any key we may want to
            drop is not visible to it.
            * or it owns the window, then we are using only existing
            keys.
          - or it will be optimized after, so it's not using any key yet.

          used_keys is a mix of possible used keys and existing used keys.
        */
        if (t->pos_in_table_list->query_block == query_block) {
          JOIN_TAB *jtab = t->reginfo.join_tab;
          Key_use *keyuse_1 = jtab->position()->key;
          if (keyuse_1) used_keys.set_bit(keyuse_1->key);
        }
      }

      uint new_idx = table->s->find_first_unused_tmp_key(
          used_keys);  // Also updates table->s->first_unused_tmp_key.
      if (keyuse == nullptr) {
        continue;
      }

      const uint old_idx = keyuse->key;
      assert(old_idx != new_idx);

      if (old_idx > new_idx) {
        assert(table->s->owner_of_possible_tmp_keys == query_block);
        it.rewind();
        while (TABLE *t = it.get_next()) {
          /*
            Unlike the collection of used_keys, references from other query
            blocks must be considered here, as they need a key_info array
            consistent with the to-be-changed table->s->keys.
          */
          t->copy_tmp_key(old_idx, it.is_first());
        }
      } else
        new_idx = old_idx;  // Index stays at same slot

      /*
        If the key was created by earlier-optimized query blocks, and is
        already used by nonlocal references, those don't need any further
        update: they are already setup to use it and we're not moving the
        key.
        If the key was created by this query block, nonlocal references cannot
        possibly be referencing it.
        In both cases, only local references need to update their Key_use.
      */
      it.rewind();
      while (TABLE *t = it.get_next()) {
        if (t->pos_in_table_list->query_block != query_block) continue;
        JOIN_TAB *jtab = t->reginfo.join_tab;
        Key_use *keyuse_1 = jtab->position()->key;
        if (keyuse_1 && keyuse_1->key == old_idx) {
          processed_tables |= t->pos_in_table_list->map();
          const bool key_is_const = jtab->const_keys.is_set(old_idx);
          // tab->keys() was never set, so must be set
          jtab->keys().clear_all();
          jtab->keys().set_bit(new_idx);
          jtab->const_keys.clear_all();
          if (key_is_const) tab->const_keys.set_bit(new_idx);
          for (Key_use *kit = keyuse_1;
               kit->table_ref == jtab->table_ref && kit->key == old_idx; kit++)
            kit->key = new_idx;
        }
      }
    }
  }


// Source: sql_optimizer.cc
// Lines 8845-8976
