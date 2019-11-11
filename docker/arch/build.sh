#! /bin/bash -ex
SRC_PATH=/app/brewtarget
AUR_PATH=/app/arch

echo -e "\nStarting build...\n"
cd $AUR_PATH
makepkg -sirc --noconfirm
