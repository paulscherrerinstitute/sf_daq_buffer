import os
import json
import sys

config_file = '/home/dbe/sls/hosts/xbl-daq-28/cSAXS.EG01V01.json'

if len(sys.argv) != 2:
    print("error... bit depth argument missing")

bit_d = sys.argv[1]

with open(config_file) as f:
    content = json.load(f)

content['bit_depth'] = int(bit_d)
    #f.truncate(0)
    #json.dump(content, f, indent='\t', separators=(',', ': '))

with open(config_file, 'w') as f:
    f.seek(0)
    json.dump(content, f, indent='\t')

#os.chdir('/home/dbe/sls/hosts/xbl-daq-28/')
#rc = os.system(f'git pull && git add {config_file} && git commit -m "[LOG] config test change push" && git push')
#print('result value: ', rc)



