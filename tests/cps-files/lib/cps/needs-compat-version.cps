{
    "name": "needs-components2",
    "cps_version": "0.12.0",
    "requires": {
        "has-compat-version": {
            "components": ["default"],
            "version": "1.0.0"
        }
    },
    "components": {
        "default": {
            "type": "interface",
            "requires": ["has-compat-version"]
        }
    },
    "default_components": [
        "default"
    ]
}
