let path = require('path');
let Tray = require('bindings')('tray');

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
    .then(_ => console.log("Closed Tray without any error"));


    let tray2 = new Tray(path.join(__dirname,"ctray.ico"), [
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
            tray2.menu = [
                "Good Bye!",
                "-",
                {text: "Quit", callback: () => {
                        tray.stop();
                    }}
                ];
            tray.update();
        }}
    ]);
    
    tray2.start()
        .then(_ => console.log("Closed Tray without any error"));