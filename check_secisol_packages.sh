#!/bin/sh
#
# Check to see if all packages needed for the "Linux Security and Isolation
# APIs" course (https://man7.org/training/secisol/) are installed.
#
# (C) 2022, Michael Kerrisk
#
# Licensed under the GNU General Public License version 2 or later
#

if [ -f "/etc/arch-release" ]; then
    distro="Arch"
else
    distro=$(lsb_release -is)
fi

case "$distro" in
Debian|Ubuntu|Neon)
    pkg_check_cmd="dpkg -S"
    field=1
    ;;
Fedora)
    pkg_check_cmd="rpm -qi --quiet"
    field=2
    ;;
Arch)
    pkg_check_cmd="pacman -Qi"
    field=3
    ;;
*)
     echo "Sorry: this script doesn't support checking on $distro"
     echo "If you can help improve it for your distro, send a mail for mtk@man7.org"
     exit 1
esac

# Build list of packages for this distro
#
# Column 1 is for Debian/Ubuntu/all
# Column 2 is for RPM based distributions such as Fedora
# Column 3 is for Arch
# '.' means package is as in column 1
# '@' means that this package is not officially supported by the distro

packages=$(cat << EOF | \
	   awk '/^[^#]/ { p=$'$field'
	                  if (p == ".") p = $1
			  else if (p == "@") p = ""
			  print p }'
# Ubuntu/Debian     Fedora              Arch
gcc                 .                   .
golang              .                   go
inotify-tools       .                   .
libseccomp-dev      libseccomp-devel    libseccomp
libcap-dev          libcap-devel        libcap
libcap-ng-utils     .                   libcap-ng
libacl1-dev         libacl-devel        acl
# libselinux1-dev     libselinux-devel    @
make                .                   .
moreutils           .                   .
sudo                .                   .
util-linux          .                   .
EOF
)

missing=

cmd="dpkg -S"
if test "X$distro" = XFedora; then
    cmd="rpm -qi --quiet"
fi

for p in $packages; do
    $pkg_check_cmd $p > /dev/null 2>&1
    if [ $? -eq 0 ] ; then
	echo "$p is present"
    else
	missing="$missing $p"
    fi
done

if test -z "$missing"; then
    echo
    echo "All checked packages are present"
    exit 0
else
    echo
    echo "These packages are missing: $missing"
    echo
    echo "You can probably fix this with: "
    echo

    case "$distro" in
    Fedora)
        echo "    sudo dnf install $missing"
	exit 0
        ;;
    Debian|Ubuntu|Neon)
        echo "    sudo apt install $missing"
	exit 0
	;;
    Arch)
        echo "    sudo pacman -S $missing"
	exit 0
        ;;
    *)
        echo "OOPS! 'case' error for \"distro\""
	exit 1
	;;
    esac
fi
