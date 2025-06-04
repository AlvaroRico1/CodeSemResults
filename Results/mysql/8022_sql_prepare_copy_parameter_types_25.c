void Prepared_statement::copy_parameter_types(Item_param **from_param_array) {
  for (uint i = 0; i < param_count; ++i) {
    Item_param *from = from_param_array[i];
    Item_param *to = this->param_array[i];
    to->copy_param_actual_type(from);
  }


// Source: sql_prepare.cc
// Lines 849-854
