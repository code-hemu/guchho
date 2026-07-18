#!/usr/bin/env node

import { execFileSync } from 'child_process';
import { getBinaryPath, binaryName } from '../lib/platform.js';

try {
  const binaryPath = getBinaryPath();
  const args = process.argv.slice(2);
  const result = execFileSync(binaryPath, args, {
    stdio: 'inherit',
  });
  process.exit(result.status ?? 0);
} catch (err) {
  if (err.status) {
    process.exit(err.status);
  }
  console.error(`guchho: ${err.message}`);
  process.exit(1);
}
