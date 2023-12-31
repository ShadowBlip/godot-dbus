#ifndef DBUS_MESSAGE_CLASS_H
#define DBUS_MESSAGE_CLASS_H

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dbus/dbus.h>
#include <iostream>

#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/packed_byte_array.hpp"
#include "godot_cpp/variant/packed_string_array.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/variant.hpp"
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
  bool is_signal(godot::String iface, godot::String name);
  godot::String get_error_name();
  godot::String get_signature();
  void new_method_call(godot::String bus_name, godot::String path,
                       godot::String iface, godot::String method);
  godot::Array get_args();
  godot::String get_path();
  godot::String get_sender();
  godot::String get_member();
};

godot::Variant get_arg(DBusMessageIter *iter);

#endif // DBUS_MESSAGE_CLASS_H
