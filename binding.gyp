{
    "targets": [
        {
            "target_name": "tray",
            "cflags": ["-g -Wall -Wextra -pedantic"], # tray/Makefile:13 
            "ldflags": ["-g"],
            "cflags!": [ "-fno-exceptions" ],
            "cflags_cc!": [ "-fno-exceptions" ],
            "sources": [ "addon.cc" ],
            "include_dirs": [
                "<!@(node -p \"require('node-addon-api').include\")"
            ],
            "dependencies": [
                "<!@(node -p \"require('node-addon-api').gyp\")"
            ],
            'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
            'conditions': [ # Initials checks on tray/Makefile
                ["OS=='linux'", {
                    "cflags+": ["-DTRAY_APPINDICATOR=1", "<!@(pkg-config --cflags appindicator3-0.1)"],
                    "ldflags+": ["<!@(pkg-config --libs appindicator3-0.1)"],
                }]
            ]
        }]
}