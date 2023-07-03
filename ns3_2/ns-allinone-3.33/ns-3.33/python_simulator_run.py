import subprocess

import time

seed=3 # 何個のseed値でシミュレーションを行うか


for s in range(seed):

	cmd="./waf"
	arg="simulation-proposal-scenario --seed=" + str(s+1) + " &"
        
	print("Command>>"+arg)

	subprocess.Popen(["nohup",cmd,"--run",arg])

	time.sleep(4.0)

        
