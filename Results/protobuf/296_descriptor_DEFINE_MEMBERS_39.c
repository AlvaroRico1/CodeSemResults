  DEFINE_MEMBERS(Subpackage, SUB_PACKAGE, sub_package_file_descriptor)

  // Enum values have two different parents.
  // We use two different identitied for the same object to determine the two
  // different insertions in the map.
  static Symbol EnumValue(EnumValueDescriptor* value, int n) {
    Symbol s;
    internal::SymbolBase* ptr;
    if (n == 0) {
      ptr = static_cast<internal::SymbolBaseN<0>*>(value);
      ptr->symbol_type_ = ENUM_VALUE;
    } else {
      ptr = static_cast<internal::SymbolBaseN<1>*>(value);
      ptr->symbol_type_ = ENUM_VALUE_OTHER_PARENT;
    }
    s.ptr_ = ptr;
    return s;
  }


// Source: descriptor.cc
// Lines 614-631
