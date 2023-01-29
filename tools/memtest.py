import os
import sys
from datetime import datetime

def line_parser (path):
    file = open(path, 'r');

    vmem = {}
    loads = {}
    allmods = []
    total_check = 0
    
    while True:
      line = file.readline()
      if not line:
        break

      if '**' in line:
        # parse!
        total_check += 1
        tokens = line.split(';')
        tokens[2] = tokens[2].replace('\n', '')

        if tokens[2] not in allmods:
          allmods.append(tokens[2])

        if tokens[0] == '**ALLC':
          vmem[tokens[1]] = tokens[2]
          if tokens[2] not in loads:
            loads[tokens[2]] = 0
          else:
            loads[tokens[2]] += 1

        elif tokens[0] == '**FREE':
          del vmem[tokens[1]]

    #
    # Working Modules
    #
    total_mod_count = len(allmods)
    for ptr in vmem:
      if vmem[ptr] in allmods:
        allmods.remove(vmem[ptr])

    print('Leak Free Modules:')
    for wmod in allmods:
      print(f'\tModule {wmod} is leak free!')


    #
    # Loads
    #

    print('\nMemory Loads:')
    for load in loads:
      print(f'\tModule {load} allocated {loads[load]} times')
    
    # check contents of Virtual Memory for memory leaks
    fails = 0
    mods = {}

    #
    # Memory Leaks
    #

    print('\nMemory Leaks:')

    for ptr in vmem:
      print(f'\t{ptr} not freed, {vmem[ptr]}')
      fails += 1

      if vmem[ptr] not in mods:
        mods[vmem[ptr]] = 1
      else:
        mods[vmem[ptr]] += 1


    leaking = 0
    print('\nModule Leaks:')
    for mod in mods:
      print(f'\tModule {mod} leaked {mods[mod]} times')
      leaking += 1

    working_count = len(allmods)
    print(f'\n\tWorking Modules: {working_count} / {total_mod_count} ({round(100 * working_count / total_mod_count)}%)')
    print(f'\tFailing Modules: {leaking} / {total_mod_count} ({round(100 * leaking / total_mod_count)}%)')
    print(f'\tTotal leaks: {fails} / {total_check} ({round(100 * fails / total_check)}%)')

    file.close()

def main():
    log_file = 'log.txt'
    print(f'PERFORMING MEMCHECK\n\tFILE: {log_file}\n\tDATE: {datetime.now().date()}\n\tTIME: {datetime.now().time()}\n')
    line_parser(log_file)


if __name__ == "__main__":
  main()