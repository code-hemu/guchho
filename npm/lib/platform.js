import { createRequire } from 'module';
import { fileURLToPath } from 'url';
import path from 'path';

const require = createRequire(import.meta.url);
const __dirname = path.dirname(fileURLToPath(import.meta.url));

const knownPlatforms = {
  'win32-x64': 'guchho-win32-x64',
  'win32-arm64': 'guchho-win32-arm64',
  'linux-x64': 'guchho-linux-x64',
  'linux-arm64': 'guchho-linux-arm64',
  'linux-arm': 'guchho-linux-arm',
  'darwin-x64': 'guchho-darwin-x64',
  'darwin-arm64': 'guchho-darwin-arm64',
};

const platformKey = `${process.platform}-${process.arch}`;
const binaryName = process.platform === 'win32' ? 'guchho.exe' : 'guchho';

function getPlatformPackage() {
  const pkg = knownPlatforms[platformKey];
  if (!pkg) {
    throw new Error(
      `Unsupported platform: ${platformKey}. ` +
      `guchho supports: ${Object.keys(knownPlatforms).join(', ')}`
    );
  }
  return pkg;
}

function getBinaryPath() {
  const pkg = getPlatformPackage();
  try {
    const pkgMain = require.resolve(pkg);
    const pkgDir = path.dirname(pkgMain);
    return path.join(pkgDir, binaryName);
  } catch {
    const localBinary = path.resolve(__dirname, '..', 'bin', binaryName);
    return localBinary;
  }
}

export { getPlatformPackage, getBinaryPath, binaryName, platformKey };
