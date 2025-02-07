{
    "name": "link-requires",
    "cps_version": "0.13.0",
    "version": "1.0.0",
    "requires": {
        "full": null,
        "multiple-components": null
    },
    "prefix": "/prefix",
    "components": {
        "default": {
            "type": "interface",
            "link_requires": [
                "full"
            ]
        },
        "nested": {
            "type": "interface",
            "link_requires": [
                "multiple-components:link-requires"
            ]
        }
    },
    "default_components": [
        "default"
    ]
}
