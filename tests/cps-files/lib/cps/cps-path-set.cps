{
    "name": "cps-path-set",
    "cps_version": "0.10.0",
    "cps_path": "/sentinel/lib/cps/",
    "version": "1.0.0",
    "components": {
        "default": {
            "type": "archive",
            "includes": {
                "c": [
                    "@prefix@/err"
                ]
            },
            "location": "@prefix@/lib/libfoo.a"
        }
    },
    "default_components": [
        "default"
    ]
}
