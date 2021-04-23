let path = require('path');
let Tray = require('./tray');

let item = (txt) => ({text:txt, callback: (o) => console.log(o, txt)})

let tray = new Tray(path.join(__dirname,"ctray.ico"), [
    "Tray Example",
    "-",
    item("Hello World!"),
    item("Hello Tray!"),
    item("Hello C++!"),
    {text: "More Hello's!", submenu: [
        item("Hello NodeJS!"),
        item("Hello VSCode!"),
        item("Hello xfce-terminal!")
    ]},
    {text: "Update", callback: () => {
        tray.menu = [
            "Good Bye!",
            "-",
            {text: "Quit", callback: () => {
                    tray.stop();
                }}
            ];
        tray.update();
    }}
]);

tray.start()
    .then(_ => console.log("Tray Closed"));
