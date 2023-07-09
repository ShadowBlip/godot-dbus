#include "dbus.h"
#include "dbus/dbus-protocol.h"
#include "dbus_message.h"
#include "godot_cpp/variant/utility_functions.hpp"

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
  dbus_conn = ::dbus_bus_get((DBusBusType)bus_type, &dbus_error);
  if (dbus_conn == nullptr) {
    godot::UtilityFunctions::push_warning(
        "Unable to connect to bus: ", dbus_error.name, dbus_error.message);
    return godot::ERR_CANT_CONNECT;
  }

  return godot::OK;
};

// Adds a match rule to match messages going through the message bus.
// The "rule" argument is the string form of a match rule.
// Example: "type='signal',interface='test.signal.Type'"
int DBus::add_match(godot::String rule) {
  if (dbus_conn == nullptr) {
    godot::UtilityFunctions::push_error("No dbus connection exists");
    return godot::ERR_CONNECTION_ERROR;
  }
  // Create an initialize the error struct
  DBusError dbus_error;
  ::dbus_error_init(&dbus_error);

  // Add the match
  ::dbus_bus_add_match(dbus_conn, rule.ascii().get_data(), &dbus_error);
  ::dbus_connection_flush(dbus_conn);

  // Check if an error occurred
  if (::dbus_error_is_set(&dbus_error)) {
    godot::UtilityFunctions::push_warning(
        "Unable to add match: ", dbus_error.name, " ", dbus_error.message);
    ::dbus_error_free(&dbus_error);
    return godot::ERR_CANT_CREATE;
  }

  ::dbus_error_free(&dbus_error);
  return godot::OK;
}

// Removes a previously-added match rule "by value"
// The "rule" argument is the string form of a match rule.
// Example: "type='signal',interface='test.signal.Type'"
int DBus::remove_match(godot::String rule) {
  if (dbus_conn == nullptr) {
    godot::UtilityFunctions::push_error("No dbus connection exists");
    return godot::ERR_CONNECTION_ERROR;
  }
  // Create an initialize the error struct
  DBusError dbus_error;
  ::dbus_error_init(&dbus_error);

  // Add the match
  ::dbus_bus_remove_match(dbus_conn, rule.ascii().get_data(), &dbus_error);

  // Check if an error occurred
  if (::dbus_error_is_set(&dbus_error)) {
    godot::UtilityFunctions::push_warning(
        "Unable to remove match: ", dbus_error.name, " ", dbus_error.message);
    ::dbus_error_free(&dbus_error);
    return godot::ERR_CANT_CREATE;
  }

  ::dbus_error_free(&dbus_error);
  return godot::OK;
}

// Pop the next available message from the bus and return it. This should be
// used in conjunction with add_match to listen for messages.
DBusMessage *DBus::pop_message() {
  if (dbus_conn == nullptr) {
    godot::UtilityFunctions::push_error("No dbus connection exists");
    return nullptr;
  }

  // non blocking read of the next available message
  ::dbus_connection_read_write(dbus_conn, 0);
  ::DBusMessage *msg = ::dbus_connection_pop_message(dbus_conn);
  if (msg == nullptr) {
    return nullptr;
  }

  // Create a new message object to contain the reply
  DBusMessage *response = memnew(DBusMessage());
  response->message = msg;

  return response;
}

// Sets the given argument on the DBusMessage with the given iterator
void append_arg(DBusMessageIter *iter, Variant variant) {
  auto arg_type = variant.get_type();
  if (arg_type == variant.STRING) {
    String arg = String(variant);
    // Duplicate the string and append it to the message
    const char *data = String(arg.ascii().get_data()).ascii().get_data();
    puts(data);
    ::dbus_message_iter_append_basic(iter, DBUS_TYPE_STRING, &data);
    puts(data);
    return;
  }
  if (arg_type == variant.INT) {
    dbus_int32_t arg = (dbus_int32_t)variant;
    ::dbus_message_iter_append_basic(iter, DBUS_TYPE_INT32, &arg);
    return;
  }
  if (arg_type == variant.BOOL) {
    dbus_bool_t arg = variant.booleanize();
    // HACK
    // TODO: We need to figure out the signature, then set the types. This is
    // an example of passing a BOOL as a variant
    DBusMessageIter sub_iter;
    ::dbus_message_iter_open_container(iter, DBUS_TYPE_VARIANT,
                                       DBUS_TYPE_BOOLEAN_AS_STRING, &sub_iter);
    ::dbus_message_iter_append_basic(&sub_iter, DBUS_TYPE_BOOLEAN, &arg);
    ::dbus_message_iter_close_container(iter, &sub_iter);
    return;
  }

  godot::UtilityFunctions::push_warning("Invalid/unhandled argument type");
}

// Send the given message and wait for a reply
DBusMessage *DBus::send_with_reply_and_block(String bus_name, String path,
                                             String iface, String method,
                                             Array args) {
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

  // Create an iterator to append arguments to the message
  DBusMessageIter iter;
  ::dbus_message_iter_init_append(msg, &iter);

  // Add arguments to the message
  for (int i = 0; i < args.size(); i++) {
    Variant variant = args[i];
    append_arg(&iter, variant);
  }

  // PRINT THE MSG!
  DBusMessage *new_msg = memnew(DBusMessage());
  new_msg->message = msg;
  godot::UtilityFunctions::print(new_msg->get_args());

  // Send the message and check for errors
  reply = ::dbus_connection_send_with_reply_and_block(
      dbus_conn, msg, DBUS_TIMEOUT_USE_DEFAULT, &dbus_error);
  if (reply == nullptr) {
    godot::UtilityFunctions::push_warning(
        "Unable to send message: ", dbus_error.name, " ", dbus_error.message);
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
bool DBus::name_has_owner(String name) {
  if (dbus_conn == nullptr) {
    godot::UtilityFunctions::push_error("No dbus connection exists");
    return false;
  }

  // Create an initialize the error struct
  DBusError dbus_error;
  ::dbus_error_init(&dbus_error);

  bool ret = ::dbus_bus_name_has_owner(dbus_conn, name.ascii().get_data(),
                                       &dbus_error);
  if (::dbus_error_is_set(&dbus_error)) {
    godot::UtilityFunctions::push_warning(
        "Failed to see if name has owner: ", dbus_error.name, " ",
        dbus_error.message);
  }
  dbus_error_free(&dbus_error);

  return ret;
};

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
        "Failed to request name: ", dbus_error.name, " ", dbus_error.message);
  }
  dbus_error_free(&dbus_error);

  return ret;
};

// Register the methods with Godot
void DBus::_bind_methods() {
  ClassDB::bind_method(D_METHOD("add_match", "rule"), &DBus::add_match);
  ClassDB::bind_method(D_METHOD("remove_match", "rule"), &DBus::remove_match);
  ClassDB::bind_method(D_METHOD("connect", "bus_type"), &DBus::connect);
  ClassDB::bind_method(D_METHOD("get_unique_name"), &DBus::get_unique_name);
  ClassDB::bind_method(D_METHOD("request_name", "name", "flags"),
                       &DBus::request_name);
  ClassDB::bind_method(D_METHOD("name_has_owner", "name"),
                       &DBus::name_has_owner);
  ClassDB::bind_method(D_METHOD("pop_message"), &DBus::pop_message);
  ClassDB::bind_method(D_METHOD("send_with_reply_and_block", "bus_name", "path",
                                "iface", "method", "args"),
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
