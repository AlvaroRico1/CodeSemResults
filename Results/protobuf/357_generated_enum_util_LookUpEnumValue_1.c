bool LookUpEnumValue(const EnumEntry* enums, size_t size,
                     StringPiece name, int* value) {
  EnumEntry target{name, 0};
  auto it = std::lower_bound(enums, enums + size, target, EnumCompareByName);
  if (it != enums + size && it->name == name) {
    *value = it->value;
    return true;
  }
  return false;
}

int LookUpEnumName(const EnumEntry* enums, const int* sorted_indices,
                   size_t size, int value) {
  auto comparator = [enums, value](int a, int b) {
    return GetValue(enums, a, value) < GetValue(enums, b, value);
  };
  auto it =
      std::lower_bound(sorted_indices, sorted_indices + size, -1, comparator);
  if (it != sorted_indices + size && enums[*it].value == value) {
    return it - sorted_indices;
  }
  return -1;
}

bool InitializeEnumStrings(
    const EnumEntry* enums, const int* sorted_indices, size_t size,
    internal::ExplicitlyConstructed<std::string>* enum_strings) {
  for (size_t i = 0; i < size; ++i) {
    enum_strings[i].Construct(enums[sorted_indices[i]].name);
    internal::OnShutdownDestroyString(enum_strings[i].get_mutable());
  }
  return true;
}

}  // namespace internal


// Source: generated_enum_util.cc
// Lines 59-93
