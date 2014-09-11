clear
echo "Building"
pebble build || exit

echo "Installing"
pebble install --phone 192.168.1.104 || exit
echo "Done"


