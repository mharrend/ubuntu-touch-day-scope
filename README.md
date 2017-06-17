# Day Scope for UBPorts of Ubuntu Touch
Day Scope for Ubuntu Touch / UBPorts

## Note: Fork from Launchpad / Credit to the original developers
* The Day Scope was originally developed and written by the [day-scope-team](https://launchpad.net/~day-scope-team) using the GNU GPLv3.
* After Canonical stopped developing Ubuntu Touch, I was inspired by the following thread in the [UBPorts forum](https://forums.ubports.com/topic/255/today-scope-without-showing-day-scope) to fork the day scope and to derive a working click package which I could use on my phone running UBPorts.
* The original launchpad repo can be found using the following [link.](https://code.launchpad.net/day-scope)

### Changes of the original code
* I did the following changes with regards to the original launchpad version to derive the first working version of the click package. So refer to my first commit to this repository.
* Please note that I will not document later code changes in this README, instead check out the history of this repository.

Changes:
* Changed the query.cpp file to deactivate Moon Phase part so far, since MoonPhase API Provider requires a license which costs money. Without a secret key and an api key the moon phase cannot be obtained any longer.
* Removed requirement of providing an assets file which contained the secret and api key for the moon phase API in the CMakeLists.txt file.
* Changed the name of the maintainer to myself in the CMakeLists.txt file. In the long run I hope that the UBPorts team will include the Day Scope in its repository.


## Requirements to build a click package from this repository
* clickable, follow installation instructions in https://github.com/bhdouglass/clickable
* qtbase5-dev:armhf library in LXC container

## Compilation and installation of day scope
* Follow Clickable instructions and obtain an clickable-armhf LXC image
* Log into the LXC image via: `lxc exec clickable-armhf /bin/bash`
* Inside of the LXC image install the missing QT5 libraries via
```
apt-get install qtbase5-dev:armhf libgles2-mesa-dev:armhf libegl1-mesa-dev:armhf libwayland-dev:armhf libmirclient-dev:armhf libmircommon-dev:armhf libxkbcommon-dev:armhf
```
1. Logout from the LXC image
1. Add clickable to your PATH environment variable via `export PATH=$PATH:/PATH/TO/clickable`
1. Check out this github repository 
1. cd into the created folder
1. Build the day scope for Ubuntu Touch via
```
clickable build  click-build   -t cmake
```
1. If you want, you can build and install the click package directly to your phone at the same (Note: You must have SSH with public/private key enabled and your phone must be accessible via Wifi)
```
clickable build  click-build install   -t cmake -i IP-AdressOfYourPhone
```
1. Enjoy!!!


## Fast install of click package
* You can find a click package ready for installation on UBPorts 15.04 under releases.
* Legal note: THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

