{
    "name": "needs-components2",
    "cps_version": "0.12.0",
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
