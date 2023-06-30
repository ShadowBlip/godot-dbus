#include "dbus_message.h"
#include "dbus/dbus-protocol.h"

using godot::Array;
using godot::ClassDB;
using godot::D_METHOD;
using godot::Dictionary;
using godot::PackedStringArray;
using godot::String;

DBusMessage::DBusMessage(){};
DBusMessage::~DBusMessage() {
  if (message == nullptr) {
    return;
  }
  ::dbus_message_unref(message);
};

bool DBusMessage::is_empty() { return (message == nullptr); }

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

// Convert a DBus dictionary into a godot dictionary
Dictionary get_arg_dictionary(DBusMessageIter *iter) {
  Dictionary dict = Dictionary();

  return dict;
}

// Convert a DBus array into a godot array
Array get_arg_array(DBusMessageIter *iter) {
  Array arr = Array();

  // Get the number of elements and type of the array
  int element_count = ::dbus_message_iter_get_element_count(iter);
  int array_type = ::dbus_message_iter_get_element_type(iter);
  char array_char = (char)array_type;
  godot::UtilityFunctions::print("Found array type: ", String(&array_char));

  // Iterate through the container with a sub-iterator
  DBusMessageIter sub_iter;
  ::dbus_message_iter_recurse(iter, &sub_iter);

  // Handle string arrays
  if (array_type == DBUS_TYPE_STRING) {
    godot::UtilityFunctions::print("Found string array type!");
    // PackedStringArray arr = PackedStringArray();
    for (int i = 0; i < element_count; i++) {
      const char *value;
      ::dbus_message_iter_get_basic(&sub_iter, &value);
      arr.append(String(value));
      ::dbus_message_iter_next(&sub_iter);
    }

    return arr;
  }

  // Handle dictionary arrays
  if (array_type == DBUS_TYPE_DICT_ENTRY) {
    godot::UtilityFunctions::print("Found dict array type!");
    for (int i = 0; i < element_count; i++) {
      Dictionary dict = get_arg_dictionary(&sub_iter);
      arr.append(dict);
    }

    return arr;
  }

  //// Handle dictionaries
  // if (array_type == DBUS_TYPE_DICT_ENTRY) {
  //   godot::UtilityFunctions::print("Found dictionary entry type!");

  //  Dictionary dict = Dictionary();
  //  DBusMessageIter sub_iter;
  //  ::dbus_message_iter_recurse(&iter, &sub_iter);

  //  for (int i = 0; i < element_count; i++) {
  //    // Get key
  //    int key_type = ::dbus_message_iter_get_arg_type(&sub_iter);
  //    godot::UtilityFunctions::print("Found key type: ", key_type);
  //    const char *key;
  //    ::dbus_message_iter_get_basic(&sub_iter, &key);
  //    ::dbus_message_iter_next(&sub_iter);

  //    // Get value
  //    int sub_arg_type = ::dbus_message_iter_get_arg_type(&sub_iter);
  //    dict[key] = "<unkn>";
  //    ::dbus_message_iter_next(&sub_iter);
  //  }

  //  args.append(dict);
  //}

  return arr;
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

    // Handle arrays
    if (arg_type == DBUS_TYPE_ARRAY) {
      Array arr = get_arg_array(&iter);
      args.append(arr);
    }

    // Handle basic shit
    if (arg_type == DBUS_TYPE_STRING) {
      godot::UtilityFunctions::print("Found string type!");
      const char *value;
      ::dbus_message_iter_get_basic(&iter, &value);
      args.append(String(value));
    }
    if (arg_type == DBUS_TYPE_INT32) {
      godot::UtilityFunctions::print("Found int32 type!");
      int *value;
      ::dbus_message_iter_get_basic(&iter, &value);
      args.append(*value);
    }

    dbus_message_iter_next(&iter);
  }

  return args;
}

// Gets the arguments from the message
// https://dbus.freedesktop.org/doc/api/html/group__DBusMessage.html#gad8953f53ceea7de81cde792e3edd0230
String DBusMessage::get_string_args() {
  // Create an initialize the error struct
  DBusError dbus_error;
  ::dbus_error_init(&dbus_error);

  // Store the result
  const char *dbus_result = nullptr;

  // Get the arguments from the message
  bool ok = ::dbus_message_get_args(message, &dbus_error, DBUS_TYPE_STRING,
                                    &dbus_result, DBUS_TYPE_INVALID);
  if (!ok) {
    godot::UtilityFunctions::push_warning(
        "Error getting message args: ", dbus_error.name, dbus_error.message);
    ::dbus_error_free(&dbus_error);
    return String();
  }
  ::dbus_error_free(&dbus_error);

  return String(dbus_result);
};

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
  ClassDB::bind_method(D_METHOD("get_type"), &DBusMessage::get_type);
  ClassDB::bind_method(D_METHOD("get_signature"), &DBusMessage::get_signature);
  ClassDB::bind_method(D_METHOD("get_args"), &DBusMessage::get_args);
  ClassDB::bind_method(D_METHOD("get_error_name"),
                       &DBusMessage::get_error_name);
  ClassDB::bind_method(D_METHOD("get_string_args"),
                       &DBusMessage::get_string_args);
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
