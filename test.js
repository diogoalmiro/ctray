let path = require('path');
let Tray = require('./tray');

let item = (txt) => ({text:txt, callback: () => console.log(txt)})

let icons = ["ctray.ico", "ctray-alert.ico"]
let i = 0;

let otherTray = null;
let simpleTray = new Tray(path.join(__dirname,"ctray.ico"), [
    "Simple Tray Example",
    "-",
    { text: "Test Callback", callback: () => console.log("Working") },
    { text: "Alert", callback: () => {
        let icon = icons[((++i) % icons.length)];
        simpleTray.icon = path.join(__dirname,icon);
        simpleTray.update();
    } },
    { text: "Toogle Other Tray", callback: () => {
        if( !otherTray ){
            otherTray = new Tray(path.join(__dirname,"ctray.ico"), [
                "Other Tray Says Hello",
                {text: "Update", callback: () => {
                    otherTray.menu[0] = "Other Tray Says It Updated";
                    otherTray.menu.push({
                        text: "Submenu",
                        submenu : ["Hello From submenu!"]
                    })
                    otherTray.menu = otherTray.menu;
                    otherTray.update();
                }},
                {text: "Quit", callback: () => {
                    otherTray.stop();
                    otherTray = null;
                }}
            ])
            otherTray.start();
        }
        else{
            otherTray.stop();
            otherTray = null;
        }
    } },
    { text: "Quit", callback: () => {
        simpleTray.stop();
        if( otherTray ){
            otherTray.stop();
        }
    } }
]);

simpleTray.start()
    .then(_ => console.log("Tray Closed"));
