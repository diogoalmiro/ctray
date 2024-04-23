<p align="center">
    <img src="https://raw.githubusercontent.com/diogoalmiro/ctray/main/ctray.svg" alt="CTray Logo" height="90">
    <img src="https://raw.githubusercontent.com/diogoalmiro/ctray/main/ctray-alert.svg" alt="CTray Logo" height="90">
</p>

# CTray

Cross-platform Node.js class to create system trays.

**Note:** This package does not support macOS. [(See issue #2)](https://github.com/diogoalmiro/ctray/issues/2)

## Installation

```$ npm install ctray```

## Usage

### Simple Example

```javascript
let Tray = require("ctray");

let tray = new Tray("path/to/icon");

tray.tooltip = "Simple Example Tray"; // Sets the tooltip to display

tray.menu = [ // Sets the tray menu
    "Example",
    "-",
    {text: "Quit", callback: _ => tray.close()}
]

tray.on("close", () => console.log("Tray Closed"));
```

See [the example file](example.js) for a more complex example.

### Constructor `new Tray(icon: string) : Tray`

Creates the tray's instance displays the icon on the taskbar.
Tray extends from EventEmitter, and it emits "close" once it is closed.

### `tray#close(): void`

Requests the tray to close and emits "close".

### `tray#tooltip: string`

Getter/Setter for the tooltip to show over the tray.

### `tray#icon: string`

Getter/Setter for the absolute path to the icon to display.

### `tray#menu: MenuItem[]`

Setter for the menu of the tray. Each element of the array can either be a String or an Object with the following format:

```javascript
type MenuItem = {
    text:      string,     // Label of the element in the tray. Required
    checked?:  boolean,    // Item starts checked? defaults: false
    disabled?: boolean,    // Item is disabled? defaults: false
    callback?: () => void, // Function without arguments to run on click.
    submenu?:  MenuItem[], // Array With the same rules as menu
}
```

Note that getting this property will return the menu Array as an Object to prevent Array functions that add/remove elements as they are not supported.

Setting `text = '-'` creates a MenuItem of type "separator". Its text cannot be changed nor have submenus.

## Next steps

 - [ ] Improve documentation.
 - [x] ~~Test in other plattforms.~~
 - [ ] Create package.json Scripts.
 - [x] ~~Allow changing the menu dynamically.~~
 - [ ] Test in MacOS

###### Notes to self

 - Generate ico from svg command:
   `$ convert -density 2048 -background transparent icon.svg -define icon:auto-resize -colors 256 icon.ico`
