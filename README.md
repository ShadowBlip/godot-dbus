# Godot DBus

Godot DBus is a native GDExtension for Godot 4.x that allows you to interact
with DBus.

## Requirements

### Build Requirements

The following are required to build Open Gamepad UI:

- Godot 4.x
- GCC 7+ or Clang 6+.
- Python 3.5+.
- SCons 3.0+ build system
- pkg-config (used to detect the dependencies below).
- X11, Xcursor, Xinerama, Xi and XRandR development libraries.
- MesaGL development libraries.
- ALSA development libraries.
- PulseAudio development libraries.
- make (optional)
- unzip (optional)
- wget (optional)

If you are using ArchLinux, you can run the following:

```bash
pacman -S --needed scons pkgconf gcc libxcursor libxinerama libxi libxrandr mesa glu libglvnd alsa-lib pulseaudio make unzip wget git
```

## Building

You can build the project using the following:

```bash
make build
```

## Usage

Copy the `addons` folder after you have built the project into your Godot
project directory.

## Exploring DBus

Reference: https://ukbaz.github.io/howto/python_gio_1.html

### List objects from a service

```
$ busctl tree org.bluez
└─/org
  └─/org/bluez
    └─/org/bluez/hci0
```

### Find methods/properties

```
$ busctl introspect org.bluez /org/bluez/hci0
NAME                                TYPE      SIGNATURE RESULT/VALUE                          >
org.bluez.Adapter1                  interface -         -                                     >
.GetDiscoveryFilters                method    -         as                                    >
NAME                                TYPE      SIGNATURE RESULT/VALUE                          >
org.bluez.Adapter1                  interface -         -                                     >
.GetDiscoveryFilters                method    -         as                                    >
.RemoveDevice                       method    o         -                                     >
.SetDiscoveryFilter                 method    a{sv}     -                                     >
.StartDiscovery                     method    -         -                                     >
.StopDiscovery                      method    -         -                                     >
.Address                            property  s         "11:11:11:11:1C:11"                   >
.AddressType                        property  s         "public"                              >
.Alias                              property  s         "bravo"                               >
.Class
```

### Get a property

```
$ busctl get-property org.bluez /org/bluez/hci0 org.bluez.Adapter1 Address
s "11:11:11:11:1C:11"
```

### Call a method

```
busctl call org.bluez /org/bluez/hci0 org.bluez.Adapter1 StartDiscovery
```
