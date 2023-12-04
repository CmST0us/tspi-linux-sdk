#!/usr/bin/env python

import os
import re
import sys

def common_config(configs):
    allconfigs = {}

    for config in configs:
        if  not config.endswith("config"):
            continue

        allconfigs[config] = set()

        for line in open(config):
            m = re.match("#*\s*(BR2_\w+)[\s=](.*)$", line)
            if not m:
                continue
            option, value = m.groups()
            allconfigs[config].add((option, value))

    common = allconfigs.values()[0].copy()
    for config in allconfigs.keys():
        common &= allconfigs[config]
    for config in allconfigs.keys():
        allconfigs[config] -= common

    print(common)

def load_base(base_cfgs, cfg):
    base_cfgs[cfg] = set()

    for line in open(cfg):
        n = re.match('\s*#include\s*"(\w+\.\w+)"\s*$', line)
        if n:
            name = n.group(1)
            path = os.path.dirname(os.path.realpath(cfg))
            if not path.endswith("rockchip"):
                path += "/rockchip"
            name = path + '/' + name
            load_base(base_cfgs, name)
            continue

        m = re.match("#*\s*(BR2_\w+)[\s=](.*)$", line)
        if m:
            option, value = m.groups()
            base_cfgs[cfg].add((option, value))


def split_config(base_cfgs, config, output):
    load_base(base_cfgs, config)
    base_cfgs.pop(config)

    with open(output, "w+") as f:
        for line in open(config):
            m = re.match("#*\s*(BR2_\w+)[\s=](.*)$", line)
            if not m:
                f.write(line)
                continue

            option, value = m.groups()
            temp = set()
            temp.add((option, value))

            if option == "BR2_ROOTFS_OVERLAY" and value != "is not set":
                values = value.strip('"').split(' ')

                for cfg in base_cfgs.keys():
                    for item in base_cfgs[cfg]:
                        opt, val = item
                        if opt != option:   # Skip not BR2_ROOTFS_OVERLAY
                            continue

                        vals = val.strip('"').split(' ')
                        for v in vals:
                            try:
                                while True:
                                    values.remove(v)
                            except ValueError as e:
                                pass

                if len(values):
                    val = ''
                    for v in values:
                        val = val + v + ' ';
                    val = val.strip()
                    f.write('BR2_ROOTFS_OVERLAY="' + val + '"\n')
            else:
                discard = False

                for cfg in base_cfgs.keys():
                    if temp.issubset(base_cfgs[cfg]):
                        discard = True

                if not discard:
                    f.write(m.group() + '\n')

def merge_base(config):
    result = ''
    overlay = ''
    for line in open(config):
        n = re.match('#include\s*"(\w+\.\w+)"$', line)

        if n:
            path = os.path.dirname(os.path.realpath(config))
            if not path.endswith("rockchip"):
                path += "/rockchip"
            name = path + '/' + n.group(1)

            res, ovl = merge_base(name)
            result += res
            overlay = (overlay + " " + ovl).strip()
        else:
            # W/O quotation mark(")
            m = re.match('BR2_ROOTFS_OVERLAY="(.*)"$', line)
            if m:
                overlay = (overlay + " " + m.group(1)).strip()
            else:
                result += line
    return result, overlay

def merge_cfgs(config, output):
    with open(output, "w+") as f:
        result, overlay = merge_base(config)

        for line in open(config):
            # override ROOTS_OVERLAY
            m = re.match('BR2_ROOTFS_OVERLAY:="(.*)"$', line)
            if m:
                overlay = m.group(1)

        f.write(result)
        f.write('BR2_ROOTFS_OVERLAY="' + overlay + '"\n')

def print_usage():
        print ("Usage:")
        print ("   Split defconfig: " + sys.argv[0] + " -s <defconfig> <output>")
        print ("   Merge defconfig: " + sys.argv[0] + " -m <defconfig> <output>")

if __name__ == '__main__':
    #common_config(sys.argv)

    if len(sys.argv) != 4:
        print_usage()
        sys.exit(-1)

    if sys.argv[1] == '-s':
        base_cfgs = {}
        split_config(base_cfgs, sys.argv[2], sys.argv[3])
    elif sys.argv[1] == '-m':
        merge_cfgs(sys.argv[2], sys.argv[3])
    else:
        print_usage()
