import { createRequire } from 'module';
import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

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

function install() {
  const platformPkg = knownPlatforms[platformKey];
  if (!platformPkg) {
    console.warn(`guchho: unsupported platform "${platformKey}" — no native binary available.`);
    return;
  }

  const binDir = path.resolve(__dirname, 'bin');
  const targetPath = path.join(binDir, binaryName);

  try {
    const pkgMain = require.resolve(platformPkg);
    const pkgDir = path.dirname(pkgMain);
    const binarySource = path.join(pkgDir, binaryName);

    if (!fs.existsSync(binarySource)) {
      console.warn(`guchho: binary not found in ${platformPkg} at ${binarySource}`);
      return;
    }

    fs.mkdirSync(binDir, { recursive: true });

    if (fs.existsSync(targetPath)) {
      fs.unlinkSync(targetPath);
    }

    fs.copyFileSync(binarySource, targetPath);

    if (process.platform !== 'win32') {
      fs.chmodSync(targetPath, 0o755);
    }

    console.log(`guchho: installed native binary for ${platformKey}`);
  } catch {
    console.warn(
      `guchho: platform package "${platformPkg}" not installed. ` +
      `Run "npm install ${platformPkg}" to install the native binary.`
    );
  }
}

install();
