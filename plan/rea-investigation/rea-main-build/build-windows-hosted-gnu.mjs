// Receita experimental: fontes e flags upstream, GNU MinGW alojado no Windows.
// Não altera scripts upstream nem constitui o percurso oficial Linux.
import { createHash } from 'node:crypto';
import { spawnSync } from 'node:child_process';
import { readFile, mkdir, writeFile } from 'node:fs/promises';
import { join } from 'node:path';

const root = 'C:/ORCA/Fantasynt/tools/vendor/rea-main-source/rea-df2c08b546d4f40103e11b8b31dbfc3d41ee687b';
const toolchain = 'C:/ORCA/Fantasynt/tools/vendor/rea-main-source/toolchain';
const output = join(root, 'native/windows/build');
const bin = join(toolchain, 'winlibs/mingw64/bin');
const headers = join(toolchain, 'node-v22.19.0/include/node');
const cache = join(toolchain, 'rea-build');
const digest = bytes => createHash('sha256').update(bytes).digest('hex');
const packageJson = JSON.parse(await readFile(join(root, 'package.json'), 'utf8'));
const publishedChecksums = await readFile(join(toolchain, 'SHASUMS256.txt'), 'utf8');
const expectedHeaders = publishedChecksums.split('\n').find(line => line.slice(66).trim() === 'node-v22.19.0-headers.tar.gz')?.slice(0,64);
if (expectedHeaders === undefined || digest(await readFile(join(toolchain, 'node-v22.19.0-headers.tar.gz'))) !== expectedHeaders)
  throw new Error('Official Node headers SHA-256 mismatch');
const winlibsRelease = JSON.parse(await readFile('C:/ORCA/Fantasynt/plan/rea-investigation/rea-main-build/winlibs-release.json', 'utf8'));
const winlibsAsset = winlibsRelease.assets.find(asset => asset.name === 'winlibs-x86_64-posix-seh-gcc-16.2.0-mingw-w64ucrt-14.0.0-r2.zip');
if (winlibsAsset?.digest !== `sha256:${digest(await readFile(join(toolchain, winlibsAsset.name)))}`)
  throw new Error('Pinned WinLibs SHA-256 mismatch');
await mkdir(output, { recursive: true });
await mkdir(cache, { recursive: true });
const run = (command, args) => {
  const result = spawnSync(command, args, {
    cwd: root, encoding: 'utf8', maxBuffer: 4 * 1024 * 1024,
    timeout: 120000, windowsHide: true,
  });
  if (result.error || result.status !== 0)
    throw new Error(`Experimental GNU command failed: ${command}\n${result.error?.message ?? ''}\n${result.stdout}\n${result.stderr}`);
  process.stdout.write(result.stdout);
  process.stderr.write(result.stderr);
};
const compiler = join(bin, 'x86_64-w64-mingw32-g++.exe');
// WinLibs fornece o dlltool GNU deste alvo com nome sem prefixo.
const dlltool = join(bin, 'dlltool.exe');
run(compiler, ['--version']);
run(dlltool, ['--version']);
const sources = ['addon.cc', 'filesystem.cc', 'process.cc'];
const sourceText = (await Promise.all(sources.map(path => readFile(join(root, 'native/windows/src', path), 'utf8')))).join('\n');
const symbols = [...new Set([...sourceText.matchAll(/\b(napi_[a-z0-9_]+)\s*\(/gu)].map(match => match[1]))].sort();
const definition = join(cache, 'node-api.def');
const library = join(cache, 'libnode-api.a');
await writeFile(definition, `LIBRARY node.exe\nEXPORTS\n${symbols.join('\n')}\n`);
run(dlltool, ['-d', definition, '-y', library]);
const artifact = 'rea-windows-x64.node';
const flags = [
  '-std=c++17', '-O2', '-DNAPI_VERSION=8', '-D_WIN32_WINNT=0x0A00',
  '-shared', '-static', '-static-libgcc', '-static-libstdc++',
  '-Wl,--no-insert-timestamp', `-I${headers}`,
  'native/windows/src/addon.cc', 'native/windows/src/filesystem.cc',
  'native/windows/src/process.cc', library,
  '-ladvapi32', '-lbcrypt', '-lntdll', '-o', join(output, artifact),
];
run(compiler, flags);
const bytes = await readFile(join(output, artifact));
const manifest = {
  packageVersion: packageJson.version, platform: 'win32', architecture: 'x64',
  abiVersion: 1, nodeApiVersion: 8, artifact, artifactSha256: digest(bytes),
};
await writeFile(join(output, 'manifest.json'), `${JSON.stringify(manifest, null, 2)}\n`);
await writeFile(join(cache, 'recipe-result.json'), `${JSON.stringify({
  experimentalHost: process.platform, root, compiler, dlltool, headers,
  symbols, compilerFlags: flags, ...manifest, artifactBytes: bytes.length,
}, null, 2)}\n`);
process.stdout.write(`${JSON.stringify({ ...manifest, artifactBytes: bytes.length })}\n`);
