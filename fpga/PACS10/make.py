BOARD='PACS10'
TIME='/usr/bin/time -f TIME=%E'
AOCL_FLAGS=''

import sys
sys.path.append('..')
import make_board as mb

blocking=False

mb.makeBpc(BOARD, AOCL_FLAGS, blocking=blocking)
