/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* cap_hex_to_text.c

   Convert a hexadecimal capabilities mask to a list of capability names.

   The same task is performed by the command 'capsh --decode'.
*/
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/* The order of this list matches the order of the capability numbers in
   <linux/capability.h */

char *capNames[] = {
        "cap_chown", "cap_dac_override", "cap_dac_read_search", "cap_fowner",
        "cap_fsetid", "cap_kill", "cap_setgid", "cap_setuid", "cap_setpcap",
        "cap_linux_immutable", "cap_net_bind_service", "cap_net_broadcast",
        "cap_net_admin", "cap_net_raw", "cap_ipc_lock", "cap_ipc_owner",
        "cap_sys_module", "cap_sys_rawio", "cap_sys_chroot", "cap_sys_ptrace",
        "cap_sys_pacct", "cap_sys_admin", "cap_sys_boot", "cap_sys_nice",
        "cap_sys_resource", "cap_sys_time", "cap_sys_tty_config", "cap_mknod",
        "cap_lease", "cap_audit_write", "cap_audit_control", "cap_setfcap",
        "cap_mac_override", "cap_mac_admin", "cap_syslog", "cap_wake_alarm",
        "cap_block_suspend", "cap_audit_read", "cap_perfmon"
    };

int
main(int argc, char *argv[])
{
    uint64_t hexCaps = strtoll(argv[1], NULL, 16);
    int maxCap = sizeof(capNames) / sizeof(capNames[0]);

    for (int j = 0; j < 64; j++) {
        uint64_t mask = (uint64_t) 1 << j;

        if (hexCaps & mask) {
            if (j < maxCap)
                printf("%s ", capNames[j]);
            else
                printf("%d ", j);
        }

        hexCaps &= ~mask;

        if (hexCaps == 0)
            break;
    }

    printf("\n");
    exit(EXIT_FAILURE);
}
