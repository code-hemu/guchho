import { describe, it } from 'node:test';
import assert from 'node:assert';
import { build } from '../../../npm/lib/index.js';

describe('React project build', () => {
  it('builds the react fixture', async () => {
    const result = await build({
      entryPoints: ['tests/fixtures/react/src/index.jsx'],
      outDir: 'tests/fixtures/react/dist',
      format: 'esm',
    });
    assert.ok(result);
    assert.equal(result.errors?.length, 0);
  });
});
