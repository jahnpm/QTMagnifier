# QTMagnifier
A Windows tool which magnifies the area around the mouse cursor written in C++ using the QT framework. It is meant for visually impaired people or really anyone who wants to magnify something on their screen in real time in a separate window.

## Installation
Either download the latest release or use your favorite method of compiling a QT 5 application.

## Usage
The application's window can be dragged anywhere on the screen and resized to an arbitrary size. Holding down the right mouse button while dragging or resizing will constrain position and size to the current screen (the one where the center of the application currently is). The size of the rectangular area that is magnified depends on the window size and the chosen zoom factor.

## Settings
Zoom factor and other settings are controlled by a settings.ini file, placed in the same directory as the executable. The following default settings.ini is created upon the first termination of the application:

```
[General]
ui_scale=1
zoom_factor=2
frame_width=5
window_x=0
window_y=0
window_width=600
window_height=200
refresh_interval_ms=17
```

| Setting                | Type  | Description                                                |
| ---------------------- | ----- | ---------------------------------------------------------- |
| ui_scale               | float | Adjust if not using 100% scale in Windows display settings |
| zoom_factor            | float | Determines by how much screen content is magnified         |
| frame_width            | float | The width of the border around the application window      |
| window_x               | int   | X-coordinate of the window position                        |
| window_y               | int   | Y-coordinate of the window position                        |
| window_width           | int   | Window width                                               |
| window_height          | int   | Window height                                              |
| refresh_interval_ms    | int   | Time in milliseconds between redrawings of window content  |

Application needs to be restarted for changes to take effect. Window geometry is saved to the ini file if changed during runtime.

## Known issues
- window will crash if resized to negative size
- window size could increase under some circumstances when resizing against a screen border while holding right click
- cursor is hard to see over white input fields, because the I-beam cursor is always white

## Immediate future plans
- address known issues
- make it possible to change and apply settings during runtime
- linux compatibility
