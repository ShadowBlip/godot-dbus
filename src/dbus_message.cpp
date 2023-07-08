#include "dbus_message.h"
#include "dbus/dbus-protocol.h"

using godot::Array;
using godot::ClassDB;
using godot::D_METHOD;
using godot::Dictionary;
using godot::PackedStringArray;
using godot::String;
using godot::Variant;

DBusMessage::DBusMessage(){};
DBusMessage::~DBusMessage() {
  if (message == nullptr) {
    return;
  }
  ::dbus_message_unref(message);
};

bool DBusMessage::is_empty() { return (message == nullptr); }

// Returns true if the message matches the given interface and name
bool DBusMessage::is_signal(String iface, String name) {
  if (is_empty()) {
    return false;
  }
  return ::dbus_message_is_signal(message, iface.ascii().get_data(),
                                  name.ascii().get_data());
}

// Gets the type of a message.
// Types include DBUS_MESSAGE_TYPE_METHOD_CALL, DBUS_MESSAGE_TYPE_METHOD_RETURN,
// DBUS_MESSAGE_TYPE_ERROR, DBUS_MESSAGE_TYPE_SIGNAL, but other types are
// allowed and all code must silently ignore messages of unknown type.
// DBUS_MESSAGE_TYPE_INVALID will never be returned.
int DBusMessage::get_type() {
  if (is_empty()) {
    return -1;
  }

  return ::dbus_message_get_type(message);
};

// Return the path to the object this message is for or from
String DBusMessage::get_path() {
  if (is_empty()) {
    return String();
  }

  const char *path = ::dbus_message_get_path(message);
  return String(path);
}

// Return the sender of this message
String DBusMessage::get_sender() {
  if (is_empty()) {
    return String();
  }

  const char *sender = ::dbus_message_get_sender(message);
  return String(sender);
}

// Return the member of this message
String DBusMessage::get_member() {
  if (is_empty()) {
    return String();
  }

  const char *membr = ::dbus_message_get_member(message);
  return String(membr);
}

// Gets the type signature of the message, i.e. the arguments in the message
// payload. The signature is a string made up of type codes such as
// DBUS_TYPE_INT32. The string is terminated with nul (nul is also the value of
// DBUS_TYPE_INVALID).
String DBusMessage::get_signature() {
  if (is_empty()) {
    return String();
  }
  const char *signature = ::dbus_message_get_signature(message);

  return String(signature);
}

// Gets the error name (DBUS_MESSAGE_TYPE_ERROR only) or NULL if none.
String DBusMessage::get_error_name() {
  if (is_empty()) {
    return String();
  }
  const char *err = ::dbus_message_get_error_name(message);

  return String(err);
}

// Convert a DBus string into a godot string
String get_arg_string(DBusMessageIter *iter) {
  const char *value;
  ::dbus_message_iter_get_basic(iter, &value);
  return String(value);
}

// Convert a DBus variant into a I dunno wtf
Variant get_arg_variant(DBusMessageIter *iter) {
  // const char *signature = ::dbus_message_iter_get_signature(iter);
  // godot::UtilityFunctions::print("Found variant signature: ", signature);

  // Open the container and get a sub iterator that we can use to check the type
  DBusMessageIter sub_iter;
  ::dbus_message_iter_recurse(iter, &sub_iter);

  Variant value = get_arg(&sub_iter);

  return value;
}

