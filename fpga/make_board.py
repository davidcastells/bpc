import os

gCompiled = 0
gCompiling = 0
gStarted = 0

gOnFlyMax = 5

def modified_date(path_to_file):
    stat = os.stat(path_to_file)
    return stat.st_mtime

def withoutExtension(file):
    part = file.split('.')
    return part[0]

def isCompiling(aocx):
    stream = os.popen('ps -ef')
    outStr = stream.read()
    return (aocx in outStr)

def isCompiled(aocx, cl):

    if (os.path.exists(aocx) == False):
        print(aocx + ' is not compiled')
        return False

    aocx_date = modified_date(aocx)
    cl_date = modified_date(cl)
    if (aocx_date > cl_date):
        return True

def getAoco(aocx):
    part = aocx.split('.aocx')
    return part[0] + '.aoco'

def aocoExist(aocx):
    aoco = getAoco(aocx)
    if (os.path.exists(aoco) == True):
        print('AOCO file found ' , aoco)
        return True
    else:
        return False


def designConstant(aocx):
    part = aocx.split('_')
    return part[0].upper()

def system(cmd):
    print(cmd)
    os.system(cmd)

def metaprogram(dsg,  meta=None, level='', cl=None, flags=''):
    if (cl == None):
       cl = '../{}.cl'.format(dsg)
    cl_generator = '/tmp/{}.cl.generator'.format(dsg)
    cl_generator_cpp = cl_generator +'.cpp'
    if (meta == None):
       meta = '../{}{}.cl.metaprogram'.format(dsg, level)

    if (isCompiled(cl, meta)):
        print('Skipping metaprogramming. {} already metaprogrammed'.format(cl))
        return

    system('../metagenerator {} > {}'.format(meta, cl_generator_cpp))
    system('g++ {} {} -o {}'.format(cl_generator_cpp, flags, cl_generator))
    system('{} > {}'.format(cl_generator, cl))

def makeAocx(aocx, cl, threshold=-1, pattern_len=-1, text_len=-1, entry_type=0, extra_flags='', blocking=False, ignoretargets=[], meta=None):
    global gCompiling
    global gCompiled
    global gStarted

    if (meta == None):
        filedate = meta
    else:
        filedate = cl

    if (aocx in ignoretargets):
        print(aocx, 'in ignore list')
        return

    if (isCompiling(aocx)):
        print('Already compiling ' + aocx)
        gCompiling = gCompiling + 1
        return
    if (isCompiled(aocx, filedate)):
        print('Already compiled ' + aocx)
        gCompiled = gCompiled + 1
        return

    if ((gCompiling + gStarted) > gOnFlyMax and blocking == False):
        print('concurrent works limit reached')
        return

    if (aocoExist(aocx)):
        cl = getAoco(aocx)

    entry_type_flags = ' -D ENTRY_TYPE_{} '.format(entry_type)
    common_flags = '-D BASIC_AP_UINT -I $INTELFPGAOCLSDKROOT/include/kernel_headers -g'
    design_name = withoutExtension(aocx)

    if (threshold >= 0):
        threshold_flag = ' -D ' + designConstant(aocx) + '_THRESHOLD={}'.format(threshold)
    else:
        threshold_flag = ' '

    if (pattern_len > 0):
        item_flags = ' -D PATTERN_LEN={} -D TEXT_LEN={} '.format(pattern_len, text_len)
    else:
        item_flags = ' '

    if (blocking):
        nohup = ''
        noblocksuffix = ''
    else:
        nohup = 'nohup'
        noblocksuffix = '&'
    outlog = ' >> compile.' + design_name + '.out'
    cmd = nohup + ' aoc '+common_flags + threshold_flag + entry_type_flags + extra_flags + item_flags+cl+' -o '+aocx + outlog + noblocksuffix
    print(cmd)
    os.system('echo "'+cmd+'" > compile.' + design_name + '.out')
    os.system(cmd)

    gStarted = gStarted + 1





def makeBpc(BOARD, AOCL_FLAGS, blocking=False):

   print('COMPILING Myers {}:'.format(BOARD));

   dsg = 'bpc'
   meta = '../bpc_v1.cl.metaprogram'

   for erate in  [0.03, 0.05, 0.1]:
       for pl in  [100,200,300]:
         #erate = 0.1
         #pl = 100
         th = int(pl * erate) 
         tl = pl + 2*th

         total = pl + tl 
         if ((total + 2) < (512//2)):
            etype = 0
         elif (pl < (512//2) and tl < (512//2)):
            etype = 1
         elif (pl < (1024//2) and tl < (1024//2)):
            etype = 2
         else:
            raise Exception('total = {} pl: {} tl: {}'.format(total, pl, tl))

         print('Total length ={}'.format(total))

         cl = 'bpc_e{}_{}_{}_{}.cl'.format(etype, th, pl, tl)
         aocx = 'bpc_e{}_{}_{}_{}.aocx'.format(etype, th, pl, tl)
         flags='-D ENTRY_TYPE_{} -D BPC_THRESHOLD={} -D PATTERN_LEN={} -D TEXT_LEN={}'.format(etype, th,pl, tl)
         metaprogram(dsg, meta=meta, cl=cl, flags=flags)

         makeAocx(aocx=aocx, cl=cl, threshold=th, pattern_len=pl, text_len=tl, extra_flags=AOCL_FLAGS, entry_type=etype, blocking=blocking, meta=meta)




