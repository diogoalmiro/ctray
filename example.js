let path = require('path');
let Tray = require('./tray');

// Easier way to create items with a text that output that text on click
let item = (txt) => ({text:txt, callback: _ => console.log(txt)});

// Create first tray
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
        // On click update menu to be able to close tray
        tray.menu = [
            "Good Bye!",
            "-",
            {text: "Quit", callback: () => tray.close()}
        ];
    }}
];

tray.on("close", () => console.log("First Tray Closed!"));

let i = tray.menu[0];
i.text = "Overwrite text";
i.on("click", () => {
    console.log("Add more callbacks using the EventEmitter of this item");
    i.checked = !i.checked;
})

// Multiple trays at the same time
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
            {text: "Quit", callback: () => tray2.close()}
        ];
    }}
];


// Event more trays
for(let i=0; i < 5; i++){
    let c = i;
    let tray = new Tray(path.join(__dirname,"ctray.ico"))
    tray.menu = [
        { text: `Tray #${c}`, callback: () => console.log("Hello from tray", c) },
        { text: "Close", callback: () => tray.close() }
    ]
}