// Convert a DBus dictionary into a godot dictionary. For DBus, a dictionary
// is actually an array of dictionary entries.
Dictionary get_arg_dictionary(DBusMessageIter *iter) {
  Dictionary dict = Dictionary();

  // A DBus Dictionary is actually an Array of DictionaryEntry objects.
  // Iterate through the array container with a sub-iterator.
  DBusMessageIter sub_iter;
  ::dbus_message_iter_recurse(iter, &sub_iter);

  // Loop through each DictionaryEntry in the array.
  int arg_type;
  while ((arg_type = ::dbus_message_iter_get_arg_type(&sub_iter)) !=
         DBUS_TYPE_INVALID) {

    // Recurse into the dictionary entry
    DBusMessageIter entry_iter;
    ::dbus_message_iter_recurse(&sub_iter, &entry_iter);

    // Loop through each DictionaryEntry object. This will alternate between key
    // and value
    int i = 0;
    Variant key;
    int entry_type;
    while ((entry_type = ::dbus_message_iter_get_arg_type(&entry_iter)) !=
           DBUS_TYPE_INVALID) {

      // Convert the key or value to a Godot type
      Variant item = get_arg(&entry_iter);

      // If it's even, it's a key, odd is a value
      if (i % 2 == 0) {
        key = item;
      } else {
        dict[key] = item;
      }

      ::dbus_message_iter_next(&entry_iter);
      i++;
    }

    ::dbus_message_iter_next(&sub_iter);
  }

  return dict;
}

// Convert a DBus array into a godot array
Array get_arg_array(DBusMessageIter *iter) {
  Array arr = Array();

  // Get the number of elements and type of the array
  int element_count = ::dbus_message_iter_get_element_count(iter);
  int array_type = ::dbus_message_iter_get_element_type(iter);
  char array_char = (char)array_type;
  // godot::UtilityFunctions::print("Found array type: ", String(&array_char));

  // Iterate through the container with a sub-iterator
  DBusMessageIter sub_iter;
  ::dbus_message_iter_recurse(iter, &sub_iter);

  // Loop through each item and add it to the array
  for (int i = 0; i < element_count; i++) {
    Variant value = get_arg(&sub_iter);
    arr.append(value);
    ::dbus_message_iter_next(&sub_iter);
  }

  return arr;
}

// Converts the given DBus argument to a Godot variant
Variant get_arg(DBusMessageIter *iter) {
  int arg_type = ::dbus_message_iter_get_arg_type(iter);
  char type[2];
  type[0] = (char)arg_type;
  type[1] = '\0';
  // godot::UtilityFunctions::print("Found type ", type);

  // Handle arrays and dictionaries
  if (arg_type == DBUS_TYPE_ARRAY) {
    // Check to see if this is actually a dictionary. A dictionary is an array
    // of dict entries
    int array_type = ::dbus_message_iter_get_element_type(iter);
    if (array_type == DBUS_TYPE_DICT_ENTRY) {
      // This is a dictionary!
      // godot::UtilityFunctions::print("Found dict type!");
      Dictionary dict = get_arg_dictionary(iter);
      return Variant(dict);
    }

    // godot::UtilityFunctions::print("Found array type!");
    Array arr = get_arg_array(iter);
    return Variant(arr);
  }

  // Handle basic shit
  if (arg_type == DBUS_TYPE_BYTE) {
    // godot::UtilityFunctions::print("Found byte type!");
    char value;
    ::dbus_message_iter_get_basic(iter, &value);
    return Variant(value);
  }
  if (arg_type == DBUS_TYPE_BOOLEAN) {
    // godot::UtilityFunctions::print("Found bool type!");
    bool value;
    ::dbus_message_iter_get_basic(iter, &value);
    return Variant(value);
  }
  if (arg_type == DBUS_TYPE_INT16) {
    // godot::UtilityFunctions::print("Found int16 type!");
    int16_t value;
    ::dbus_message_iter_get_basic(iter, &value);
    return Variant(value);
  }
  if (arg_type == DBUS_TYPE_INT32) {
    // godot::UtilityFunctions::print("Found int32 type!");
    int32_t value;
    ::dbus_message_iter_get_basic(iter, &value);
    return Variant(value);
  }
  if (arg_type == DBUS_TYPE_INT64) {
    // godot::UtilityFunctions::print("Found int64 type!");
    int64_t value;
    ::dbus_message_iter_get_basic(iter, &value);
    return Variant(value);
  }
  if (arg_type == DBUS_TYPE_UINT16) {
    // godot::UtilityFunctions::print("Found uint16 type!");
    uint16_t value;
    ::dbus_message_iter_get_basic(iter, &value);
    return Variant(value);
  }
  if (arg_type == DBUS_TYPE_UINT32) {
    // godot::UtilityFunctions::print("Found uint32 type!");
    uint32_t value;
    ::dbus_message_iter_get_basic(iter, &value);
    return Variant(value);
  }
  if (arg_type == DBUS_TYPE_UINT64) {
    // godot::UtilityFunctions::print("Found uint64 type!");
    uint64_t value;
    ::dbus_message_iter_get_basic(iter, &value);
    return Variant(value);
  }
  if (arg_type == DBUS_TYPE_DOUBLE) {
    // godot::UtilityFunctions::print("Found double type!");
    double_t value;
    ::dbus_message_iter_get_basic(iter, &value);
    return Variant(value);
  }
  if (arg_type == DBUS_TYPE_STRING || arg_type == DBUS_TYPE_OBJECT_PATH ||
      arg_type == DBUS_TYPE_SIGNATURE) {
    // godot::UtilityFunctions::print("Found string type!");
    String value = get_arg_string(iter);
    return Variant(value);
  }
  if (arg_type == DBUS_TYPE_VARIANT) {
    // godot::UtilityFunctions::print("Found variant type!");
    Variant value = get_arg_variant(iter);
    return value;
  }

  // TODO: Implement struct, unix fd
  // godot::UtilityFunctions::push_warning("Unknown type!");
  return Variant();
}

