inline const EnumValueDescriptor* FileDescriptorTables::FindEnumValueByNumber(
    const EnumDescriptor* parent, int number) const {
  // If `number` is within the sequential range, just index into the parent
  // without doing a table lookup.
  const int base = parent->value(0)->number();
  if (base <= number &&
      number <= static_cast<int64_t>(base) + parent->sequential_value_limit_) {
    return parent->value(number - base);
  }

  Symbol::QueryKey query;
  query.parent = parent;
  query.field_number = number;

  auto it = enum_values_by_number_.find(query);
  return it == enum_values_by_number_.end() ? nullptr
                                            : it->enum_value_descriptor();
}


// Source: descriptor.cc
// Lines 1639-1656
