### New changes
* Plugins: Show External CC1101 module status in Weather Station and in POCSAG Pager plugins
* SubGHz: Fix false detections of StarLine 72bit, flipper can decode only 64bit
* SubGHz: ScherKhan, more informative messages instead of "Unknown", may be not correct, because we guessing type by bits
* SubGHz: Clear code in "Add Manually" scene (by @gid9798 | PR #403)
* Infrared: Universal remote assets updated (by @amec0e | PR #404)
* Plugins: GPS NMEA (UART) modifications
```
- Ability to change baudrate using Up button, hold button to switch between baudrates (9600, 57600, 115200) (i set 57600 as default)
- Ok button will set backlight to always on mode, to disable press ok button again (it will restore default settings after app exit too)
- Long press Right button to change speed from knots to kilometers per hour
- Exit from app using long press on back button instead of short press, may be useful in case you want to turn backlight on and accidentally click back
```
* OFW: Picopass: Elite progress
* OFW: Improved thread lifecycle
* OFW: View Model: recursive mutex

#### [🎲 Download latest extra apps pack](https://github.com/xMasterX/all-the-plugins/archive/refs/heads/main.zip)

[-> How to install firmware](https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/HowToInstall.md)

[-> Download qFlipper (official link)](https://flipperzero.one/update)

### Thanks to our sponsors:
callmezimbra, Quen0n, MERRON, grvpvl (lvpvrg), art_col, ThurstonWaffles, Moneron, UterGrooll, LUCFER, Northpirate, zloepuzo, T.Rat, Alexey B., ionelife, ...
and all other great people who supported our project and me (xMasterX), thanks to you all!

**Note: To avoid issues with .dfu, prefer installing using .tgz with qFlipper, web updater or by self update package, all needed assets will be installed**

**Recommended option - Web Updater**

What means `n` or `e` in - `flipper-z-f7-update-(version)(n / e).tgz` ? - `n` means this build comes without our custom animations, only official flipper animations, 
`e` means build has extra apps pack preinstalled

Self-update package (update from microSD) - `flipper-z-f7-update-(version).zip` or download `.tgz` for mobile app / qFlipper


