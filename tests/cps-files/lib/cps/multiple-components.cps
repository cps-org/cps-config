{
    "name": "multiple-components",
    "cps_version": "0.13.0",
    "prefix": "/sentinel/",
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
            "definitions": {},
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
            "definitions": {
                "c": {
                    "FOO": "1"
                }
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
        "link-flags": {
            "type": "dylib",
            "link_flags": [
                "-L/usr/lib/",
                "-lbar",
                "-flto"
            ],
            "location": "/something/lib/libfoo.so"
        },
        "requires-external": {
            "type": "interface",
            "requires": [
                "minimal:sample0"
            ]
        },
        "link-requires": {
            "type": "interface",
            "link_requires": [
                "minimal:sample1"
            ]
        },
        "same-component-twice": {
            "type": "interface",
            "requires": [
                ":sample3",
                ":sample4"
            ]
        }
    },
    "default_components": [
        "sample1",
        "sample2"
    ]
}
