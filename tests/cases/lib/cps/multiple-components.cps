{
    "name": "multiple-components",
    "cps_version": "0.10.0",
    "requires": {
        "minimal": {}
    },
    "components": {
        "sample1": {
            "type": "archive",
            "compile_flags": [
                "-fopenmp"
            ],
            "includes": {
                "c": [
                    "/usr/local/include"
                ]
            },
            "defines": [],
            "location": "fake"
        },
        "sample2": {
            "type": "archive",
            "compile_flags": [
                "-fopenmp"
            ],
            "includes": [
                "/opt/include"
            ],
            "defines": {
                "c": [
                    "FOO=1"
                ],
                "c++": [
                    "!FOO"
                ]
            },
            "location": "/something/lib/libfoo.so.1.2.0"
        },
        "sample3": {
            "type": "archive",
            "includes": {
                "c": [
                    "/something"
                ]
            },
            "link_libraries": [
                "dl",
                "rt"
            ],
            "location": "/something/lib/libfoo.so.1.2.0",
            "link_location": "/something/lib/libfoo.so"
        },
        "sample4": {
            "type": "interface",
            "requires": [
                ":sample3"
            ]
        },
        "requires-external": {
            "type": "interface",
            "requires": [
                "minimal:sample0"
            ]
        }
    },
    "default_components": [
        "sample1",
        "sample2"
    ]
}
