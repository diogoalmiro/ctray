<p align="center">
    <img src="ctray.svg" alt="CTray Logo" height="90">
    <img src="ctray-alert.svg" alt="CTray Logo" height="90">
</p>

# CTray

Cross-platform Node.js class to create system trays.

**Note:** This package was not yet tested on MacOS [Se issue #2](https://github.com/diogoalmiro/ctray/issues/2)

## Installation

```$ npm install ctray```

## Usage

### Simple Example

```
let Tray = require("ctray");

let tray = new Tray("path/to/icon", [
    {text: "Hello World!"},
    {text: "Quit", callback: _ => tray.stop()}
])

tray.start().then( () => console.log("Tray Closed") )
```

See [the example file](example.js) for a more complex example.

### Constructor `new Tray(icon: String, menu: Array<String|Object>) : Tray`

Creates an instance of the tray.

The `icon` parameter is an absolute path to an `.ico` file.

The `menu` parameter is an array with at least one element. Each element of the array can either be a String or an Object with the following format:

```
{
    text: String,                    // Label of the element in the tray. Required
    [checked: Boolean,]              // Item starts checked? defaults: false
    [disabled: Boolean,]             // Item is disabled? defaults: false
    [callback: Function,]            // Function without arguments to run on click.
    [submenu: Array<String|Object>,] // Array With the same rules as menu
}
```
Internaly a String will be converted to the object text specified and the default arguments (without a callback).

When `text` is `"-"` the tray will create an separator in the tray. The other values of the item are ignored in this case.

### `tray#start() : Promise`

Shows the tray, promise ends when the tray is closed.

### `tray#update() : undefined`

Updates tray after changes on `tray.menu` or `tray.icon`. 

**Note:** It reacts to `tray.menu = ...`, **not updates inside the array** (e.g. `tray.menu[0].text = ...`)

### `tray#stop() : undefined`

Stops the tray.

## Next steps

 - [ ] Improve documentation.
 - [x] ~~Test in other plattforms.~~
 - [ ] Create package.json Scripts.
 - [x] ~~Allow changing the menu dynamically.~~
 - [ ] Test in MacOS

###### Notes to self

 - Generate ico from svg command:
   `$ convert -density 2048 -background transparent icon.svg -define icon:auto-resize -colors 256 icon.ico`
