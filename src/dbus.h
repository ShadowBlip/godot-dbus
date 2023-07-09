#ifndef DBUS_CLASS_H
#define DBUS_CLASS_H

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dbus/dbus.h>
#include <iostream>

#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/variant/array.hpp"
#include "godot_cpp/variant/packed_byte_array.hpp"
#include "godot_cpp/variant/packed_string_array.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/variant.hpp"
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "dbus_message.h"

class DBus : public godot::RefCounted {
  GDCLASS(DBus, godot::RefCounted);

protected:
  static void _bind_methods();

private:
  DBusConnection *dbus_conn = nullptr;

public:
  // Constructor/deconstructor
  DBus();
  ~DBus();

  // Methods
  int add_match(godot::String match);
  int remove_match(godot::String match);
  int connect(int bus_type);
  godot::String get_unique_name();
  DBusMessage *pop_message();
  bool name_has_owner(godot::String name);
  int request_name(godot::String name, unsigned int flags);
  DBusMessage *send_with_reply_and_block(godot::String bus_name,
                                         godot::String path,
                                         godot::String iface,
                                         godot::String method,
                                         godot::Array args);
};

#endif // DBUS_CLASS_H
