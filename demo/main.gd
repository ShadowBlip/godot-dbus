extends Control

var bluez := Bluez.new()

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	bluez.set_powered(true)
	bluez.start_discovery()
	bluez.get_managed_objects()


func _process(delta: float) -> void:
	bluez._process(delta)
