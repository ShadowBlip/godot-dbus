#include "dbus_message.h"

using godot::ClassDB;
using godot::D_METHOD;
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
