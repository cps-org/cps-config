{
    "Name": "minimal",
    "Cps-Version": "0.9.0",
    "Components": {
        "sample1": {
            "Type": "archive",
            "Compile-Flags": ["-fopenmp"],
            "Includes": {"C": ["/usr/local/include"]},
            "Defines": []
        },
        "sample2": {
            "Type": "archive",
            "Compile-Flags": ["-fopenmp"],
            "Includes": ["/opt/include"],
            "Defines": {"C": ["FOO=1"], "C++": ["!FOO"]}
        },
        "sample3": {
            "Type": "archive",
            "Includes": {"C": ["/something"]}
        }
    },
    "Default-Components": ["sample1", "sample2"]
}

