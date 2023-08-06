#!/bin/sh

userresources=$HOME/.Xresources
usermodmap=$HOME/.Xmodmap
sysresources=/etc/X11/xinit/.Xresources
sysmodmap=/etc/X11/xinit/.Xmodmap

# merge in defaults and keymaps

if [ -f $sysresources ]; then







    xrdb -merge $sysresources

fi

if [ -f $sysmodmap ]; then
    xmodmap $sysmodmap
fi

if [ -f "$userresources" ]; then







    xrdb -merge "$userresources"

fi

if [ -f "$usermodmap" ]; then
    xmodmap "$usermodmap"
fi

# start some nice programs

if [ -d /etc/X11/xinit/xinitrc.d ] ; then
 for f in /etc/X11/xinit/xinitrc.d/?*.sh ; do
  [ -x "$f" ] && . "$f"
 done
 unset f
fi

#XBINDKEYS
xbindkeys &

#XRANDR
xrandr --output DP-1 --mode 1920x1080 --rate 144 &

#wait 2 seconds and run aslstatus
(sleep 2 && aslstatus) & 

#run wp_init and restore wallpaper and colorscheme
~/.config/wpg/wp_init.sh &&

#automatically restart dwm after quitting
while true; do
  # Log stderror to a file 
  dwm 2> ~/.dwm.log
  # No error logging
  dwm >/dev/null 2>&1
done

#STARTDWM
exec dwm

