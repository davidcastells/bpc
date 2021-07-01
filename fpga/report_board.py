import os
import sys

def systemOutput(cmd):
    stream = os.popen(cmd)
    outStr = stream.read()
    lines = outStr.split('\n');
    return lines

def getInfo():
    info = {
		'DE5NET': {'fmax':'kernel_fmax', 'fmax-file':'quartus_sh_compile.log', 'fmax-sep':' ', 'fmax-pos':2, 'resources-file':'top.flow.rpt', 'start':68, 'len':30, 'registers-key':''},
		'OSK' :   {'fmax':'kernel_fmax', 'fmax-file':'quartus_sh_compile.log', 'fmax-sep':' ', 'fmax-pos':2,'resources-file':'top.flow.rpt', 'start':63, 'len':25},
		'HARP' :   {'fmax':'"Kernel fmax"', 'fmax-file':'acl_quartus_report.txt', 'fmax-sep':':', 'fmax-pos':1,'resources-file':'top.flow.rpt', 'start':63, 'len':25, 'registers-key':'Total registers', 'total_alms':427200, 'total_kles':1150, 'total_registers':1708800},
		'PAC10' :   {'fmax':'"Kernel fmax"', 'fmax-file':'acl_quartus_report.txt', 'fmax-sep':':','fmax-pos':1,'resources-file':'acl_quartus_report.txt', 'start':7, 'len':7},
		'PACS10' :   {'fmax':'"Kernel fmax"', 'fmax-file':'build/acl_quartus_report.txt', 'fmax-sep':':', 'fmax-pos':1,'resources-file':'build/output_files/afu_default.flow.rpt', 'start':75, 'len':22, 'registers-key':'Total dedicated logic registers', 'total_alms':933120, 'total_kles':2753, 'total_registers':3732480},
    }
    return info

def getDesignDir(dsg, et, th, pl, tl=None):
    if (tl==None):
        tl = pl
    if (th == -1):
        dir = '{}_e{}'.format(dsg, et)
    else:
        dir = '{}_e{}_{}'.format(dsg, et, th)
    if (pl == -1):
        ptype = '_var'
    else:
        ptype = '_{}_{}'.format(pl, tl)
    return dir+ptype

def getDesignFmax(board, dsg, et, th, pl, tl=None):
    if (tl==None):
        tl = pl

    info = getInfo()
    sfmaxsep = info[board]['fmax-sep']
    fmaxpos = info[board]['fmax-pos']
    sfmaxfile = info[board]['fmax-file']
    query = info[board]['fmax']

    sdir = getDesignDir(dsg, et, th, pl, tl)
    sfile = '{}/{}'.format(sdir, sfmaxfile)
    if (os.path.exists(sfile) == False):
        return '?'
    cmd = 'grep {} {}'.format(query, sfile)
    sout = systemOutput(cmd)
    sout = sout[0]
    if ('fmax' in sout):
        return int(float(sout.split(sfmaxsep)[fmaxpos].strip()))
    else:
        return '?'

def getDesignResources(board, dsg, et, th, pl, tl=None):
    if (tl==None):
        tl = pl

    info = getInfo()
    sresourcesfile = info[board]['resources-file']
    sdir = getDesignDir(dsg, et, th, pl, tl)
    sfile = '{}/{}'.format(sdir, sresourcesfile)
    if (os.path.exists(sfile) == False):
        return '?'
    cmd = ' grep "Logic utilization" {}'.format(sfile)
    sout = systemOutput(cmd)
    ret = sout[0]
    if ('ALMs' in ret):
        # PACS10
        part = ret.split('(')
        part = part[2].split(')')
        ret = part[0]
        return ret.strip()
    elif ('Logic utilization' in ret):
        part = ret.split('(')
        part = part[1].split(')')
        ret = part[0].strip()
    else:
        return '?'
    return ret

def getDesignResourcesRaw(board, dsg, et, th, pl, tl=None):
    if (tl==None):
        tl = pl

    info = getInfo()
    sresourcesfile = info[board]['resources-file']
    sdir = getDesignDir(dsg, et, th, pl, tl)
    sfile = '{}/{}'.format(sdir, sresourcesfile)
    if (os.path.exists(sfile) == False):
        return '?'
    cmd = ' grep "Logic utilization" {}'.format(sfile)
    sout = systemOutput(cmd)
    ret = sout[0]
    if ('ALMs' in ret):
        # PACS10
        part = ret.split(';')
        part = part[2].split('(')
        ret = part[0]
        return ret.strip()
    elif ('Logic utilization' in ret):
        part = ret.split('(')
        part = part[1].split(')')
        ret = part[0].strip()
    else:
        return '?'
    return ret

def getDesignRegisters(board, dsg, et, th, pl, tl=None):
    if (tl==None):
        tl = pl

    info = getInfo()
    sresourcesfile = info[board]['resources-file']
    sdir = getDesignDir(dsg, et, th, pl, tl)
    sfile = '{}/{}'.format(sdir, sresourcesfile)
    if (os.path.exists(sfile) == False):
        return '?'
    cmd = ' grep "{}" {}'.format(info[board]['registers-key'], sfile)
    sout = systemOutput(cmd)
    ret = sout[0]
    if (';' in ret):
        part = ret.split(';')
        ret = part[2].strip()
        
    else:
        return '?'
    return ret

def reportDesign(board, name, th, et, pl):
    info = getInfo()

    if (th == -1):
        dir = '{}_e{}'.format(name, et)
    else:
        dir = '{}_e{}_{}'.format(name, et, th)

    if (pl == -1):
        ptype = '_var'
    else:
        ptype =  '_{}_{}'.format(pl, pl)

    aocx = dir+ptype

    print("", flush=True)	
    print("========================================================", flush=True)
    print('| Results for', aocx,  flush=True)
    print("========================================================", flush=True)
	
    sfmax = info[board]['fmax']
    sfmaxfile = info[board]['fmax-file']
    sstart = info[board]['start']
    slen = info[board]['len']
    sresourcesfile = info[board]['resources-file']
    os.system('grep {} {}/{}'.format(sfmax, aocx, sfmaxfile))
    os.system('head -n {} {}/{} | tail -n {}'.format(sstart, aocx, sresourcesfile, slen))



