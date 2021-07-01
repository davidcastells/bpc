BOARD='HARP'
TIME='/usr/bin/time -f TIME=%E'
AOCL_FLAGS=''

import time
import sys
sys.path.append('..')
import make_board as mb

blocking=False

mb.makeBpc(BOARD, AOCL_FLAGS, blocking=blocking)



