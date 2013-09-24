sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
sudo update-alternatives --remove-all gcc 
sudo update-alternatives --remove-all g++ 
sudo apt-get update
sudo apt-get install g++-4.8 -y
sudo apt-get upgrade -y && sudo apt-get dist-upgrade -y
