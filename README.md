# centripedal

This is a tiny little daemon for macOS that coalesces keyboard modifier events (Ctrl, Alt, Cmd) between different keyboards (or other human interface devices like foot pedals) that are plugged into the same machine. By default, macOS keeps keyboard states independently of each other, so that pressing F on keyboard A while holding down CTRL on keyboard B is recognized only as an F. Though this behavior is sensible for an OS default, it interferes with workflows that use a dedicated input device for modifier events.

You'll need to grant this app accessibility privileges (in System Preferences.app) or run it as root.

## How it works

This uses the Quartz `CGEventTap` API to install a listener that listens for modifier-key events and adds them to other key-down events if necessary. Its state resets 10 seconds after the last modifier key is pressed, just to ensure that the system never gets in an unusable state. (In the event that it does, logging out and in will cure it.)

## To build

Run `xcodebuild`.

GPL3
