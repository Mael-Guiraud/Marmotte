

for i in `cat /home/merelas/Bureau/parallel/addressescalcul`;
do

	scp /home/merelas/Bureau/parallel/const.h pi@$i:/home/pi
	
done

