const {fork} = require("child_process");
const EventEmiter = require("events").EventEmitter;

module.exports = class Tray extends EventEmiter{
    constructor(name, menu){
        super();
        this.name = name;
        this.menu = menu;
        this.started = false;
    }
    
    start(){
        if( this.started ) return;
        this.started = true;
        this.process = fork("tray-process.js");
        this.process.on("message", msg => {
            let {type, index} = JSON.parse(msg);
            if( type=='click' && this.cbs[index] ){
                this.cbs[index]();
            }
        })
        this.process.on("error", e => {
            console.error(e)
            this.stop();
        })

        this.process.on("exit", e => {
            this.stop();
        })
        this.cbs = [];
        let i = -1;
        this.process.send(JSON.stringify({type:"setup", name: this.name, menu: this.menu}, (key, value) => {
            if( key != "callback" ) return value;
            this.cbs[++i] = value;
            return i;
        }));
        return 
    }

    stop(){
        if( !this.started ) return;
        this.started = false;
        this.process.kill();
    }
}

