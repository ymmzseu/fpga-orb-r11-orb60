"""Fast-forward an authorized Git branch through GitHub Git Data API.

Uses the existing Git credential helper in memory only. Never logs credentials.
Checks all uploaded blob/tree/commit SHAs against the exact local Git objects.
"""
import argparse
import base64
from concurrent.futures import ThreadPoolExecutor,as_completed
from datetime import datetime,timezone
import hashlib
import json
from pathlib import Path
import re
import subprocess
from urllib.error import HTTPError
from urllib.request import Request,urlopen

def run(a):
    repo=a.repo.resolve();output=a.output.resolve();output.mkdir(parents=True,exist_ok=True)
    def git(*args):return subprocess.check_output(['git','-C',str(repo),*args])
    assert not git('status','--porcelain').strip(),'Commit the reviewable package first'
    branch=git('branch','--show-current').decode().strip();assert branch=='codex/orb60-4ppc'
    commit=git('rev-parse','HEAD').decode().strip();parent=git('rev-parse','HEAD^').decode().strip()
    raw=git('cat-file','commit',commit);head,message=raw.split(b'\n\n',1)
    tree_sha=git('rev-parse','HEAD^{tree}').decode().strip()
    auth=subprocess.run(['git','-C',str(repo),'credential','fill'],input='protocol=https\nhost=github.com\n\n',
                        text=True,capture_output=True,check=True)
    fields=dict(line.split('=',1) for line in auth.stdout.splitlines() if '=' in line)
    secret=fields['password'];auth=None;fields=None
    endpoint='https://api.github.com/repos/ymmzseu/fpga-orb-r11-orb60'
    def api(method,path,value=None):
        body=json.dumps(value).encode() if value is not None else None
        request=Request(endpoint+path,data=body,method=method,headers={
            'Authorization':'Bearer '+secret,'Accept':'application/vnd.github+json',
            'Content-Type':'application/json','User-Agent':'ORB60-source-publication',
            'X-GitHub-Api-Version':'2022-11-28'})
        try:
            with urlopen(request,timeout=30) as response:return json.load(response)
        except HTTPError as error:
            # Status only: avoid printing response/request headers containing auth.
            raise RuntimeError(f'GitHub API {method} {path}: HTTP{error.code}') from None
    ref='/git/ref/heads/'+branch
    assert api('GET',ref)['object']['sha']==parent,'Remote branch changed; preserve its new work'
    prior=api('GET','/git/commits/'+parent)
    def entries(revision):
        result={}
        for item in git('ls-tree','-rz',revision).split(b'\0'):
            if not item:continue
            meta,path=item.split(b'\t',1);mode,kind,digest=meta.decode().split()
            assert kind=='blob';result[path.decode('utf-8')]={'mode':mode,'type':kind,'sha':digest}
        return result
    old,new=entries(parent),entries(commit)
    known={e['sha'] for e in old.values()}
    changes=[dict(path=name,**entry) for name,entry in new.items() if old.get(name)!=entry]
    changes += [dict(path=name,mode=e['mode'],type='blob',sha=None) for name,e in old.items() if name not in new]
    blobs={entry['sha']:entry['path'] for entry in changes if entry['sha'] and entry['sha'] not in known}
    state=dict(State='RUNNING',Stage='BLOBS',Repository='ymmzseu/fpga-orb-r11-orb60',Branch=branch,
        LocalCommit=commit,Parent=parent,Files=len(new),ChangedFiles=len(changes),UniqueNewBlobs=len(blobs),Uploaded=0)
    def save(): (output/'github_api_status.json').write_text(json.dumps(state,indent=2)+'\n')
    save();print(f'GITHUB_API_BLOBS total={len(blobs)} changed_paths={len(changes)}',flush=True)
    def upload(item):
        digest,path=item;content=git('cat-file','blob',digest)
        assert hashlib.sha1(b'blob '+str(len(content)).encode()+b'\0'+content).hexdigest()==digest
        assert api('POST','/git/blobs',dict(content=base64.b64encode(content).decode(),encoding='base64'))['sha']==digest
    try:
        with ThreadPoolExecutor(max_workers=4) as pool:
            for future in as_completed([pool.submit(upload,item) for item in blobs.items()]):
                future.result();state['Uploaded']+=1;save()
                if state['Uploaded']%25==0:print(f"GITHUB_API_UPLOADED {state['Uploaded']}/{len(blobs)}",flush=True)
        state['Stage']='TREE';save()
        assert api('POST','/git/trees',dict(base_tree=prior['tree']['sha'],tree=changes))['sha']==tree_sha
        def identity(kind):
            line=next(line.decode() for line in head.splitlines() if line.startswith((kind+' ').encode()))
            match=re.fullmatch(kind+r' (.*) <([^>]+)> (\d+) ([+-]\d{4})',line)
            assert match and match[4]=='+0000','Create the local publication commit with UTC dates'
            return dict(name=match[1],email=match[2],date=datetime.fromtimestamp(int(match[3]),timezone.utc).isoformat())
        state['Stage']='COMMIT';save()
        payload=dict(message=message.decode('utf-8'),tree=tree_sha,parents=[parent],author=identity('author'),committer=identity('committer'))
        remote=api('POST','/git/commits',payload);assert remote['sha']==commit,'Remote commit serialization differs; ref remains untouched'
        assert api('GET',ref)['object']['sha']==parent,'Concurrent branch update; ref remains untouched'
        state['Stage']='FAST_FORWARD';save()
        assert api('PATCH','/git/refs/heads/'+branch,dict(sha=commit,force=False))['object']['sha']==commit
        assert api('GET',ref)['object']['sha']==commit
        state.update(State='SUCCEEDED',Stage='REMOTE_VERIFIED',RemoteCommit=commit,
                     Url='https://github.com/ymmzseu/fpga-orb-r11-orb60/tree/'+branch)
        print('GITHUB_API_REMOTE_VERIFIED '+commit,flush=True)
    except BaseException as error:
        state.update(State='FAILED',Error=str(error));raise
    finally:save()

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--repo',type=Path,required=True);p.add_argument('--output',type=Path,required=True)
    run(p.parse_args())
