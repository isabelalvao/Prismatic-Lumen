"""Cliente temporário MCP: pedidos em ficheiros, evidência completa e sessão única."""
import json
import os
from pathlib import Path
import queue
import subprocess
import threading
import time
import sys

ROOT = Path('C:/ORCA/Fantasynt')
OUTPUT = ROOT / 'plan/rea-investigation'
REQUESTS = ROOT / 'temp/rea-requests-minimal'
REQUESTS.mkdir(exist_ok=True)
launcher = str(ROOT / 'tools/vendor/rea-main-source/rea-df2c08b546d4f40103e11b8b31dbfc3d41ee687b/scripts/rea.mjs')
environment = dict(os.environ,
    GHIDRA_INSTALL_DIR=str(ROOT / 'tools/vendor/ghidra_12.1.4_PUBLIC'),
    JAVA_HOME=str(ROOT / 'tools/vendor/jdk-21.0.12.1+1'),
    REA_ANALYSIS_PROVIDER='ghidra', GHIDRA_HEADLESS_MAXMEM='4G',
    REA_GHIDRA_DISABLED_ANALYZERS='Disassemble Entry Points,Reference,Scalar Operand References,WindowsResourceReference,Stack,Function ID,Decompiler Switch Analysis,Non-Returning Functions - Discovered',
    REA_GHIDRA_SEED_FILE=str(OUTPUT / 'targeted-native/priority-seeds.tsv'),
    REA_GHIDRA_STARTUP_TIMEOUT_MS='1200000')
errors = (OUTPUT / 'server-stderr-minimal.log').open('w', encoding='utf-8')
process = subprocess.Popen(['C:/Program Files/nodejs/node.exe', launcher, 'mcp'],
    stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=errors,
    text=True, encoding='utf-8', creationflags=subprocess.CREATE_NO_WINDOW,
    env=environment)
replies = queue.Queue()
def receive():
    for line in process.stdout:
        try:
            replies.put(json.loads(line))
        except json.JSONDecodeError:
            print('Non-JSON server output suppressed', flush=True)
    replies.put(None)
threading.Thread(target=receive, daemon=True).start()
next_id = 0
def rpc(method, params, timeout=1300):
    global next_id
    next_id += 1
    identifier = next_id
    process.stdin.write(json.dumps({'jsonrpc': '2.0', 'id': identifier,
        'method': method, 'params': params}) + '\n')
    process.stdin.flush()
    deadline = time.monotonic() + timeout
    while True:
        if time.monotonic() >= deadline:
            raise TimeoutError(f'REA request deadline: {method}')
        response = replies.get(timeout=max(0.001, deadline-time.monotonic()))
        if response is None:
            raise RuntimeError(f'REA exited: {process.poll()}')
        if response.get('id') == identifier:
            return response
def evidence(response):
    result = response.get('result', {})
    structured = result.get('structuredContent')
    if structured:
        return structured
    for block in result.get('content', []):
        if block.get('type') == 'text':
            try:
                return json.loads(block['text'])
            except (KeyError, json.JSONDecodeError):
                pass
    return result
try:
    initialized = rpc('initialize', {'protocolVersion': '2024-11-05',
        'capabilities': {}, 'clientInfo': {'name': 'Fantasynt research', 'version': '1'}}, 30)
    print('INITIALIZED', initialized.get('result', {}).get('serverInfo'), flush=True)
    process.stdin.write(json.dumps({'jsonrpc': '2.0', 'method': 'notifications/initialized'})+'\n')
    process.stdin.flush()
    completed = set()
    while True:
        pending = sorted(p for p in REQUESTS.glob('*.json') if p.name not in completed)
        if not pending:
            if process.poll() is not None:
                raise RuntimeError(f'REA exited: {process.returncode}')
            time.sleep(0.2)
            continue
        for request_file in pending:
            request = json.loads(request_file.read_text(encoding='utf-8'))
            if request.get('stop'):
                raise SystemExit(0)
            name = request['name']
            destination = OUTPUT / request['output']
            if destination.parent.resolve() != OUTPUT.resolve() or destination.exists():
                raise ValueError('Output must be new and directly inside evidence directory')
            print('START', request_file.name, name, flush=True)
            response = rpc('tools/call', {'name': name, 'arguments': request.get('arguments', {})})
            destination.write_text(json.dumps(response, indent=2), encoding='utf-8')
            record = evidence(response)
            print('DONE', request_file.name, name,
                record.get('evidence_id') if isinstance(record, dict) else '',
                'error' if response.get('error') or response.get('result', {}).get('isError') else 'ok',
                flush=True)
            completed.add(request_file.name)
            if response.get('error') or response.get('result', {}).get('isError'):
                raise RuntimeError(f'Recorded REA failure: {name}; stopping dependent requests')
finally:
    if process.poll() is None:
        try:
            rpc('tools/call', {'name': 'close_binary', 'arguments': {}}, 30)
        except (OSError,RuntimeError,TimeoutError,queue.Empty):
            pass
        finally:
            try:
                process.stdin.close()
            except OSError:
                pass
            try:
                process.wait(timeout=30)
            except subprocess.TimeoutExpired:
                process.terminate()
                process.wait(timeout=20)
    errors.close()
