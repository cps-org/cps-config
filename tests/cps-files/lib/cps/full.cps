{
    "name": "full",
    "cps_version": "0.10.0",
    "version": "1.2.1",
    "compat_version": "1.0.0",
    "components": {
        "sample0": {
            "type": "archive",
            "compile_flags": [
                "-fvectorize"
            ],
            "includes": {
                "c": [
                    "/err"
                ]
            },
            "definitions": {
                "c": [
                    "-DFAIL"
                ]
            },
            "location": "fake"
        },
        "fullSample": {
            "type": "dylib",
            "compile_flags": [
                "-fvectorize"
            ],
            "includes": {
                "c": [
                    "/usr/local/include",
                    "/opt/include"
                ]
            },
            "link_flags": ["-L/usr/lib/", "-lbar", "-flto"],
            "definitions": {
                "c": [
                    "FOO=1",
                    "BAR=2",
                    "!BAR",
                    "OTHER"
                ],
                "c++": [
                    "!FOO"
                ]
            },
            "location": "/something/lib/libfoo.so.1.2.0",
            "link_location": "/something/lib/libfoo.so"
        }
    },
    "default_components": [
        "fullSample"
    ]
}
