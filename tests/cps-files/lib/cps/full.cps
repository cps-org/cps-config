{
    "name": "full",
    "cps_version": "0.13.0",
    "version": "1.2.1",
    "compat_version": "1.0.0",
    "prefix": "/sentinel/",
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
                "c": {
                    "FAIL": null
                }
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
            "link_flags": [
                "-L/usr/lib/",
                "-lbar",
                "-flto"
            ],
            "definitions": {
                "c": {
                    "FOO": "1",
                    "BAR": "2",
                    "OTHER": null
                }
            },
            "location": "/something/lib/libfoo.so.1.2.0",
            "link_location": "/something/lib/libfoo.so"
        },
        "star_values": {
            "type": "dylib",
            "compile_flags": {
                "*": [
                    "-fvectorize"
                ]
            },
            "includes": {
                "*": [
                    "/usr/local/include",
                    "/opt/include"
                ]
            },
            "link_flags": [
                "-L/usr/lib/",
                "-lbar",
                "-flto"
            ],
            "definitions": {
                "*": {
                    "FOO": "1",
                    "BAR": "2",
                    "OTHER": null
                }
            },
            "location": "/something/lib/libfoo.so.1.2.0",
            "link_location": "/something/lib/libfoo.so"
        },
        "star_values_override": {
            "type": "dylib",
            "compile_flags": {
                "c": [
                    "-fvectorize"
                ],
                "*": [
                    "-bad-value"
                ]
            },
            "includes": {
                "c": [
                    "/usr/local/include",
                    "/opt/include"
                ],
                "*": [
                    "/dev/null"
                ]
            },
            "link_flags": [
                "-L/usr/lib/",
                "-lbar",
                "-flto"
            ],
            "definitions": {
                "c": {
                    "FOO": "1",
                    "BAR": "2",
                    "OTHER": null
                },
                "*": {
                    "BAD": "value"
                }
            },
            "location": "/something/lib/libfoo.so.1.2.0",
            "link_location": "/something/lib/libfoo.so"
        }
    },
    "default_components": [
        "fullSample"
    ]
}
