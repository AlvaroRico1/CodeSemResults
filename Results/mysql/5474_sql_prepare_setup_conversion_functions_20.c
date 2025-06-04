static void setup_conversion_functions(Prepared_statement *stmt,
                                       PS_PARAM *parameters) {
  /*
    First execute or types altered by the client, setup the
    conversion routines for all parameters (one time)
  */
  Item_param **it = stmt->param_array;
  Item_param **end = it + stmt->param_count;
  for (uint i = 0; it < end; ++it, ++i) {
    Item_param *const param = *it;
    param->set_type_actual(parameters[i].type, parameters[i].unsigned_type);
    setup_one_conversion_function(param,
                                  stmt->thd->variables.character_set_client);
    param->sync_clones();
  }


// Source: sql_prepare.cc
// Lines 857-871
