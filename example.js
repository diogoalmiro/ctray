let path = require('path');
let Tray = require('./tray');

let item = (txt) => ({text:txt, callback: () => console.log(txt)})

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


let tray2 = new Tray(path.join(__dirname,"ctray-alert.ico"), [
    "Tray2 Example",
    "-",
    item("Hello World!"),
    item("Hello Tray2!"),
    item("Hello C++!"),
    {text: "More Hello's!", submenu: [
        item("Hello NodeJS!"),
        item("Hello VSCode!"),
        item("Hello xfce-terminal!")
    ]},
    {text: "Update", callback: () => {
        tray2.menu = [
            "Good Bye!",
            "-",
            {text: "Quit", callback: () => {
                    tray2.stop();
                }}
            ];
        tray2.update();
    }}
]);

tray2.start()
    .then(_ => console.log("Tray2 Closed"));


    for(let i=0; i < 5; i++){
        let c = i;
        let tray = new Tray(path.join(__dirname,"ctray.ico"), [
            { text: `Tray #${c}`, callback: () => console.log("Hello from tray", c) },
            { text: "Close", callback: () => tray.stop() }
        ])
        tray.start().then(_ => console.log(`Tray ${c} closed.`)).catch(e => console.log(e))
    }
    