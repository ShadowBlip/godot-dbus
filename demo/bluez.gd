extends Resource
class_name Bluez

var dbus := DBus.new()


func _init() -> void:
	if dbus.connect(dbus.DBUS_BUS_SYSTEM) != OK:
		print("Unable to connect to dbus!")
	
	# Watch for some signals
	if dbus.add_match("type='signal',sender='org.bluez',interface='org.freedesktop.DBus.Properties'") != OK:
		print("Unable to add matcher for signals!")


func _process(_delta: float) -> void:
	var msg := dbus.pop_message()
	if not msg:
		return
	
	# Message to process!
	print("Got message!")
	print(msg.get_args())


func get_discovery_filters(device: String = "hci0") -> PackedStringArray:
	var path := "/".join(["/org/bluez", device])
	var response := dbus.send_with_reply_and_block("org.bluez", path, 'org.bluez.Adapter1', "GetDiscoveryFilters", [], "")
	
	return response.get_args()[0]


func get_managed_objects():
	var response := dbus.send_with_reply_and_block("org.bluez", "/", 'org.freedesktop.DBus.ObjectManager', "GetManagedObjects", [], "")
	if not response:
		return

	# "GetManagedObjects" returns one argument; A dictionary of dictionaries
	var res := response.get_args()
	if res.size() != 1:
		return
	
	var objects := res[0] as Dictionary
	for obj_path in objects.keys():
		var data := objects[obj_path] as Dictionary
		
		# Look for objects that implement the 'org.bluez.Device1' interface
		if not "org.bluez.Device1" in data:
			continue
		var device := data["org.bluez.Device1"] as Dictionary
		
		# Look for the address and name
		print("Discovered device [", device["Address"], "] at object path: ", obj_path)


func start_discovery():
	var response := dbus.send_with_reply_and_block("org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1", "StartDiscovery", [], "")


func stop_discovery():
	var response := dbus.send_with_reply_and_block("org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1", "StopDiscovery", [], "")


func set_powered(powered: bool):
	var response := dbus.send_with_reply_and_block(
			"org.bluez", 
			"/org/bluez/hci0", 
			"org.freedesktop.DBus.Properties", 
			"Set", 
			["org.bluez.Adapter1", "Powered", powered],
			"ssv"
		)
