import { describe, it } from 'node:test';
import assert from 'node:assert';
import { transform } from '../../../npm/lib/index.js';

describe('guchho.transform()', () => {
  it('returns a result with expected fields', async () => {
    const result = await transform('const x = 1;', { format: 'esm' });
    assert.ok(result);
    assert.equal(typeof result.code, 'string');
    assert.ok(Array.isArray(result.errors));
    assert.equal(result.errors.length, 0);
  });

  it('returns result with source map field', async () => {
    const result = await transform('const x = 1;', {
      format: 'esm',
      sourceMap: true,
    });
    assert.equal(typeof result.code, 'string');
    assert.ok(Array.isArray(result.warnings));
  });
});
