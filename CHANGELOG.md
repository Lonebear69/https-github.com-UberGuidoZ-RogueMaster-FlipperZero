### New changes
* If you have copied apps into `apps` folder - remove `apps` folder on your microSD before installing this release to avoid issues!
* Dev Builds: Add extra pack dev branch to avoid "bug" reports with `API mismatch`
* App Loader: Add option to ignore api mismatch (warning! some apps WILL not work, please update them to avoid any issues) -> (by @Willy-JL | PR #395)
* SubGHz: Add manually -> GSN protocol support
* SubGHz: Add 318 and 418 MHz back to hopping list
* SubGHz: Fix hopper stuck at 433.42 due to wide range signals - 
When we using 433.92 remote flipper in hopping mode will stuck at 433.42 and may loose signal because of that, need to avoid using close freqs in hopping, only freqs with bigger difference like 310 -> 315
* Plugins: Update **TOTP (Authenticator)** [(by akopachov)](https://github.com/akopachov/flipper-zero_authenticator) -> BadBT Support
* OFW: Screen streaming improvements
* OFW: 1-Wire Overdrive Mode -> **Breaking API change, api was changed from 19.x to 20.x** 
* OFW: Disable UART IRQs by default
* OFW: BadUSB: implement boot protocol
* OFW: Remove hmac_sha256 from public API -> **Breaking API change, api was changed from 18.x to 19.x** 
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


