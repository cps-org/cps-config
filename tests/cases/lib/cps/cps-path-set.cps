{
    "Name": "cps-path-set",
    "Cps-Version": "0.9.0",
    "Cps-Path": "/sentinel/lib/cps/",
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

