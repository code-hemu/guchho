import { describe, it } from 'node:test';
import assert from 'node:assert';
import { build } from '../../../npm/lib/index.js';

describe('guchho.build()', () => {
  it('builds a basic project', async () => {
    const result = await build({
      entryPoints: ['tests/fixtures/basic/src/index.js'],
      outDir: 'tests/fixtures/basic/dist',
      format: 'esm',
    });
    assert.ok(result);
    assert.equal(result.errors?.length, 0);
  });
});
