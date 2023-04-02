### New changes
* SubGHz: Support for Allmatic(reads as Beninca), also fix Beninca protocol 
* SubGHz: Allmatic -> Add Manually
* Plugins: Show External CC1101 module status in Weather Station and in POCSAG Pager plugins
* Plugins: Update **TOTP (Authenticator)** [(by akopachov)](https://github.com/akopachov/flipper-zero_authenticator) (Fixes BT issues)
* SubGHz: Fix false detections of StarLine 72bit, flipper can decode only 64bit
* SubGHz: ScherKhan, more informative messages instead of "Unknown", may be not correct, because we guessing type by bits
* SubGHz: Clear code in "Add Manually" scene (by @gid9798 | PR #403)
* SubGHz: Unification of custom buttons (by @gid9798 | PR #405)
* Infrared: Universal remote assets updated (by @amec0e | PR #404)
* Plugins: GPS NMEA (UART) modifications
```
- Ability to change baudrate using Up button, hold button to switch between baudrates (9600, 57600, 115200) (i set 57600 as default)
- Ok button will set backlight to always on mode, to disable press ok button again (it will restore default settings after app exit too)
- Long press Right button to change speed from knots to kilometers per hour
- Exit from app using long press on back button instead of short press, may be useful in case you want to turn backlight on and accidentally click back
```
* OFW PR 2536: Fixing parsing troika card number (by @achistyakov)
* OFW PR 2548: Picopass Add more detail to saved info screen (by @bettse)
* OFW PR 2544: added WAIT_FOR_BUTTON_PRESS functionality to badusb (by @p4p1) 
* OFW PR 2475: AVR ISP Programmer FAP (by @Skorpionm) (+ AVR ISP enable 5v power on start fix) (beta test, report found issues in our repo)
* OFW: FatFS: use rtc for timestamping
* OFW: RPC: increase max message size
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


