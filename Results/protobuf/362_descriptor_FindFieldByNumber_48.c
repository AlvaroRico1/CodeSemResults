inline const FieldDescriptor* FileDescriptorTables::FindFieldByNumber(
    const Descriptor* parent, int number) const {
  // If `number` is within the sequential range, just index into the parent
  // without doing a table lookup.
  if (parent != nullptr &&  //
      1 <= number && number <= parent->sequential_field_limit_) {
    return parent->field(number - 1);
  }

  Symbol::QueryKey query;
  query.parent = parent;
  query.field_number = number;

  auto it = fields_by_number_.find(query);
  return it == fields_by_number_.end() ? nullptr : it->field_descriptor();
}


// Source: descriptor.cc
// Lines 1557-1572
