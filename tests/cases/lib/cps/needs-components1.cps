{
    "Name": "needs-components1",
    "Cps-Version": "0.9.0",
    "Requires": {
        "multiple-components": {
            "components": ["sample3"]
        }
    },
    "Components": {
        "default": {
            "Type": "interface",
            "Requires": ["multiple-components:sample3"]
        }
    },
    "Default-Components": ["default"]
}


