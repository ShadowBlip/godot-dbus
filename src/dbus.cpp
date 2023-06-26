#include "dbus.h"
#include "dbus_message.h"
#include "godot_cpp/variant/array.hpp"
#include "godot_cpp/variant/variant.hpp"

// References:
// http://www.matthew.ath.cx/misc/dbus
// https://github.com/makercrew/dbus-sample

using godot::Array;
using godot::ClassDB;
using godot::D_METHOD;
using godot::String;
using godot::Variant;

DBus::DBus(){};
DBus::~DBus() {
  if (dbus_conn == nullptr) {
    return;
  }
  // When using the System Bus, unreference
  // the connection instead of closing it
  ::dbus_connection_unref(dbus_conn);
};

// Connect to the dbus interface
int DBus::connect(int bus_type) {
  DBusError dbus_error;

  // Initialize D-Bus error
  ::dbus_error_init(&dbus_error);

  // Connect to dbus
  dbus_conn = ::dbus_bus_get(DBUS_BUS_SYSTEM, &dbus_error);
  if (dbus_conn == nullptr) {
    godot::UtilityFunctions::push_warning(
        "Unable to connect to bus: ", dbus_error.name, dbus_error.message);
    return godot::ERR_CANT_CONNECT;
  }

  return godot::OK;
};

// Send the given message and wait for a reply
DBusMessage *DBus::send_with_reply_and_block(String bus_name, String path,
                                             String iface, String method) {
  if (dbus_conn == nullptr) {
    godot::UtilityFunctions::push_error("No dbus connection exists");
    return nullptr;
  }

  // Create an initialize the error struct
  DBusError dbus_error;
  ::dbus_error_init(&dbus_error);

  // Build the message to send
  ::DBusMessage *reply = nullptr;
  ::DBusMessage *msg = ::dbus_message_new_method_call(
      bus_name.ascii().get_data(), path.ascii().get_data(),
      iface.ascii().get_data(), method.ascii().get_data());

  // Add arguments to the message
  // for (int i = 0; i < args.size(); i++) {
  //  Variant variant = args[i];
  //  switch (variant.get_type()) {
  //  case variant.STRING:
  //    String arg = (String)variant;
  //    ::dbus_message_append_args(msg, DBUS_TYPE_STRING,
  //    arg.ascii().get_data(),
  //                               DBUS_TYPE_INVALID);
  //    break;
  //  }
  //  godot::UtilityFunctions::push_warning("Invalid argument type");
  //  return nullptr;
  //}

  // Send the message and check for errors
  reply = ::dbus_connection_send_with_reply_and_block(
      dbus_conn, msg, DBUS_TIMEOUT_USE_DEFAULT, &dbus_error);
  if (reply == nullptr) {
    godot::UtilityFunctions::push_warning(
        "Unable to send message: ", dbus_error.name, dbus_error.message);
    ::dbus_error_free(&dbus_error);
    return nullptr;
  }

  // Clean up the message
  ::dbus_message_unref(msg);
  ::dbus_error_free(&dbus_error);

  // Create a new message object to contain the reply
  DBusMessage *response = memnew(DBusMessage());
  response->message = reply;

  return response;
};

// Return the unique name of the client on the bus.
String DBus::get_unique_name() {
  if (dbus_conn == nullptr) {
    return String();
  }
  return String(::dbus_bus_get_unique_name(dbus_conn));
}

// Asks the bus to assign the given name to this connection by invoking the
// RequestName method on the bus.
int DBus::request_name(String name, unsigned int flags) {
  if (dbus_conn == nullptr) {
    godot::UtilityFunctions::push_error("No dbus connection exists");
    return godot::ERR_CANT_CONNECT;
  }

  // Create an initialize the error struct
  DBusError dbus_error;
  ::dbus_error_init(&dbus_error);

  int ret = ::dbus_bus_request_name(dbus_conn, name.ascii().get_data(), flags,
                                    &dbus_error);
  if (::dbus_error_is_set(&dbus_error)) {
    godot::UtilityFunctions::push_warning(
        "Failed to request name: ", dbus_error.name, dbus_error.message);
  }
  dbus_error_free(&dbus_error);

  return ret;
};

// Register the methods with Godot
void DBus::_bind_methods() {
  ClassDB::bind_method(D_METHOD("connect", "bus_type"), &DBus::connect);
  ClassDB::bind_method(D_METHOD("get_unique_name"), &DBus::get_unique_name);
  ClassDB::bind_method(D_METHOD("request_name", "name", "flags"),
                       &DBus::request_name);
  ClassDB::bind_method(D_METHOD("send_with_reply_and_block", "bus_name", "path",
                                "iface", "method"),
                       &DBus::send_with_reply_and_block);

  // Constants
  BIND_CONSTANT(DBUS_BUS_SESSION);
  BIND_CONSTANT(DBUS_BUS_SYSTEM);
  BIND_CONSTANT(DBUS_BUS_STARTER);
  BIND_CONSTANT(DBUS_NAME_FLAG_DO_NOT_QUEUE);
  BIND_CONSTANT(DBUS_NAME_FLAG_REPLACE_EXISTING);
  BIND_CONSTANT(DBUS_NAME_FLAG_ALLOW_REPLACEMENT);
  BIND_CONSTANT(DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER);
  BIND_CONSTANT(DBUS_REQUEST_NAME_REPLY_IN_QUEUE);
  BIND_CONSTANT(DBUS_REQUEST_NAME_REPLY_EXISTS);
  BIND_CONSTANT(DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER);
};
