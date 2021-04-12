const {fork} = require("child_process");
const {join} = require("path");

module.exports = class Tray{
    constructor(icon, menu){
        this.icon = icon;
        this.menu = menu;
        this.cbs = [];
        this.cbs.stop = () => this.process.kill();
        this.i = 0;
        // create connection to child
        this.process = fork(join(__dirname, "tray-process.js"));
        this.process.on("message", msg => {
            let {type, action, error} = JSON.parse(msg);
            if( error ){
                console.error(`tray.${action}() -> ${error}`);
            }
            else{
                if(this.cbs[action]){
                    this.cbs[action]();
                }
            }
        })

        this.update();
    }
    
    start(){
        this.process.send(JSON.stringify({
            action: "start",
            arguments: []
        }))
    }

    stop(){
        this.process.send(JSON.stringify({
            action: "stop",
            arguments: []
        }))
    }

    update(){
        this.process.send(JSON.stringify({
            action: "update",
            arguments: [this.icon, JSON.stringify(this.menu, (key, value) => {
                if( key != "callback" ) return value;
                this.cbs[++this.i] = value;
                return this.i;
            })]
        }))  
    }
}

