let path = require('path');
let Tray = require('./tray');

let item = (txt) => ({text:txt, callback: function(o){console.log(this, o, txt)}})

let tray = new Tray(path.join(__dirname,"ctray.ico"));

tray.menu = ["Tray Example",
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
                    tray.close();
                }}
            ];
    }}
];
let tray2 = new Tray(path.join(__dirname,"ctray-alert.ico"));

tray2.menu = [
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
                    tray2.close();
                }}
            ];
    }}
];


    for(let i=0; i < 5; i++){
        let c = i;
        let tray = new Tray(path.join(__dirname,"ctray.ico"))
        tray.menu = [
            { text: `Tray #${c}`, callback: () => console.log("Hello from tray", c) },
            { text: "Close", callback: () => tray.close() }
        ]
    }