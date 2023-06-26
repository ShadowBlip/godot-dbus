#ifndef DBUS_MESSAGE_CLASS_H
#define DBUS_MESSAGE_CLASS_H

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dbus/dbus.h>
#include <iostream>

#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/variant/packed_byte_array.hpp"
#include "godot_cpp/variant/packed_string_array.hpp"
#include "godot_cpp/variant/string.hpp"
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

class DBusMessage : public godot::RefCounted {
  GDCLASS(DBusMessage, godot::RefCounted);

protected:
  static void _bind_methods();

private:
public:
  // Constructor/deconstructor
  DBusMessage();
  ~DBusMessage();

  // Properties
  ::DBusMessage *message;

  // Methods
  bool is_empty();
  int get_type();
  void new_method_call(godot::String bus_name, godot::String path,
                       godot::String iface, godot::String method);
  godot::String get_string_args();
};

#endif // DBUS_MESSAGE_CLASS_H
