"""Native bulk integration gate, with explicit DDR-model limits and frozen inputs."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess

from prepare_native_frame import prepare
from check_native_harness import check


def run(args):
    state = dict(State='RUNNING', Stage='PREPARE', ProcessId=args.parent_pid,
                 StartedUtc=datetime.now(timezone.utc).isoformat(),
                 Scope='Actual two-core RTL, one randomized arbitrated DDR model; host scheduling only, no PS firmware/AMD DDR PHY/board claim')
    def save(): (args.output/'status.json').write_text(json.dumps(state, indent=2)+'\n')
    save()
    try:
        proof = json.loads((args.protocol/'status.json').read_text(encoding='utf-8-sig'))
        assert proof['State'] == 'SUCCEEDED'
        assert Path(proof['LayerHlsJob']).resolve() == args.layer.resolve()
        assert Path(proof['DescriptorHlsJob']).resolve() == args.descriptor.resolve()
        for entry in json.loads((args.output/'test_manifest.json').read_text(encoding='utf-8-sig')):
            assert hashlib.sha256((args.output/entry['Path']).read_bytes()).hexdigest().upper() == entry['SHA256']
        scratch = Path(os.environ['FPGA_VIVADO_BUILD_ROOT'])/'orb60'/('n'+args.output.name[-8:])
        scratch.mkdir(parents=True, exist_ok=False)
        state['Scratch'] = str(scratch);save()
        images = prepare(args, scratch)
        check(scratch,Path(__file__).resolve().parent,args.output)
        shutil.copyfile(Path(__file__).with_name('native_frame.cpp'), scratch/'native_frame.cpp')
        rtl = sorted(scratch.glob('layer2/*.v')) + sorted(scratch.glob('descriptor/*.v'))
        version = subprocess.run([str(args.verilator), '--version'], capture_output=True, text=True, check=True)
        (args.output/'tool_version.txt').write_text(version.stdout+version.stderr)
        state['Stage']='COMPILE';save()
        # Windows .cmd launchers have an 8191-character command limit. Keep the
        # immutable RTL list in Verilator's native argument-file format.
        sources=['orb60_frame_top.sv','native_frame.cpp']+[p.relative_to(scratch).as_posix() for p in rtl]
        (scratch/'sources.f').write_text('\n'.join(sources)+'\n')
        cmd = [str(args.verilator), '--cc', '--exe', '--build', '--no-timing', '-j', '4', '-Wno-fatal',
               '--top-module', 'orb60_frame_top', '--Mdir', 'obj', '-CFLAGS', '-std=c++17 -O2',
               '-I'+str(scratch/'layer2'), '-I'+str(scratch/'descriptor'),
               '-f','sources.f']
        print('ORB60_NATIVE_COMPILE', flush=True)
        with (args.output/'compile.log').open('w') as log:
            code=subprocess.run(cmd, cwd=scratch, stdout=log, stderr=subprocess.STDOUT).returncode
        if code: raise RuntimeError(f'Verilator compilation exit {code}; inspect compile.log')
        exe = scratch/'obj/Vorb60_frame_top.exe'
        assert exe.is_file(), exe
        with (args.output/'link_reset.log').open('w') as log:
            code=subprocess.run([str(exe),'--link-smoke'],cwd=scratch,stdout=log,stderr=subprocess.STDOUT).returncode
        if code: raise RuntimeError(f'Native link/reset startup exit {code}; inspect link_reset.log')
        runs = []
        for beats, stalls, seed in ((2, 0, 6001), (1, 27, 6002)):
            label=f'beats{beats}_stall{stalls}'
            state['Stage']='SIMULATE_'+label;save()
            result=args.output/(label+'.json')
            print('ORB60_NATIVE_SIMULATE '+label, flush=True)
            command=[str(exe),str(images.resolve()),str((args.descriptor/'descriptor_vectors').resolve()),
                     str(result.resolve()),str(beats),str(stalls),str(seed)]
            with (args.output/(label+'.log')).open('w') as log:
                code=subprocess.run(command, cwd=scratch, stdout=log, stderr=subprocess.STDOUT).returncode
            if code: raise RuntimeError(f'Native simulation {label} exit {code}; inspect {label}.log')
            value=json.loads(result.read_text())
            assert value['status']=='PASS' and len(value['frames'])==3
            assert [v['selected'] for v in value['frames']]==[280,942,0]
            for frame in value['frames']:
                frame['milliseconds']=frame['cycles']/100000
                frame['equivalent_fps']=100000000/frame['cycles']
            runs.append(value)
        # Prove neither native integration nor staging altered the accepted RTL.
        for entry in json.loads((args.output/'rtl_manifest.json').read_text()):
            job=args.layer if entry['kernel']=='layer2' else args.descriptor
            source=job/f"projects/{entry['kernel']}/solution1/syn/verilog/{entry['name']}"
            assert hashlib.sha256(source.read_bytes()).hexdigest()==entry['sha256']
        result=dict(status='PASS', frequency_mhz=100, runs=runs, physical_board_fps_verified=False,
                    ps_modified=False, rtl_unchanged=True,
                    limits=['DDR service budgets are hypotheses, not measured Zynq DDR/AMD-interconnect models',
                            'Strict descriptor(n)/image(n+1) overlap includes simulated AXI-Lite scheduling',
                            'Three inputs and two seeds do not establish representative-video P99/max',
                            'Board PS cache visibility/DDR bandwidth/video/power remain unverified'])
        (args.output/'performance.json').write_text(json.dumps(result,indent=2)+'\n')
        state.update(State='SUCCEEDED', Stage='COMPLETE')
        print('ORB60_NATIVE_FRAME_PASS',flush=True)
    except BaseException as error:
        state.update(State='FAILED', Error=str(error));raise
    finally:
        state['FinishedUtc']=datetime.now(timezone.utc).isoformat();save()


if __name__=='__main__':
    p=argparse.ArgumentParser()
    for key in ('layer','descriptor','benchmark','protocol','verilator','output'):
        p.add_argument('--'+key,type=Path,required=True)
    p.add_argument('--parent-pid',type=int,required=True)
    run(p.parse_args())
