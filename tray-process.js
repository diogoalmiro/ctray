let singleTray = require('bindings')('tray');

process.on("message", json => {

    let {action, arguments} = JSON.parse(json);
    if( action == "update" ){
        arguments[1] = JSON.parse(arguments[1], (name, value) => {
            if( name != "callback" ) return value;
            
            return () => process.send(JSON.stringify({type:"callback", action: value}));
        })
    }
    
    if( action in singleTray ){
        try{
            let promise = singleTray[action].apply(singleTray, arguments);

            
            if( !promise ){
                process.send(JSON.stringify({type: "callback", action}))
            }
            else{
                promise.then(_ => {
                    process.send(JSON.stringify({type: "callback", action}))
                }).catch(e => {
                    process.send(JSON.stringify({type: "callback", action, error: e.message}))
                })
            }
        }
        catch(e){
            process.send(JSON.stringify({type:"callback", action, error: e.message}))
        }
    }
    else{
        process.send(JSON.stringify({type:"callback", action, error: `Not found.`}))
    }
});

