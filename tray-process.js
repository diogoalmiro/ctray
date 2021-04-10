let singleTray = require('bindings')('tray');

process.once("message", json => {
    

    
    let {type, name, menu} = JSON.parse(json, (name, value) => {
        if( name != "callback" ) return value;

        return () => process.send(JSON.stringify({type:"click", index: value}));
    });

    if( type == "setup"){
        singleTray.setup(name, menu).then( _ => console.log("ended") ).catch( e => console.log(e) );
    }
});

