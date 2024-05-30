script="scratch/experiment"

nDevices=182    # Change from 50 to 200

MobileNodeProbability=1

period=30

folder="scratch/experiments/Logs_period-${period}_nDevices-${nDevices}/run"

echo -n "Running experiments: "

for r in `seq 1 30`;

do

  echo -n " ${r} "

  mkdir -p $folder${r}

	./ns3 run "$script --RngRun=$r --nDevices=$nDevices --Period=${period} --OutputFolder=${folder}${r}" > "$folder${r}/log.txt" 2>&1

done

echo " END"
