Directory Structure
tests/
│
├── CMakeLists.txt
│
├── unit/
│   ├── CMakeLists.txt
│   ├── lexer_test.cpp
│   └── parser_test.cpp
│
├── integration/
│   ├── CMakeLists.txt
│   └── bundler_test.cpp
│
├── regression/
│   ├── CMakeLists.txt
│   └── issue_001.cpp
│
└── benchmark/
    ├── CMakeLists.txt
    └── parser_benchmark.cpp

CLI STYLE
guchho build
guchho dev
guchho test
guchho fmt
guchho lint
guchho publish
guchho doctor
guchho init
guchho create
guchho upgrade 