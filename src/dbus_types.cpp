#include "dbus_types.h"

// Base
DBusType::DBusType(){};
DBusType::~DBusType(){};
void DBusType::_bind_methods(){};

// UInt32
DBusUInt32::DBusUInt32(){};
DBusUInt32::~DBusUInt32(){};
void DBusUInt32::set_value(uint32_t v) { value = v; };
uint32_t DBusUInt32::get_value() { return value; };
void DBusUInt32::_bind_methods(){};
