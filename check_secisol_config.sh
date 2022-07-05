#!/bin/sh
#
# Check if the kernel is configured with suitable options for the "Linux
# Security and Isolation APIs" course (https://man7.org/training/secisol/).
#
# (C) 2022, Michael Kerrisk
#
# Licensed under the GNU General Public License version 2 or later
#

config_start=/tmp/config_start
config_options_file=/tmp/config_options

export LC_ALL=C		# So that sort treats '_' correctly
cat << EOF | sort > $config_start
CONFIG_AUDIT            seccomp  # Auditing infrastructure
CONFIG_CFS_BANDWIDTH    cgroups  # Cgroups CPU bandwidth scheduler
CONFIG_CGROUPS          cgroups  # Cgroups!
CONFIG_CGROUP_PIDS      cgroups  # Cgroups v1
CONFIG_CGROUP_FREEZER   cgroups  # Cgroups v1 Freezer controller
CONFIG_CGROUP_SCHED     cgroups  # Cgroups CPU controller
CONFIG_MEMCG            cgroups  # Cgroups v1 memory controller
CONFIG_USER_NS          capns    # User namespaces
CONFIG_VETH             capns    # Virtual Ethernet device
CONFIG_SECCOMP          seccomp  # Seccomp
CONFIG_SECCOMP_FILTER   seccomp  # Seccomp filter mode
EOF

if test $# -eq 0; then
    cat $config_start > $config_options_file
else
    grep $1 $config_start > $config_options_file
fi
sed -i 's/[[:space:]].*//' $config_options_file

echo "Checking for the following options:"
echo
cat $config_options_file | sed 's/^/        /'
echo

if test -e /proc/config.gz; then
    config=/tmp/proc_config
    zcat /proc/config.gz > $config
elif test -e /lib/modules/$(uname -r)/build/.config; then
    config=/lib/modules/$(uname -r)/build/.config
elif test -e /boot/config-$(uname -r); then
    config=/boot/config-$(uname -r)
else
    echo "Can't detect kernel configuration file"
    echo "Email mtk@man7.org to tell him that his script needs to be fixed for your distro"
    exit 1
fi

cat $config_options_file | sed 's/^/^/;s/$/=/' > ${config_options_file}.re

grep -f ${config_options_file}.re $config | sed 's/=.*//' | sort > ${config_options_file}.present

if cmp -s $config_options_file ${config_options_file}.present; then
    echo "Great! It looks like your kernel has all the needed options."
else
    echo "The following kernel configuration options appear to be missing:"
    echo
    comm -3 $config_options_file ${config_options_file}.present | sed 's/^/	/'
fi
