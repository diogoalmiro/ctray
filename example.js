let path = require('path');
let Tray = require('./tray');

let item = (txt) => ({text:txt, callback: () => console.log(txt)})

let tray = new Tray(path.join(__dirname,"ctray.ico"), [
    item("Hello World!"),
    item("Hello Tray!"),
    item("Hello C++!"),
    {text: "More Hello's!", submenu: [
        item("Hello NodeJS!"),
        item("Hello VSCode!"),
        item("Hello xfce-terminal!"),
        {text: "Goodbye!", submenu: [
            {text: "Quit", callback: () => tray.stop()}
        ]}
    ]}
]);

tray.start();
