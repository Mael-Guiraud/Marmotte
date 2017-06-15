

for i in `cat ../addressescalcul`;
do

	scp ../const.h pi@$i:/home/pi

done
