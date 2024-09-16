{
    "name": "minimal",
    "cps_version": "0.12.0",
    "version": "1.0.0",
    "components": {
        "sample0": {
            "type": "archive",
            "compile_flags": [
                "-fsentinal"
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
        "sample1": {
            "type": "archive",
            "compile_flags": [
                "-fopenmp"
            ],
            "includes": {
                "c": [
                    "/usr/local/include",
                    "/opt/include"
                ]
            },
            "definitions": {
                "c": {
                    "FOO": "1",
                    "BAR": "2",
                    "OTHER": null
                }
            },
            "location": "fake"
        }
    },
    "default_components": [
        "sample1"
    ]
}
