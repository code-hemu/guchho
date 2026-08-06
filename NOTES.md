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



guchho/
│
├── CMakeLists.txt
├── README.md
├── LICENSE
├── .gitignore
├── guchho.config.json
│
├── cmake/
│
├── docs/
│
├── scripts/
│
├── tests/
│   ├── unit/
│   ├── integration/
│   ├── benchmark/
│   └── fixtures/
│
├── include/
│   └── guchho/
│       ├── api.hpp
│       ├── version.hpp
│       └── ...
│
├── src/
│   │
│   ├── main.cpp
│   │
│   ├── cli/
│   │
│   ├── config/
│   │
│   ├── core/
│   │
│   ├── fs/
│   │
│   ├── logger/
│   │
│   ├── watcher/
│   │
│   ├── resolver/
│   │
│   ├── graph/
│   │
│   ├── plugin/
│   │
│   ├── bundler/
│   │
│   ├── optimizer/
│   │
│   ├── sourcemap/
│   │
│   ├── printer/
│   │
│   ├── css/
│   │
│   ├── html/
│   │
│   ├── javascript/
│   │   │
│   │   ├── lexer/
│   │   ├── parser/
│   │   ├── ast/
│   │   ├── transformer/
│   │   ├── printer/
│   │   └── minifier/
│   │
│   ├── json/
│   │
│   ├── source_map/
│   │
│   └── utils/
│
├── third_party/
│
├── benchmark/
│
└── examples/


{
  "root": ".",

  "server": {
    "host": "localhost",
    "port": 3000,
    "open": true
  },

  "build": {
    "outDir": "dist",
    "target": "es2022",
    "format": "esm",
    "minify": true,
    "sourcemap": true,
    "treeShake": true,
    "codeSplit": true
  },

  "resolve": {
    "alias": {
      "@": "./src"
    }
  },

  "define": {
    "__DEV__": true
  }
}
CLI
guchho dev
guchho build
guchho preview



Configuration precedence
CLI flags
    ↓
guchho.config.json
    ↓
tsconfig.json (TypeScript options only)
    ↓
Built-in defaults



                               Guchho
                     Fast. Native. Zero Dependencies.

                                   │
                            CLI / Node API
                                   │
                 ┌─────────────────┴─────────────────┐
                 │                                   │
            Development                        Production
                 │                                   │
         Dev Server / HMR                    Build Pipeline
                 │                                   │
                 └─────────────────┬─────────────────┘
                                   │
                                   ▼
                             Configuration
                                   │
                                   ▼
                            Plugin Manager
                                   │
                                   ▼
                               File Scanner
                                   │
                                   ▼
                           Module Resolver
                                   │
                                   ▼
                           Dependency Graph
                                   │
                                   ▼
                             Source Loader
                                   │
                                   ▼
          ┌────────────────────────┼────────────────────────┐
          │                        │                        │
          ▼                        ▼                        ▼
      JS/TS Parser            CSS Parser            HTML Parser
          │                        │                        │
          └────────────────────────┼────────────────────────┘
                                   │
                                   ▼
                                  AST
                                   │
                                   ▼
                            AST Transform Passes
                                   │
             ┌─────────────────────┼─────────────────────┐
             │                     │                     │
             ▼                     ▼                     ▼
      JSX/TS Transform      Constant Folding      Dead Code Removal
             │                     │                     │
             └─────────────────────┼─────────────────────┘
                                   │
                                   ▼
                              Tree Shaking
                                   │
                                   ▼
                             Code Splitting
                                   │
                                   ▼
                             Chunk Generator
                                   │
                                   ▼
                          Asset Processing
                    (CSS, Images, Fonts, JSON)
                                   │
                                   ▼
                               Minification
                                   │
                                   ▼
                            Source Map Writer
                                   │
                                   ▼
                             Output Generator
                                   │
                                   ▼
                                 dist/