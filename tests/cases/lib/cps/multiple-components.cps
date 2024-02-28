{
    "Name": "multiple-components",
    "Cps-Version": "0.9.0",
    "Requires": {
        "minimal": {}
    },
    "Components": {
        "sample1": {
            "Type": "archive",
            "Compile-Flags": ["-fopenmp"],
            "Includes": {"C": ["/usr/local/include"]},
            "Defines": [],
            "Location": "fake"
        },
        "sample2": {
            "Type": "archive",
            "Compile-Flags": ["-fopenmp"],
            "Includes": ["/opt/include"],
            "Defines": {"C": ["FOO=1"], "C++": ["!FOO"]},
            "Location": "/something/lib/libfoo.so.1.2.0"
        },
        "sample3": {
            "Type": "archive",
            "Includes": {"C": ["/something"]},
            "Link-Libraries": ["dl", "rt"],
            "Location": "/something/lib/libfoo.so.1.2.0",
            "Link-Location": "/something/lib/libfoo.so"
        },
        "sample4": {
            "Type": "interface",
            "Requires": [":sample3"]
        },
        "requires-external": {
            "Type": "interface",
            "Requires": ["minimal:sample0"]
        }
    },
    "Default-Components": ["sample1", "sample2"]
}

