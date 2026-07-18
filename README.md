<p align="center">
  <a href="https://github.com/code-hemu/guchho">
    <picture>
      <source media="(prefers-color-scheme: dark)" srcset="https://raw.githubusercontent.com/code-hemu/guchho/refs/heads/main/resources/light_logo.png" />
      <source media="(prefers-color-scheme: light)" srcset="https://raw.githubusercontent.com/code-hemu/guchho/refs/heads/main/resources/dark_logo.png" />
      <img src="https://raw.githubusercontent.com/code-hemu/guchho/refs/heads/main/resources/dark_logo.png" alt="guchho logo" width="500px" />
    </picture>
  </a>
</p>

<p align="center">
  <a href="https://www.npmjs.com/package/guchho"><img src="https://img.shields.io/npm/v/guchho" alt="Version"></a>
  <a href="https://github.com/code-hemu/guchho/blob/master/LICENSE"><img src="https://img.shields.io/github/license/code-hemu/guchho" alt="License"></a>
  <a href="https://github.com/code-hemu/guchho/issues"><img src="https://img.shields.io/github/issues/code-hemu/guchho" alt="Issues"></a>
  <a href="https://github.com/code-hemu/guchho/graphs/contributors"><img src="https://img.shields.io/github/contributors/code-hemu/guchho" alt="Contributors"></a>
  <a href="https://github.com/sponsors/code-hemu"><img src="https://img.shields.io/badge/Sponsor-GitHub-red" alt="Sponsor"></a>
</p>

## Features

- **Multi-language bundling** — JavaScript, TypeScript, JSX, and TSX parsing and bundling
- **CSS & HTML** — tokenization and parsing support
- **Multiple output formats** — ESM (ES Modules), CJS (CommonJS), and IIFE
- **Code optimization** — built-in minifier and tree-shaking
- **Source maps** — inline and external source map generation
- **Plugin system** — lifecycle hooks for `on_resolve`, `on_load`, and `on_transform`
- **Dev server** — built-in development server with file watching
- **CLI & programmatic API** — use via command line or as a JavaScript library
- **Fast native performance** — core written in C++20 with zero runtime npm dependencies

## Quick Start

```bash
npm install guchho
```

Use the CLI to build your project:

```bash
guchho build --entry src/index.js --out-dir dist --format esm
```

Start the dev server with file watching:

```bash
guchho dev --entry src/index.js --out-dir dist
```

Transform a single file:

```bash
guchho transform src/component.jsx
```

## Examples

### Basic build

```js
import { build } from 'guchho'

await build({
  entryPoints: ['src/index.js'],
  outDir: 'dist',
  format: 'esm',
  minify: true,
  sourceMap: true,
})
```

### With React/Vue

```js
import { build } from 'guchho'

await build({
  entryPoints: ['src/main.jsx'],
  outDir: 'dist',
  format: 'esm',
  loader: { '.jsx': 'jsx' },
})
```

## API Reference

### `build(options)`

| Option         | Type       | Default    | Description                          |
|----------------|------------|------------|--------------------------------------|
| `entryPoints`  | `string[]` | —          | Entry module paths                   |
| `outDir`       | `string`   | `dist`     | Output directory                     |
| `format`       | `string`   | `esm`      | Output format (`esm`, `cjs`, `iife`) |
| `minify`       | `boolean`  | `false`    | Minify output                        |
| `sourceMap`    | `boolean`  | `false`    | Emit source maps                     |
| `loader`       | `object`   | `{}`       | File extension loaders               |
| `plugins`      | `array`    | `[]`       | Custom plugins                       |
| `watch`        | `boolean`  | `false`    | Watch mode                           |

### `dev(options)`

Starts a development server with file watching. Accepts the same options as `build()` plus:

| Option    | Type     | Default | Description        |
|-----------|----------|---------|--------------------|
| `port`    | `number` | `3000`  | Dev server port    |
| `host`    | `string` | `localhost` | Server host    |

### `transform(code, options)`

Transform a single code string. Returns the transformed code and source map.

### `preview(options)`

Start a static preview server for a built output directory.

## Configuration

Create a `guchho.json` file in your project root:

```json
{
  "entryPoints": ["src/index.js"],
  "outDir": "dist",
  "format": "esm",
  "minify": true,
  "sourceMap": true,
  "loader": {
    ".jsx": "jsx",
    ".ts": "ts"
  },
  "plugins": []
}
```

Supported fields: `entryPoints`, `outDir`, `format`, `minify`, `sourceMap`, `loader`, `plugins`, `watch`, `banner`, `footer`, `define`, `external`, `alias`.

## Architecture

guchho has a two-layer design:

- **C++ core** — a native binary that handles parsing (JS/TS/CSS/HTML), dependency graph construction, module resolution, code transformation, optimization (minifier, tree-shaking), and bundle emission (ESM/CJS/IIFE with source maps).
- **Node.js wrapper** — provides the CLI entry point and programmatic API. Handles platform detection, binary resolution, and process management. Zero runtime dependencies.

```
npm/                  Node.js wrapper (CLI + JS API)
include/              C++ headers
src/                  C++ implementation
tests/                C++ unit tests (Google Test) + JS integration tests
```

## Contributing

### Prerequisites

- CMake >= 3.20
- Visual Studio 17 2022 (Windows)
- Node.js >= 18

### Build from source

```bash
# Configure and build
cmake --preset release
cmake --build --preset release

# Or use the build script
./scripts/build.sh
```

### Run tests

```bash
# JavaScript integration tests
npm test

# All tests (JS + C++ unit tests)
npm run test:all
```

### Code style

This project uses Google style with clang-format and clang-tidy:

```bash
./scripts/format.sh   # Format code
./scripts/lint.sh     # Lint code
```

### Project structure

| Directory | Contents |
|-----------|----------|
| `npm/` | Node.js wrapper, CLI, JS API, platform detection |
| `include/` | C++ headers (parser, graph, optimizer, emitter, dev, plugin) |
| `src/` | C++ implementation |
| `tests/` | C++ unit tests and JS integration tests with fixtures |
| `scripts/` | Build, test, format, and lint scripts |
