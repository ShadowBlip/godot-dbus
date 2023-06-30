extends Resource
class_name Bluez

var dbus := DBus.new()


func _init() -> void:
	if dbus.connect(dbus.DBUS_BUS_SYSTEM) != OK:
		print("Unable to connect to dbus!")


func get_discovery_filters(device: String = "hci0") -> PackedStringArray:
	var path := "/".join(["/org/bluez", device])
	var response := dbus.send_with_reply_and_block("org.bluez", path, 'org.bluez.Adapter1', "GetDiscoveryFilters", [])
	
	return response.get_args()[0]


func get_managed_objects():
	var response := dbus.send_with_reply_and_block("org.bluez", "/", 'org.freedesktop.DBus.ObjectManager', "GetManagedObjects", [])
	print(response.get_args())


func start_discovery():
	var response := dbus.send_with_reply_and_block("org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1", "StartDiscovery", [])


func stop_discovery():
	var response := dbus.send_with_reply_and_block("org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1", "StopDiscovery", [])
