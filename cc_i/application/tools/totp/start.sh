gcc ./client/main.c
gcc ./server/main.c

echo "./client/a.out" > client.sh
chmod a+x ./client.sh
gnome-terminal . ./client.sh
#gnome-terminal -e sh ./client.sh 
#xterm -e sh ./client.sh
./server/a.out
rm ./client.sh
