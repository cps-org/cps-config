{
    "name": "needs-components1",
    "cps_version": "0.10.0",
    "requires": {
        "multiple-components": {
            "components": [
                "sample3"
            ]
        }
    },
    "components": {
        "default": {
            "type": "interface",
            "requires": [
                "multiple-components:sample3"
            ]
        }
    },
    "default_components": [
        "default"
    ]
}
