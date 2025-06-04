bool create_ref_for_key(JOIN *join, JOIN_TAB *j, Key_use *org_keyuse,
                        table_map used_tables) {
  DBUG_TRACE;

  const uint key = org_keyuse->key;
  const bool ftkey = (org_keyuse->keypart == FT_KEYPART);
  THD *const thd = join->thd;
  uint keyparts, length;
  TABLE *const table = j->table();
  KEY *const keyinfo = table->key_info + key;
  Key_use *chosen_keyuses[MAX_REF_PARTS];

  assert(j->keys().is_set(org_keyuse->key));

  /* Calculate the length of the used key. */
  if (ftkey) {
    Item_func_match *ifm = down_cast<Item_func_match *>(org_keyuse->val);

    length = 0;
    keyparts = 1;
    ifm->get_master()->join_key = true;
  } else /* not ftkey */
    calc_length_and_keyparts(org_keyuse, j, key, used_tables, chosen_keyuses,
                             &length, &keyparts, nullptr, nullptr);
  /* set up fieldref */
  if (init_ref(thd, keyparts, length, (int)key, &j->ref())) {
    return true;
  }

  uchar *key_buff = j->ref().key_buff;
  uchar *null_ref_key = nullptr;
  bool keyuse_uses_no_tables = true;
  bool null_rejecting_key = true;
  if (ftkey) {
    Key_use *keyuse = org_keyuse;
    j->ref().items[0] = ((Item_func *)(keyuse->val))->key_item();
    /* Predicates pushed down into subquery can't be used FT access */
    j->ref().cond_guards[0] = nullptr;
    // not supported yet. SerG
    assert(!(keyuse->used_tables & ~PSEUDO_TABLE_BITS));

    j->set_type(JT_FT);
    j->set_ft_func(down_cast<Item_func_match *>(keyuse->val));
    memset(j->ref().key_copy, 0, sizeof(j->ref().key_copy[0]) * keyparts);

    return false;
  }
  // Set up TABLE_REF based on chosen Key_use-s.
  for (uint part_no = 0; part_no < keyparts; part_no++) {
    Key_use *keyuse = chosen_keyuses[part_no];
    bool nullable = keyinfo->key_part[part_no].null_bit;

    if (keyuse->val->type() == Item::FIELD_ITEM) {
      // Look up the most appropriate field to base the ref access on.
      keyuse->val = get_best_field(down_cast<Item_field *>(keyuse->val),
                                   join->cond_equal);
      keyuse->used_tables = keyuse->val->used_tables();
    }

    if (init_ref_part(thd, part_no, keyuse->val, keyuse->cond_guard,
                      keyuse->null_rejecting, join->const_table_map,
                      keyuse->used_tables, nullable,
                      &keyinfo->key_part[part_no], key_buff, &j->ref())) {
      return true;
    }

    keyuse_uses_no_tables = keyuse_uses_no_tables && !keyuse->used_tables;

    /*
      Remember if we are going to use REF_OR_NULL
      But only if field _really_ can be null i.e. we force JT_REF
      instead of JT_REF_OR_NULL in case if field can't be null
    */
    if ((keyuse->optimize & KEY_OPTIMIZE_REF_OR_NULL) && nullable) {
      assert(null_ref_key == nullptr);  // or we would overwrite it below
      null_ref_key = key_buff;
    }
    /*
      The selected key will reject matches on NULL values if:
       - the key field is nullable, and
       - predicate rejects NULL values (keyuse->null_rejecting is true), or
       - JT_REF_OR_NULL is not effective.
    */
    if ((keyinfo->key_part[part_no].field->is_nullable() ||
         table->is_nullable()) &&
        (!keyuse->null_rejecting || null_ref_key != nullptr)) {
      null_rejecting_key = false;
    }
    key_buff += keyinfo->key_part[part_no].store_length;
  }


// Source: sql_select.cc
// Lines 2212-2301
