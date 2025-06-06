#!/bin/bash

MODULE_NAME="mydevice"

# 1. Scoate vechiul modul
if lsmod | grep "$MODULE_NAME" > /dev/null; then
    echo "[INFO] Modulul este deja încărcat. Îl dezactivez..."
    sudo rmmod $MODULE_NAME
fi

# 2. Încarcă modulul nou
echo "[INFO] Încarc $MODULE_NAME.ko..."
sudo insmod $MODULE_NAME.ko

# 3. Găsește major number din dmesg
MAJOR=$(sudo dmesg | grep "Modulul a pornit" | tail -n 1 | grep -oP "Major: \K[0-9]+")

if [ -z "$MAJOR" ]; then
    echo "[EROARE] Nu am putut obține major number din dmesg."
    exit 1
fi

# 4. Creează device node
echo "[INFO] Creez /dev/$MODULE_NAME cu major $MAJOR..."
sudo rm -f /dev/$MODULE_NAME
sudo mknod /dev/$MODULE_NAME c $MAJOR 0
sudo chmod 666 /dev/$MODULE_NAME

echo "[GATA] Dispozitivul este disponibil la /dev/$MODULE_NAME"

