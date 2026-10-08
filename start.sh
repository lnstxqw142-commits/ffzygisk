#!/system/bin/sh

[ "$(id -u 2>/dev/null || echo 0)" -ne 0 ] && echo "Please use root privileges to execute !" && exit 1

package_name="com.dts.freefiremax"
game_abi=$(dumpsys package "$package_name" 2>/dev/null | awk -F'=' '/primaryCpuAbi/ {print $2; exit}')
device_abi=$(getprop ro.product.cpu.abi 2>/dev/null)

echo
echo "Game ABI   : ${game_abi:-unknown}"
echo "Device ABI : ${device_abi:-unknown}"

case "$device_abi" in
    arm64-v8a|armeabi-v7a)
        device_abi="$game_abi"
        ;;
    x86_64)
        [ "$game_abi" = "armeabi-v7a" ] && device_abi="x86"
        ;;
    x86)
        ;;
    *)
        echo "Device ABI not supported"
        exit 1
        ;;
esac

curDir=$(dirname "$0")
injectPath="/data/local/tmp/zyInject"
soPath="$curDir/libs/$game_abi/libZyGames.so"

cp -f "$curDir/libs/$device_abi/libinject.so" "$injectPath" && chmod 711 "$injectPath"
sh -c "$injectPath -f -n $package_name -so \"$soPath\" --stop-threads --hide-maps"
rm -f "$injectPath" 2>/dev/null
