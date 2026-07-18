import { describe, it, before } from 'node:test';
import assert from 'node:assert';
import { execFileSync } from 'child_process';
import { getBinaryPath } from '../../../npm/lib/platform.js';

describe('guchho CLI', () => {
  let binaryPath;

  before(() => {
    binaryPath = getBinaryPath();
  });

  it('prints version', () => {
    const out = execFileSync(binaryPath, ['version'], { encoding: 'utf-8' });
    assert.match(out, /guchho/);
  });

  it('prints usage with --help', () => {
    const out = execFileSync(binaryPath, ['--help'], { encoding: 'utf-8' });
    assert.match(out, /Usage/i);
  });

  it('returns non-zero for unknown command', () => {
    try {
      execFileSync(binaryPath, ['unknown-command'], { encoding: 'utf-8' });
      assert.fail('Should have thrown');
    } catch (err) {
      assert.notEqual(err.status, 0);
    }
  });
});
