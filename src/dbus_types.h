#ifndef DBUS_TYPES_CLASS_H
#define DBUS_TYPES_CLASS_H

#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/variant/dictionary.hpp"
#include "godot_cpp/variant/packed_byte_array.hpp"
#include "godot_cpp/variant/packed_string_array.hpp"
#include "godot_cpp/variant/string.hpp"
#include "godot_cpp/variant/variant.hpp"
#include <cstdint>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

// Base class for DBusTypes
class DBusType : public godot::RefCounted {
  GDCLASS(DBusType, godot::RefCounted);

protected:
  static void _bind_methods();

private:
public:
  DBusType();
  ~DBusType();
};

// UInt32 wrapper
class DBusUInt32 : public DBusType {
  GDCLASS(DBusUInt32, DBusType);

protected:
  static void _bind_methods();

private:
  uint32_t value;

public:
  DBusUInt32();
  ~DBusUInt32();
  void set_value(uint32_t v);
  uint32_t get_value();
};

#endif // DBUS_TYPES_CLASS_H
