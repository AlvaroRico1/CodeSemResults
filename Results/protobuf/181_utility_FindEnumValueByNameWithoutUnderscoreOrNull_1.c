const google::protobuf::EnumValue* FindEnumValueByNameWithoutUnderscoreOrNull(
    const google::protobuf::Enum* enum_type, StringPiece enum_name) {
  if (enum_type != nullptr) {
    for (int i = 0; i < enum_type->enumvalue_size(); ++i) {
      const google::protobuf::EnumValue& enum_value = enum_type->enumvalue(i);
      std::string enum_name_without_underscore = enum_value.name();

      // Remove underscore from the name.
      enum_name_without_underscore.erase(
          std::remove(enum_name_without_underscore.begin(),
                      enum_name_without_underscore.end(), '_'),
          enum_name_without_underscore.end());
      // Make the name uppercase.
      for (std::string::iterator it = enum_name_without_underscore.begin();
           it != enum_name_without_underscore.end(); ++it) {
        *it = ascii_toupper(*it);
      }

      if (enum_name_without_underscore == enum_name) {
        return &enum_value;
      }
    }


// Source: utility.cc
// Lines 221-242
