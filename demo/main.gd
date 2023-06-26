extends Control


# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	var dbus := DBus.new()
	if dbus.connect(dbus.DBUS_BUS_SYSTEM) != OK:
		print("Unable to connect to dbus!")
		return
	
	print("Connected to dbus: " + dbus.get_unique_name())

	var msg := dbus.send_with_reply_and_block("org.freedesktop.DBus", "/", "org.freedesktop.DBus.Introspectable", "Introspect")
	print(msg.is_empty())

	print(msg.get_type())
	print(msg.get_string_args())

	var msg2 := dbus.send_with_reply_and_block("org.freedesktop.DBus", "/", "org.freedesktop.DBus.Introspectable", "IDontExist")
