# CTray

Cross-platform (tested on linux only for now) Node.js class to create system trays.

## Installation

```$ npm install ctray```

## Example Usage

```
let Tray = require("ctray");

let tray = new Tray("path/to/icon", [
    {text: "Hello World!"},
    {text: "Quit", callback: _ => tray.stop()}
])
```

See [the example file](example.js) for a more complex example.

## Next steps

 - [ ] Improve documentation.
 - [ ] Test in other plattforms.
 - [ ] Create package.json Scripts.
 - [ ] Allow changing the menu dynamically.
