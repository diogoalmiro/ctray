<p align="center">
    <img src="https://raw.githubusercontent.com/diogoalmiro/ctray/main/ctray.svg" alt="CTray Logo" height="90">
    <img src="https://raw.githubusercontent.com/diogoalmiro/ctray/main/ctray-alert.svg" alt="CTray Logo" height="90">
</p>

# CTray

Cross-platform Node.js class to create system trays.

**Note:** This package was not yet tested on MacOS. [(See issue #2)](https://github.com/diogoalmiro/ctray/issues/2)

## Installation

```$ npm install ctray```

## Usage

### Simple Example

```javascript
let Tray = require("ctray");

let tray = new Tray("path/to/icon", [
    {text: "Hello World!"},
    {text: "Quit", callback: _ => tray.stop()}
])

tray.start().then( () => console.log("Tray Closed") )
```

See [the example file](example.js) for a more complex example.

### Constructor `new Tray(icon: string, menu: MenuItem[]) : Tray`

Creates an instance of the tray.

The `icon` parameter is an absolute path to an `.ico` file.

The `menu` parameter is an array with at least one element. Each element of the array can either be a String or an Object with the following format:

```javascript
type MenuItem = {
    text:      string,     // Label of the element in the tray. Required
    checked?:  boolean,    // Item starts checked? defaults: false
    disabled?: boolean,    // Item is disabled? defaults: false
    callback?: () => void, // Function without arguments to run on click.
    submenu?:  MenuItem[], // Array With the same rules as menu
}
```
Internaly a String will be converted to the object text specified and the default arguments (without a callback).

When `text` is `"-"` the tray will create a separator in the tray. The other values of the item are ignored in this case.

### `tray#start() : Promise`

Shows the tray, the promise is fulfilled when the tray closes.

### `tray#update() : void`

Updates tray after changes on `tray.menu` or `tray.icon`. 

**Note:** It reacts to `tray.menu = ...`, **not updates inside the array** (e.g. `tray.menu[0].text = ...`)

### `tray#stop() : void`

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
