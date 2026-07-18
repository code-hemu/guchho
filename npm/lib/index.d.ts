import { ChildProcess } from 'child_process';

export interface BuildOptions {
  entryPoints?: string[];
  outDir?: string;
  outFile?: string;
  minify?: boolean;
  sourceMaps?: boolean;
  treeShaking?: boolean;
  format?: 'esm' | 'cjs' | 'iife';
  external?: string[];
  aliases?: Record<string, string>;
  plugins?: string[];
}

export interface DevOptions {
  entryPoints?: string[];
  outDir?: string;
  port?: number;
  host?: string;
}

export interface PreviewOptions {
  outDir?: string;
  port?: number;
  host?: string;
}

export interface TransformOptions {
  format?: 'esm' | 'cjs' | 'iife';
  sourceMap?: boolean;
  minify?: boolean;
  loader?: 'js' | 'ts' | 'jsx' | 'tsx' | 'css' | 'json';
}

export interface BuildResult {
  errors: string[];
  warnings: string[];
  outputFiles: { path: string; contents: string }[];
}

export interface DevResult {
  port: number;
  host: string;
}

export interface PreviewResult {
  port: number;
  host: string;
}

export interface TransformResult {
  code: string;
  map?: string;
  errors: string[];
  warnings: string[];
}

export function build(options?: BuildOptions): Promise<BuildResult>;
export function dev(options?: DevOptions): Promise<DevResult>;
export function preview(options?: PreviewOptions): Promise<PreviewResult>;
export function transform(code: string, options?: TransformOptions): Promise<TransformResult>;
export function buildStream(options?: BuildOptions): ChildProcess;
export function devStream(options?: DevOptions): ChildProcess;
export function getBinaryPath(): string;
export const binaryName: string;