// Gets the arguments from the message
Array DBusMessage::get_args() {
  Array args = Array();
  if (is_empty()) {
    return args;
  }

  // Loop through each argument
  int arg_type;
  DBusMessageIter iter;
  ::dbus_message_iter_init(message, &iter);
  while ((arg_type = ::dbus_message_iter_get_arg_type(&iter)) !=
         DBUS_TYPE_INVALID) {
    Variant arg = get_arg(&iter);
    args.append(arg);
    dbus_message_iter_next(&iter);
  }

  return args;
}

// Configure the message as a method call
void DBusMessage::new_method_call(String bus_name, String path, String iface,
                                  String method) {
  message = ::dbus_message_new_method_call(
      bus_name.ascii().get_data(), path.ascii().get_data(),
      iface.ascii().get_data(), method.ascii().get_data());
};

// Register the methods with Godot
void DBusMessage::_bind_methods() {
  ClassDB::bind_method(D_METHOD("is_empty"), &DBusMessage::is_empty);
  ClassDB::bind_method(D_METHOD("is_signal", "iface", "name"),
                       &DBusMessage::is_signal);
  ClassDB::bind_method(D_METHOD("get_type"), &DBusMessage::get_type);
  ClassDB::bind_method(D_METHOD("get_path"), &DBusMessage::get_path);
  ClassDB::bind_method(D_METHOD("get_sender"), &DBusMessage::get_sender);
  ClassDB::bind_method(D_METHOD("get_member"), &DBusMessage::get_member);
  ClassDB::bind_method(D_METHOD("get_signature"), &DBusMessage::get_signature);
  ClassDB::bind_method(D_METHOD("get_args"), &DBusMessage::get_args);
  ClassDB::bind_method(D_METHOD("get_error_name"),
                       &DBusMessage::get_error_name);
  ClassDB::bind_method(
      D_METHOD("new_method_call", "bus_type", "path", "iface", "method"),
      &DBusMessage::new_method_call);

  // Constants
  BIND_CONSTANT(DBUS_MESSAGE_TYPE_INVALID);
  BIND_CONSTANT(DBUS_MESSAGE_TYPE_METHOD_CALL);
  BIND_CONSTANT(DBUS_MESSAGE_TYPE_METHOD_RETURN);
  BIND_CONSTANT(DBUS_MESSAGE_TYPE_ERROR);
  BIND_CONSTANT(DBUS_MESSAGE_TYPE_SIGNAL);
};
