{
    "Name": "cps-path-not-set",
    "Cps-Version": "0.9.0",
    "Version": "1.0.0",
    "Components": {
        "default": {
            "Type": "archive",
            "Includes": {"C": ["@prefix@/err"]},
            "Location": "@prefix@/lib/libfoo.a"
        }
    },
    "Default-Components": ["default"]
}

