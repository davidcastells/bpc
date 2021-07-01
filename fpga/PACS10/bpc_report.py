import os
import sys
sys.path.append('..')
import report_board as rb


def extractLEs(alms):
    global info
    if (alms == '?'):
        return '?', '?'

    part = alms.split('/')
    num = int(part[0].replace(',',''))
    den = int(part[1].replace(',',''))
    perc = num/den
    totalkLEs = info['total_kles']
    kLEs = int(totalkLEs * perc)
    return kLEs, int(perc*100)

def extractFFs(regs):
    global info
    totalkFFs = info['total_registers'] / 1000.0
    if (regs == '?'):
        return '?', '?'
    num = int(regs)/1000
    den = totalkFFs
    perc = num/den
    kFFs = int(totalkFFs * perc)
    return kFFs, int(perc*100)
  

if __name__ == "__main__":
    global info
    board='PACS10'
    dsg = 'bpc'
    et = 0
    tl = 106
    info = rb.getInfo()[board]

    print('BPC resources in {}'.format(board))
    print('-------------------------------')
    print('|{: <5}|{: <5}|{: <5}|{: <5}|{: <5}|{: <5}|{: <5}|{: <5}|{: <5}|'.format('et', 'th', 'pl', 'tl', 'kLEs', '%LEs', 'kFFs', '%FFs', 'fmax'))

    for pl in [100, 200, 300]:
        for erate in [0.03, 0.05, 0.1]:
            th = int(erate*pl)
            tl = pl + 2 * th
            total = pl + tl
            if (total > (512//2)):
                et = 1
            if (pl > (512//2) or tl > (512//2)):
                et = 2 

            v = rb.getDesignResourcesRaw(board, dsg, et, th, pl, tl)
            
            kLEs, pcLEs = extractLEs(v)
            regs = rb.getDesignRegisters(board, dsg, et, th, pl, tl)
            kFFs, pcFFs = extractFFs(regs)
            
            fmax = rb.getDesignFmax(board, dsg, et, th, pl, tl)
            #print('||{: <5}|'.format(slink, v), end='')
            print('|{: <5}|{: <5}|{: <5}|{: <5}|{: <5}|{: <5}|{: <5}|{: <5}|{: <5}|'.format(et, th, pl, tl, kLEs, pcLEs, kFFs, pcFFs, fmax))



