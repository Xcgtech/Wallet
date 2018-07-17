
Debian
====================
This directory contains files used to package Xchanged/Xchange-qt
for Debian-based Linux systems. If you compile Xchanged/Xchange-qt yourself, there are some useful files here.

## Xchange: URI support ##


Xchange-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install Xchange-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your Xchange-qt binary to `/usr/bin`
and the `../../share/pixmaps/Xchange128.png` to `/usr/share/pixmaps`

Xchange-qt.protocol (KDE)

