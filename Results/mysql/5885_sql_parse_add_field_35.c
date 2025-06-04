bool Alter_info::add_field(
    THD *thd, const LEX_STRING *field_name, enum_field_types type,
    const char *length, const char *decimals, uint type_modifier,
    Item *default_value, Item *on_update_value, LEX_CSTRING *comment,
    const char *change, List<String> *interval_list, const CHARSET_INFO *cs,
    bool has_explicit_collation, uint uint_geom_type,
    Value_generator *gcol_info, Value_generator *default_val_expr,
    const char *opt_after, Nullable<gis::srid_t> srid,
    Sql_check_constraint_spec_list *col_check_const_spec_list,
    dd::Column::enum_hidden_type hidden, bool is_array) {
  uint8 datetime_precision = decimals ? atoi(decimals) : 0;
  DBUG_TRACE;
  assert(!is_array || hidden == dd::Column::enum_hidden_type::HT_HIDDEN_SQL);

  LEX_CSTRING field_name_cstr = {field_name->str, field_name->length};

  if (check_string_char_length(field_name_cstr, "", NAME_CHAR_LEN,
                               system_charset_info, true)) {
    my_error(ER_TOO_LONG_IDENT, MYF(0),
             field_name->str); /* purecov: inspected */
    return true;               /* purecov: inspected */
  }
  if (type_modifier & PRI_KEY_FLAG) {
    List<Key_part_spec> key_parts;
    auto key_part_spec =
        new (thd->mem_root) Key_part_spec(field_name_cstr, 0, ORDER_ASC);
    if (key_part_spec == nullptr || key_parts.push_back(key_part_spec))
      return true;
    Key_spec *key = new (thd->mem_root)
        Key_spec(thd->mem_root, KEYTYPE_PRIMARY, NULL_CSTR,
                 &default_key_create_info, false, true, key_parts);
    if (key == nullptr || key_list.push_back(key)) return true;
  }
  if (type_modifier & (UNIQUE_FLAG | UNIQUE_KEY_FLAG)) {
    List<Key_part_spec> key_parts;
    auto key_part_spec =
        new (thd->mem_root) Key_part_spec(field_name_cstr, 0, ORDER_ASC);
    if (key_part_spec == nullptr || key_parts.push_back(key_part_spec))
      return true;
    Key_spec *key = new (thd->mem_root)
        Key_spec(thd->mem_root, KEYTYPE_UNIQUE, NULL_CSTR,
                 &default_key_create_info, false, true, key_parts);
    if (key == nullptr || key_list.push_back(key)) return true;
  }

  if (default_value) {
    /*
      Default value should be literal => basic constants =>
      no need fix_fields()

      We allow only CURRENT_TIMESTAMP as function default for the TIMESTAMP or
      DATETIME types. In addition, TRUE and FALSE are allowed for bool types.
    */
    if (default_value->type() == Item::FUNC_ITEM) {
      Item_func *func = down_cast<Item_func *>(default_value);
      if (func->basic_const_item()) {
        if (func->result_type() != INT_RESULT) {
          my_error(ER_INVALID_DEFAULT, MYF(0), field_name->str);
          return true;
        }
        assert(dynamic_cast<Item_func_true *>(func) ||
               dynamic_cast<Item_func_false *>(func));
        default_value = new Item_int(func->val_int());
        if (default_value == nullptr) return true;
      } else if (func->functype() != Item_func::NOW_FUNC ||
                 !real_type_with_now_as_default(type) ||
                 default_value->decimals != datetime_precision) {
        my_error(ER_INVALID_DEFAULT, MYF(0), field_name->str);
        return true;
      }
    } else if (default_value->type() == Item::NULL_ITEM) {
      default_value = nullptr;
      if ((type_modifier & (NOT_NULL_FLAG | AUTO_INCREMENT_FLAG)) ==
          NOT_NULL_FLAG) {
        my_error(ER_INVALID_DEFAULT, MYF(0), field_name->str);
        return true;
      }
    } else if (type_modifier & AUTO_INCREMENT_FLAG) {
      my_error(ER_INVALID_DEFAULT, MYF(0), field_name->str);
      return true;
    }
  }

  // 1) Reject combinations of DEFAULT <value> and DEFAULT (<expression>).
  // 2) Reject combinations of DEFAULT (<expression>) and AUTO_INCREMENT.
  // (Combinations of DEFAULT <value> and AUTO_INCREMENT are rejected above.)
  if ((default_val_expr && default_value) ||
      (default_val_expr && (type_modifier & AUTO_INCREMENT_FLAG))) {
    my_error(ER_INVALID_DEFAULT, MYF(0), field_name->str);
    return true;
  }

  if (on_update_value && (!real_type_with_now_on_update(type) ||
                          on_update_value->decimals != datetime_precision)) {
    my_error(ER_INVALID_ON_UPDATE, MYF(0), field_name->str);
    return true;
  }

  // If the SRID is specified on a non-geometric column, return an error
  if (type != MYSQL_TYPE_GEOMETRY && srid.has_value()) {
    my_error(ER_WRONG_USAGE, MYF(0), "SRID", "non-geometry column");
    return true;
  }

  Create_field *new_field = new (thd->mem_root) Create_field();
  if ((new_field == nullptr) ||
      new_field->init(thd, field_name->str, type, length, decimals,
                      type_modifier, default_value, on_update_value, comment,
                      change, interval_list, cs, has_explicit_collation,
                      uint_geom_type, gcol_info, default_val_expr, srid, hidden,
                      is_array))
    return true;

  for (const auto &a : cf_appliers) {
    if (a(new_field, this)) return true;
  }

  create_list.push_back(new_field);
  if (opt_after != nullptr) {
    flags |= Alter_info::ALTER_COLUMN_ORDER;
    new_field->after = opt_after;
  }

  if (col_check_const_spec_list) {
    /*
      Set column name, required for column check constraint validation in
      Sql_check_constraint_spec::pre_validate().
    */
    for (auto &cc_spec : *col_check_const_spec_list) {
      cc_spec->column_name = *field_name;
    }
    /*
      Move column check constraint specifications to table check constraints
      specfications list.
    */
    std::move(col_check_const_spec_list->begin(),
              col_check_const_spec_list->end(),
              std::back_inserter(check_constraint_spec_list));
  }

  return false;
}


// Source: sql_parse.cc
// Lines 5126-5267
