#
while (1)
	set date = `date`
	set date = $date[2]$date[3]
	echo starting colin at `date`
	cp colin colin.r
	./colin.r -epD -S2 >>& colin.log.$date
	if (-e core) then
		mv core colin.core
		mv colin.r colin.crashed
	endif
	sleep 240
end
