### New changes
* If you have copied apps into `apps` folder - remove `apps` folder on your microSD before installing this release to avoid issues!
* SubGHz: (Bug that I decided to keep as a feature) You can change default button (Ok) for remote by holding custom button and pressing back at same time (same can be used to restore your button if you changed it accidentally) - Be careful, it might be unstable, I will make proper option to change button in next releases
* SubGHz: Fixes for custom button bugs in SubGHz Remote app
* SubGHz: Add alutech table to enviroment alloc and free
* Docs: Fix and update docs - thanks to @lesterrry
* Plugins: Bluetooth Remote - implemented YouTube Shorts Remote (may be unstable)
* Plugins: Bluetooth Remote - improvements and fixes for TikTok remote (by @krolchonok | PR #420)
* Plugins: Implement an array for baudrates on GPS UART app (+ add 19200 baud) (by @p0ns | PR #416)
* Plugins: Remove UART Echo from releases since it is locked on 115200 baud, and we have **UART Terminal** with ability to set baudrate
* Plugins: Update **TOTP (Authenticator)** [(by akopachov)](https://github.com/akopachov/flipper-zero_authenticator)
* Plugins: Update **UART Terminal** [(by cool4uma)](https://github.com/cool4uma/UART_Terminal/tree/main)
* OFW: Deep Sleep Idle - **Improves battery usage!!!** -> **Breaking API change, api was changed from 21.x to 22.x** 
* OFW: FuriHal: pwr pulls for some pins
* OFW: Bugfix: ISP Programmer and SubGhz
* OFW: AVR_ISP: fix NULL pointer dereference
* OFW: Fix gpio state isp programmer
* OFW: ufbt: project & debugging updates
* OFW: FuriHal: fix gpio naming and add explicit pulls for vibro, speaker and ir_tx -> **Breaking API change, api was changed from 20.x to 21.x** 
**(this will make your manually copied plugins not work, update them in same way you installed them, or delete `apps` folder and then install firmware, if you using extra pack builds (with `e` in version) all apps in _Extra will be updated automatically)**

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


