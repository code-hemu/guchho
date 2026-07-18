import { describe, it } from 'node:test';
import assert from 'node:assert';
import { build } from '../../../npm/lib/index.js';

describe('Vue project build', () => {
  it('builds the vue fixture', async () => {
    const result = await build({
      entryPoints: ['tests/fixtures/vue/src/main.js'],
      outDir: 'tests/fixtures/vue/dist',
      format: 'esm',
    });
    assert.ok(result);
    assert.equal(result.errors?.length, 0);
  });
});
