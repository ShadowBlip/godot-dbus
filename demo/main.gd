extends Control


# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	var bluez := Bluez.new()
	print(bluez.get_discovery_filters())
	#print(bluez.get_managed_objects())

	bluez.start_discovery()

	bluez.get_managed_objects()
