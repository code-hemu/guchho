import { execFile } from 'child_process';
import { getBinaryPath, binaryName } from './platform.js';

function runBinary(command, options = {}) {
  return new Promise((resolve, reject) => {
    const binaryPath = getBinaryPath();
    const input = JSON.stringify({ command, ...options });

    const child = execFile(
      binaryPath,
      ['--json'],
      { maxBuffer: 256 * 1024 * 1024 },
      (err, stdout, stderr) => {
        if (err) {
          reject(new Error(stderr.toString() || err.message));
          return;
        }
        try {
          resolve(JSON.parse(stdout.toString()));
        } catch {
          reject(new Error('Failed to parse binary output'));
        }
      }
    );

    child.stdin.write(input);
    child.stdin.end();
  });
}

function runBinaryStream(command, options = {}) {
  const binaryPath = getBinaryPath();
  const input = JSON.stringify({ command, ...options });

  const child = execFile(binaryPath, ['--json'], {
    maxBuffer: 256 * 1024 * 1024,
  });

  child.stdin.write(input);
  child.stdin.end();

  return child;
}

export async function build(options = {}) {
  return runBinary('build', options);
}

export async function dev(options = {}) {
  return runBinary('dev', options);
}

export async function preview(options = {}) {
  return runBinary('preview', options);
}

export async function transform(code, options = {}) {
  return runBinary('transform', { code, ...options });
}

export function buildStream(options = {}) {
  return runBinaryStream('build', options);
}

export function devStream(options = {}) {
  return runBinaryStream('dev', options);
}

export { getBinaryPath, binaryName };
