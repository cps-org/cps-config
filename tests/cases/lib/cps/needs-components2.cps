{
    "Name": "needs-components2",
    "Cps-Version": "0.9.0",
    "Requires": {
        "multiple-components": {
            "components": ["sample2"]
        }
    },
    "Components": {
        "default": {
            "Type": "interface",
            "Requires": ["multiple-components:sample2"]
        }
    },
    "Default-Components": ["default"]
}


