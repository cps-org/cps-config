{
    "name": "needs-components2",
    "cps_version": "0.13.0",
    "prefix": "/sentinel/",
    "requires": {
        "multiple-components": {
            "components": [
                "sample2"
            ]
        }
    },
    "components": {
        "default": {
            "type": "interface",
            "requires": [
                "multiple-components:sample2"
            ]
        }
    },
    "default_components": [
        "default"
    ]
}
