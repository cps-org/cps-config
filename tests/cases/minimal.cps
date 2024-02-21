{
    "Name": "minimal",
    "Cps-Version": "0.9.0",
    "Components": {
        "sample0": {
            "Type": "archive",
            "Compile-Flags": ["-fsentinal"],
            "Includes": {"C": ["/err"]},
            "Defines": {"C": ["-DFAIL"]},
            "Location": "fake"
        },
        "sample1": {
            "Type": "archive",
            "Compile-Flags": ["-fopenmp"],
            "Includes": {"C": ["/usr/local/include", "/opt/include"]},
            "Defines": {"C": ["FOO=1", "BAR=2", "!BAR", "OTHER"], "C++": ["!FOO"]},
            "Location": "fake"
        }
    },
    "Default-Components": ["sample1"]
}
